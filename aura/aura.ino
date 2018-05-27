/******************************************************************************
 * INTERFACE                                                                  *
 ******************************************************************************/

// temporary width and height for random values
#define WIDTH  100
#define HEIGHT 100

// math limit for error
#define EPSILON 0.001f

// float print precision
#define FLPR 2

// hardcoded because it's hardware based
#define NUM_OF_LEDS 32

// pulse width modulation (PWM)
#define PWM_MOD 8
#define PWM_MAX (PWM_MOD - 1)

// pins
#define SER0 0
#define SER1 1
#define SER2 2
#define SER3 3
#define SER4 4
#define SER5 5
#define SER6 6
#define SER7 7
#define RCLK  8
#define SRCLK 9

#define SW0 10
#define SW1 11

// LED_BUILTIN 13

#define SEN_L A0
#define SEN_R A1
#define SEN_C A2

#define SEN_R_THRESH 900
#define SEN_L_THRESH 900
#define SEN_C_THRESH 900

#define SEN_R_DEBOUNCE 50000
#define SEN_L_DEBOUNCE 50000
#define SEN_C_DEBOUNCE 50000


// settings
enum ledMode_t {
    testMode = 0,
    allMode  = 1,
    randMode = 2,
    calcMode = 3
};

// random float, [0,1]
#define RAND_MAX 0x7FFF

float randFloat (void) {
    return (float) random(RAND_MAX) / RAND_MAX;
}

// sort connections

bool lessThan (int a_i, int a_j, int b_i, int b_j, float** dist) {
    return dist[a_i][a_j] < dist[b_i][b_j];
}

int quicklyPartition (int* indices_i, int* indices_j, float** dist, int lo, int hi) {
    // pivot
    int p_i = indices_i[lo];
    int p_j = indices_j[lo];
    lo--;
    hi++;

    while (true) {
    	do {
    		lo++;
    	} while (lessThan(indices_i[lo], indices_j[lo], p_i, p_j, dist));

        do {
        	hi--;
        } while (lessThan(p_i, p_j, indices_i[hi], indices_j[hi], dist));

        if (lo >= hi) {
            return hi;
        }

        int a  = indices_i[lo];
        indices_i[lo] = indices_i[hi];
        indices_i[hi] = a;

        a  = indices_j[lo];
        indices_j[lo] = indices_j[hi];
        indices_j[hi] = a;
    }
}

void recQuicklySort (int* indices_i, int* indices_j, float** dist, int k, int lo, int hi) {
    // adapted from wikipedia
    if (lo < hi) {
        int p = quicklyPartition(indices_i, indices_j, dist, lo, hi);
        recQuicklySort(indices_i, indices_j, dist, k, lo, p);
        recQuicklySort(indices_i, indices_j, dist, k, p + 1, hi);
    }
}

void quicklySort (int* indices_i, int* indices_j,  float** dist, int k) {
    recQuicklySort(indices_i, indices_j, dist, k, 0, k-1);
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

    LedArray (void);
    ~LedArray (void);

    void calculatePwm (void);

    void printData (void);
    void printDisplay (void);
    void printConnections (void);
    void printPoints (void);
    void printConnectMatrix (void);
    void printDistMatrix (void);
    void printConnectionSum(void);
};

LedArray::LedArray (void) {
    n = 0;
    x = NULL;
    y = NULL;
    pwm = NULL;
    dist = NULL;
    connect = NULL;

    Serial.begin(9600);
    
    getXY();
    getDist();
    getConnect();

    pwm = (unsigned int*) calloc(n, sizeof(unsigned int));
}

LedArray::~LedArray (void) {
    free(x);
    free(y);
    x = NULL;
    y = NULL;

    for (int i = 0; i < n; i++) {
        free(dist[i]);
        free(connect[i]);
    }
    free(dist);
    free(connect);
    dist = NULL;
    connect = NULL;

    free(pwm);
    pwm = NULL;
}

