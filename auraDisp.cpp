#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <climits>

using namespace std;

///////////
// PATTERNS

//////////
// DISPLAY

#define WIDTH  30
#define HEIGHT 10

char screen[WIDTH][HEIGHT] = {' '};
void drawScreen (void);

void clearScreen (void) {
    for (int j = 0; j < HEIGHT; j++) {
        for (int i = 0; i < WIDTH; i++) {
            screen[i][j] = ' ';
        }
    }    
}
////////////
// INTERFACE

#define EPSILON 0.001f

// hardcoded because it's hardware based
#define NUM_OF_LEDS 5
#define RADIX 8

float dot (float x1, float y1, float x2, float y2) {
    return (x1 * x2) + (y1 * y2);
}

float det (float x1, float y1, float x2, float y2) {
    return (x1 * y2) - (y1 * x2);
}

class LedArray {

private:
    int n = NUM_OF_LEDS;
    float x[NUM_OF_LEDS] = {0.0f};
    float y[NUM_OF_LEDS] = {0.0f};

    float dist[NUM_OF_LEDS][NUM_OF_LEDS] = {0.0f};
    bool connect[NUM_OF_LEDS][NUM_OF_LEDS] = {true};

    unsigned int pwm[NUM_OF_LEDS] = {0};

    void getXY (void);
    void getDist (void);
    bool linesIntersect (int i1, int i2, int i3, int i4);
    void getConnect (void);

    void _fprintData (FILE* stream);
    void _fprintDisplay (FILE* stream);
    void _fprintConnect (FILE* stream);

public:
    LedArray (void);
    ~LedArray (void);

    void calculatePwm (void);

    void printData (void);
    void printDisplay (void);
    void printConnect (void);

    void fprintData (void);
    void fprintDisplay (void);
    void fprintConnect (void);
};

LedArray::LedArray (void) {
    getXY();
    getDist();
    getConnect();
}

LedArray::~LedArray (void) {
    
}

void LedArray::getXY (void) {
    for (int i = 0; i < n; i++) {
        x[i] = (((float) rand()) / INT_MAX) * WIDTH;
        y[i] = (((float) rand()) / INT_MAX) * HEIGHT;
    }
}

void LedArray::getDist (void) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < i; j++) {
            float dx = x[j] - x[i];
            float dy = y[j] - y[i];
            dist[i][j] = sqrt((dx * dx) + (dy * dy));
            dist[j][i] = dist[i][j];
        }
        dist[i][i] = 0.0f;
    }
}

bool LedArray::linesIntersect (int i1, int i2, int i3, int i4) {
    // l1 = p1 + t * v1
    // l2 = p2 + s * v2
    // with t e [0,1], s e [0,1]

    // calculate vectors
    float p1x = x[i1];
    float p1y = y[i1];
    float v1x = x[i2] - p1x;
    float v1y = y[i2] - p1y;

    float p2x = x[i3];
    float p2y = y[i3];
    float v2x = x[i4] - p2x;
    float v2y = y[i4] - p2y;

    // unit vectors
    float d1 = sqrt((v1x * v1x) + (v1y * v1y));
    float u1x = v1x / d1;
    float u1y = v1y / d1;

    float d2 = sqrt((v2x * v2x) + (v2y * v2y));
    float u2x = v2x / d2;
    float u2y = v2y / d2;

    // perpendicular to u1
    float upx = u1y;
    float upy = -1.0f*u1x;

    // displacement between points
    float dx = p2x - p1x;
    float dy = p2y - p1y;

    // | u1 . u2 | < err
    if (abs(dot(upx, upy, u2x, u2y)) < EPSILON) {
        // lines are parallel
        // test if a point from one is on the other

        // bad, lines might be extremely vertical
        //t = (p2(1) - p1(1)) / v1(1)
        //b = abs((p1(2) + t * v1(2)) - p2(2)) < err

        // if point is very close to line 
        // calculated using perpendicular unit vector
        if (abs(dot(dx, dy, upx, upy)) < EPSILON) {
            // collinear

            // project l2 onto l1
            // s0 = l2 start
            // s1 = l2 stop
            // t0 and t1 are obviously 0 and 1
            float dotv1 = dot(v1x, v1y, v1x, v1y);
            float r0 =      dot( dx,  dy, v1x, v1y) / dotv1;
            float r1 = r0 + dot(v2x, v2y, v1x, v1y) / dotv1;
            float s0 = r0 < r1 ? r0 : r1; // min
            float s1 = r0 > r1 ? r0 : r1; // max
            
            // check is intervals overlap
            return ((0.0f <= s0) && (s0 <= 1.0f)) || 
                   ((0.0f <= s1) && (s1 <= 1.0f)) || 
                   ((s0 <= 0.0f) && (0.0f <= s1)) || 
                   ((s0 <= 1.0f) && (1.0f <= s1));
        } else {
            // parallel but not collinear
            return false;
        }
    } else {
        // the lines intersect
        // calculate if (0 < t < 1) and (0 < s < t)
        float detv1v2 = det(v1x, v1y, v2x, v2y);
        
        float t = det(dx, dy, v1x / detv1v2, v1y / detv1v2);
        float s = det(dx, dy, v2x / detv1v2, v2y / detv1v2);
        return ((0.0f <= t) && (t <= 1.0f)) && 
               ((0.0f <= s) && (s <= 1.0f));
    }
}

