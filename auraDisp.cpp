#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <climits>
#include <ctime>
#include <algorithm>
#include <functional>

using namespace std;

///////////
// PATTERNS

//////////
// DISPLAY

#define WIDTH   50
#define HEIGHT  20

char screen[WIDTH][HEIGHT] = {' '};
void drawScreen (void);

void clearScreen (void) {
    for (int j = 0; j < HEIGHT; j++) {
        for (int i = 0; i < WIDTH; i++) {
            screen[i][j] = ' ';
        }
    }
}

char getDecChar (int d) {
    return (char) 0x30 + (d % 10);
}

char getHexChar (int d) {
    d %= 16;
    d += (d >= 10) ? 0x40 : 0x30; // letter if >= 10
    return (char) d;
}

////////////
// INTERFACE

// math limit for error
#define EPSILON 0.001f

// hardcoded because it's hardware based
#define NUM_OF_LEDS 15

// pulse width modulation (PWM)
#define PWM_MOD 8
#define PWM_MAX (PWM_MOD - 1)

// settings
enum ledMode_t {
    testMode = 0,
    allMode  = 1,
    randMode = 2,
    calcMode = 3
};

// used for setting up connections
struct index_t {
    int i;
    int j;
};

bool compare (index_t a, index_t b, float** dist) {
    return dist[a.i][a.j] < dist[b.i][b.j];
}

// math

float dot (float x1, float y1, float x2, float y2) {
    return (x1 * x2) + (y1 * y2);
}

float det (float x1, float y1, float x2, float y2) {
    return (x1 * y2) - (y1 * x2);
}

class LedArray {

public:
    int n;
    float* x;
    float* y;

    float** dist;
    bool** connect;

    ledMode_t mode;
    unsigned int* pwm;

    void getXY (void);
    void getDist (void);
    bool linesIntersect (int i1, int i2, int i3, int i4);
    void getConnect (void);

    void drawLine(int i,int j);
    void _fprintScreen (FILE* stream);

    void _fprintPoints (FILE* stream);
    void _fprintConnectMatrix (FILE* stream);
    void _fprintDistMatrix (FILE* stream);
    void _fprintData (FILE* stream);

    void _fprintDisplay (FILE* stream);
    void _fprintConnections (FILE* stream);


    LedArray (void);
    ~LedArray (void);

    void calculatePwm (void);

    void printData (void);
    void printDisplay (void);
    void printConnections (void);
    void printPoints (void);
    void printConnectMatrix (void);
    void printDistMatrix (void);

    void fprintData (void);
    void fprintDisplay (void);
    void fprintConnections (void);
};

LedArray::LedArray (void) {
    n = NUM_OF_LEDS;

    x = (float*) calloc(n, sizeof(float));
    y = (float*) calloc(n, sizeof(float));
    pwm = (unsigned int*) calloc(n, sizeof(unsigned int));

    dist = (float**) calloc(n, sizeof(float*));
    connect = (bool**) calloc(n, sizeof(bool*));
    for (int i = 0; i < n; i++) {
        dist[i] = (float*) calloc(n, sizeof(float));
        connect[i] = (bool*) calloc(n, sizeof(bool));
    }

    getXY();
    getDist();
    getConnect();
}

LedArray::~LedArray (void) {
    free(x);
    free(y);
    free(pwm);

    for (int i = 0; i < n; i++) {
        free(dist[i]);
        free(connect[i]);
    }
    free(dist);
    free(connect);

}

void LedArray::getXY (void) {
    for (int i = 0; i < n; i++) {
        x[i] = ((float) rand() / RAND_MAX) * WIDTH;
        y[i] = ((float) rand() / RAND_MAX) * HEIGHT;

        //float t = (((float) rand()) / RAND_MAX) * 6.28f;
        //x[i] = (WIDTH  / 2.0f) + (WIDTH  * 0.4f * cos(t));
        //y[i] = (HEIGHT / 2.0f) + (HEIGHT * 0.4f * sin(t));
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
        for (int j = 0; j < n; j++) {
            connect[i][j] = true;
        }
    }

    index_t* indices = (index_t*) calloc(n * (n-1) / 2, sizeof(index_t));
    int k = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < i; j++) {
            indices[k].i = i;
            indices[k].j = j;
            k++;
        }
        connect[i][i] = false;
    }

    // sort lines fron sortest to longest
    sort(&indices[0], &indices[k], bind(compare,  placeholders::_1, placeholders::_2, dist));

    // for each line (from shortest to longest)
    //     if line exists
    //         for each longer line
    //             if longer line exists and
    //                none of the points are the same and
    //                they intersect
    //                 remove the longer line
    // this will fail if points are same or if lines are parallel
    // those cases should be rare
    for (int i = 0; i < k; i++) {
        int a = indices[i].i;
        int b = indices[i].j;
        //printf("k = %2d    i = %2d    j = %2d    d = %5.2f\n", i, a, b, dist[a][b]);
        if (connect[a][b]) {
            for (int j = i+1; j < k; j++) {
                int c = indices[j].i;
                int d = indices[j].j;
                if (connect[c][d] &&
                    (a != c) &&
                    (a != d) &&
                    (b != c) &&
                    (b != d) &&
                    linesIntersect(a, b, c, d)) {
                    connect[c][d] = false;
                    connect[d][c] = false;
                    //printf("    i = %2d    j = %2d    d = %5.2f\n", c, d, dist[c][d]);
                }
            }
        }
    }
}