void LedArray::getXY (void) {
    n = NUM_OF_LEDS;
    x = (float*) calloc(n, sizeof(float));
    y = (float*) calloc(n, sizeof(float));

    // this section should be all hardcoded for final version

    // use random values for testing
    for (int i = 0; i < n; i++) {
        x[i] = randFloat() * WIDTH;
        y[i] = randFloat() * HEIGHT;

        // random values in a circle
        //float t = randFloat() * 2 * PI;
        //x[i] = (WIDTH  / 2.0f) + (WIDTH  * 0.4f * cos(t));
        //y[i] = (HEIGHT / 2.0f) + (HEIGHT * 0.4f * sin(t));
    }
}

void LedArray::getDist (void) {
    dist = (float**) calloc(n, sizeof(float*));
    for (int i = 0; i < n; i++) {
        dist[i] = (float*) calloc(n, sizeof(float));
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
    connect = (bool**) calloc(n, sizeof(bool*));
    for (int i = 0; i < n; i++) {
        connect[i] = (bool*) calloc(n, sizeof(bool));
        for (int j = 0; j < n; j++) {
            connect[i][j] = true;
        }
    }

    int* indices_i = (int*) calloc(n * (n-1) / 2, sizeof(int));
    int* indices_j = (int*) calloc(n * (n-1) / 2, sizeof(int));
    int k = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < i; j++) {
            indices_i[k] = i;
            indices_j[k] = j;
            k++;
        }
        connect[i][i] = false;
    }

    // sort lines fron sortest to longest
    quicklySort(indices_i, indices_j, dist, k);

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
        int a = indices_i[i];
        int b = indices_j[i];
        if (connect[a][b]) {
            for (int j = i+1; j < k; j++) {
                int c = indices_i[j];
                int d = indices_j[j];
                if (connect[c][d] &&
                    (a != c) &&
                    (a != d) &&
                    (b != c) &&
                    (b != d) &&
                    linesIntersect(a, b, c, d)) {
                    connect[c][d] = false;
                    connect[d][c] = false;
                }
            }
        }
    }
    free(indices_i);
    indices_i = NULL;
    free(indices_j);
    indices_j = NULL;
}

void LedArray::printPoints (void) {
    Serial.print("points = [\n");
    for (int i = 0; i < n; i++) {
        Serial.print("          ");
        Serial.print(x[i], FLPR);
        Serial.print(", ");
        Serial.print(y[i], FLPR);
        Serial.print(";\n");
    }
    Serial.print("          ];\n\n");
}

void LedArray::printDistMatrix (void) {
    Serial.print("distance = [\n");
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < n; i++) {
            Serial.print(dist[i][j], FLPR);
            Serial.print(", ");
        }
        Serial.print(";\n");
    }
    Serial.print("            ];\n\n");
}

void LedArray::printConnectMatrix (void) {
    Serial.print("connection = [\n");
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < n; i++) {
            Serial.print((int)connect[i][j]);
            Serial.print(", ");
        }
        Serial.print(";\n");
    }
    Serial.print("              ];\n\n");
}

void LedArray::printData (void) {
    Serial.print("N = ");
    Serial.print(n);
    Serial.print(";\n\n");
    printPoints();
    printDistMatrix();
    printConnectMatrix();
    printConnectionSum();
}

void LedArray::printConnectionSum (void) {
    // determine if there are the right number of connections
    int sum = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            sum += (int) connect[i][j];
        }
    }
    Serial.print("% expected connection min:");
    Serial.println((2 * n) - 3);
    Serial.print("% expected connection max:");
    Serial.println((3 * n) - 6);
    Serial.print("% actual connection sum:");
    Serial.println(sum);
    Serial.print("\n");
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

/******************************************************************************
 * MAIN                                                                       *
 ******************************************************************************/

void setup (void) {
    Serial.println("% Hello.");

    LedArray leds;
    leds.printData();
}

void loop (void) {
}