void LedArray::getConnect (void) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < i; j++) {
            // dist[i][j]
            
            // next, test only connections below this one
            for (int k = 0; k < i; k++) {
                for (int h = 0; h < k; h++) {
                    if (connect[k][h] &&
                        (h != j) &&
                        linesIntersect(i,j,k,h)) {
                        // dist[h][k]
                        if (dist[i][j] >= dist[k][h]) {
                            connect[k][h] = false;
                        } else {
                            connect[i][j] = false;
                        }
                    }
                }
            }
        }
        connect[i][i] = false;
    }
}

void LedArray::_fprintData (FILE* stream) {
    fprintf(stream, "POINTS:\n");
    fprintf(stream, "  i     x     y\n");
    for (int i = 0; i < n; i++) {
        fprintf(stream, "%3d %5.2f %5.2f\n", i, x[i], y[i]);
    }
    fprintf(stream, "\n");

    fprintf(stream, "distance:\n");
    fprintf(stream, "      ");
    for (int i = 0; i < n; i++) {
        fprintf(stream, "   %3d", i);
    }
    fprintf(stream, "\n");
    for (int j = 0; j < n; j++) {
        fprintf(stream, "   %3d", j);
        for (int i = 0; i < n; i++) {
            fprintf(stream, " %5.2f", dist[i][j]);
        }
        fprintf(stream, "\n");
    }
    fprintf(stream, "\n");    

    fprintf(stream, "connection:\n");
    fprintf(stream, "   ");
    for (int i = 0; i < n; i++) {
        fprintf(stream, "%3d", i);
    }
    fprintf(stream, "\n");
    for (int j = 0; j < n; j++) {
        fprintf(stream, "%3d", j);
        for (int i = 0; i < n; i++) {
            fprintf(stream, "  %c", connect[i][j] ? '1' : '.');
        }
        fprintf(stream, "\n");
    }
    fprintf(stream, "\n");    
}

void LedArray::_fprintDisplay (FILE* stream) {
	clearScreen();
	
    for (int i = 0; i < n; i++) {
        screen[(int)x[i]][(int)y[i]] = (char) (0x30 + pwm[i]);
    }

    fprintf(stream, "+");
    for (int i = 0; i < WIDTH; i++) {
        fprintf(stream, "-");
    }
    fprintf(stream, "+\n");

    for (int j = 0; j < HEIGHT; j++) {
        fprintf(stream, "|");
        for (int i = 0; i < WIDTH; i++) {
            fprintf(stream, "%c", screen[i][j]);
        }
        fprintf(stream, "|\n");
    }

    fprintf(stream, "+");
    for (int i = 0; i < WIDTH; i++) {
        fprintf(stream, "-");
    }
    fprintf(stream, "+\n");
}
    
void LedArray::_fprintConnect (FILE* stream) {
	clearScreen();
	
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (connect[i][j]) {
                // *sigh*, can't think of a good algorithm
                // let's just go along x then go along y
                float x1 = x[i];
                float y1 = y[i];
                float x2 = x[j];
                float y2 = y[j];
                float dx = x2 - x1;
                float dy = y2 - y1;
                float xoy = dx/dy;
                float yox = dy/dx;
                for (int t = 0; t < (int)dx; t++) {
                    screen[(int)(x1 + t)][(int)(y1 + (yox * t))] = 'X';
                }
                for (int t = 0; t < (int)dy; t++) {
                    screen[(int)(x1 + (xoy * t))][(int)(y1 + t)] = 'X';
                }
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        screen[(int)x[i]][(int)y[i]] = 'O';    
    }
}

void LedArray::printData (void) {
    _fprintData(stdout);
}

void LedArray::printDisplay (void) {
    _fprintDisplay(stdout);
}

void LedArray::printConnect (void) {
    _fprintConnect(stdout);
}

void LedArray::fprintData (void) {
    FILE* fout = fopen("aura_data.txt", "w");
    if (fout != NULL) {
        _fprintData(fout);
        fclose(fout);
    }
}

void LedArray::fprintDisplay (void) {
    FILE* fout = fopen("aura_display.txt", "w");
    if (fout != NULL) {
        _fprintDisplay(fout);
        fclose(fout);
    }
}

void LedArray::fprintConnect (void) {
    FILE* fout = fopen("aura_connect.txt", "w");
    if (fout != NULL) {
        _fprintConnect(fout);
        fclose(fout);
    }
}

///////
// MAIN

int main (void) {
    printf("Hello, brave, new World!\n");
    
    LedArray leds;
    leds.printData();
    leds.printDisplay();
    leds.printConnect();

    leds.fprintData();
    leds.fprintDisplay();
    leds.fprintConnect();
    
    printf("Good-bye, cruel World!\n");
    return 0;
}