void LedArray::_fprintPoints (FILE* stream) {
    fprintf(stream, "POINTS:\n");
    fprintf(stream, "  i     x     y\n");
    for (int i = 0; i < n; i++) {
        fprintf(stream, "%3d %5.2f %5.2f\n", i, x[i], y[i]);
    }
    fprintf(stream, "\n");
}

void LedArray::_fprintDistMatrix (FILE* stream) {
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
}

void LedArray::_fprintConnectMatrix (FILE* stream) {
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

void LedArray::_fprintData (FILE* stream) {
    fprintf(stream, "N = %d\n\n", n);
    _fprintPoints(stream);
    //_fprintDistMatrix(stream);
    _fprintConnectMatrix(stream);

    // determine if there are the right number of connections
    int sum = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            sum += (int) connect[i][j];
        }
    }
    fprintf(stream, "connection sum    expected: %d    actual: %d\n", ((n * 2) - 3) * 2, sum);
}

void LedArray::_fprintScreen(FILE* stream) {
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

void LedArray::_fprintDisplay (FILE* stream) {
    clearScreen();

    for (int i = 0; i < n; i++) {
        screen[(int)x[i]][(int)y[i]] = (char) (0x30 + pwm[i]);
    }
    _fprintScreen(stream);
}

void LedArray::drawLine(int i,int j) {
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
    int dirx = (x2 > x1) ? 1 : -1;
    int diry = (y2 > y1) ? 1 : -1;

    //printf("From: %5.2f %5.2f\n", x1, y1);
    //printf("To:   %5.2f %5.2f\n", x2, y2);

    for (int t = 0; abs(t) < (int)abs(dx); t += dirx) {
        int xt = (int)(x1 + t);
        int yt = (int)(y1 + (yox * t));
        screen[xt][yt] = '.';
        //printf("x     %2d %2d\n", xt, yt);
    }
    for (int t = 0; abs(t) < (int)abs(dy); t += diry) {
        int xt = (int)(x1 + (xoy * t));
        int yt = (int)(y1 + t);
        screen[xt][yt] = '.';
        //printf("y     %2d %2d\n", xt, yt);
    }

    screen[(int)x[i]][(int)y[i]] = getHexChar(i);
    screen[(int)x[j]][(int)y[j]] = getHexChar(j);
}

void LedArray::_fprintConnections (FILE* stream) {
    clearScreen();

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (connect[i][j]) {
                drawLine(i, j);
            }
        }
    }

    for (int i = 0; i < n; i++) {
        screen[(int)x[i]][(int)y[i]] = getHexChar(i);
    }

    _fprintScreen(stream);
}

void LedArray::printPoints (void) {
    _fprintPoints(stdout);
}

void LedArray::printDistMatrix (void) {
    _fprintDistMatrix(stdout);
}

void LedArray::printConnectMatrix (void) {
    _fprintConnectMatrix(stdout);
}

void LedArray::printData (void) {
    _fprintData(stdout);
}

void LedArray::printDisplay (void) {
    _fprintDisplay(stdout);
}

void LedArray::printConnections (void) {
    _fprintConnections(stdout);
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

void LedArray::fprintConnections (void) {
    FILE* fout = fopen("aura_connect.txt", "w");
    if (fout != NULL) {
        _fprintConnections(fout);
        fclose(fout);
    }
}

void LedArray::calculatePwm (void) {
    // read settings
    mode = randMode;

    switch (mode) {
        case testMode:
            break;
        case allMode:
            for (int i = 0; i < n; i++) {
                pwm[i] = PWM_MAX;
            }
            break;
        case randMode:
            for (int i = 0; i < n; i++) {
                pwm[i] = rand() % PWM_MOD;
            }
            break;
        case calcMode:
            break;
        default:
            break;
    }
}

///////
// MAIN

int main (void) {
    printf("Hello, brave, new World!\n");

    srand(time(NULL));

    LedArray leds;

    leds.printData();
    leds.printConnections();

    for (int i = 0; i < 10; i++) {
        //leds.calculatePwm();
        //leds.printDisplay();
    }

    printf("Good-bye, cruel World!\n");
    return 0;
}
