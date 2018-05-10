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

////////////
// INTERFACE

// hardcoded because it's hardware based
#define NUM_OF_LEDS 32
#define RADIX 8

class LedArray {

private:
	float ledX[NUM_OF_LEDS] = {0.0f};
	float ledY[NUM_OF_LEDS] = {0.0f};

	float distances[NUM_OF_LEDS][NUM_OF_LEDS] = {0.0f};
	bool connections[NUM_OF_LEDS][NUM_OF_LEDS] = {true};

	unsigned int ledPwm[NUM_OF_LEDS] = {0};

	void getXY (float* &x, float* &y, unsigned int n);
	void getDist (float* x, float* y, float** &dist, unsigned int n);
	bool linesIntersect (int i1, int i2, int j1, int j2);
	void getConnect (float** dist, bool** &connect, unsigned int n);

	void _fprintData (FILE* stream, float* x, float* y, float** dist, bool**  connect, unsigned int n);
	void _fprintDisplay (FILE* stream, float* x, float* y, unsigned int* ledPwm, unsigned int n);
	void _fprintConnect (FILE* stream, float* x, float* y, bool** connect, unsigned int n);

public:
	LedArray (void);
	~LedArray (void);

	void calculatePwm (void);

	void printData (float* x, float* y, float** dist, bool**  connect, unsigned int n);
	void printDisplay (float* x, float* y, unsigned int* ledPwm, unsigned int n);
	void printConnect (float* x, float* y, bool** connect, unsigned int n);

	void fprintData (float* x, float* y, float** dist, bool**  connect, unsigned int n);
	void fprintDisplay (float* x, float* y, unsigned int* ledPwm, unsigned int n);
	void fprintConnect (float* x, float* y, bool** connect, unsigned int n);
};

LedArray::LedArray (void) {
	
}

LedArray::~LedArray (void) {
	
}

void LedArray::getXY (float* &x, float* &y, unsigned int n) {
	for (int i = 0; i < n; i++) {
		x[i] = (((float) rand()) / INT_MAX) * WIDTH;
		y[i] = (((float) rand()) / INT_MAX) * HEIGHT;
	}
}

void LedArray::getDist (float* x, float* y, float** &dist, unsigned int n) {
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
	float x1 = x[i1];
	float y1 = y[i1];
	float x2 = x[i2];
	float y2 = y[i2];

	float x3 = x[i3];
	float y3 = y[i3];
	float x4 = x[i4];
	float y4 = y[i4];

	float dx2 = x2 - x1;
	float dy2 = y2 - y1;
	float d2 = sqrt((dx2 * dx2) + (dy2 * dy2));
	float ux2 = dx2 / d2;
	float uy2 = dy2 / d2;

	float dx4 = x4 - x3;
	float dy4 = y4 - y3;
	float d4 = sqrt((dx4 * dx4) + (dy4 * dy));
	float ux4 = dx4 / d4;
	float uy4 = dy4 / d4;
	
	
}

void LedArray::getConnect (float** dist, bool** &connect, unsigned int n) {
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

void LedArray::_fprintData (FILE* stream, float* x, float* y, float** dist, bool**  connect, unsigned int n) {
	fprintf(stream, "POINTS:\n");
	fprintf(stream, "  i     x     y\n");
	for (int i = 0; i < n; i++) {
		fprintf(stream, "%3d %5.2f %5.2f\n", x[i], y[i]);
	}
	fprintf(stream, "\n");

	fprintf(stream, "distance:\n");
	fprintf(stream, "      ");
	for (int i = 0; i < n; i++) {
		fprintf(stream, "   %3d\n", i);
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
		fprintf(stream, "%3d\n", i);
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

void LedArray::_fprintDisplay (FILE* stream, float* x, float* y, unsigned int* ledPwm, unsigned int n) {
	for (int i = 0; i < n; i++) {
		screen[(int)x[i]][(int)y[i]] = (char) (0x30 + ledPwm[i]);
	}

	for (int j = 0; j < HEIGHT; j++) {
		for (int i = 0; i < WIDTH; i++) {
			fprintf(stream, "%c", screen[i][j]);
		}
		fprintf(stream, "\n");
	}
}
	
void LedArray::_fprintConnect (FILE* stream, float* x, float* y, bool** connect, unsigned int n) {
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

	for (int j = 0; j < HEIGHT; j++) {
		for (int i = 0; i < WIDTH; i++) {
			screen[i][j] = ' ';
		}
	}	
}

void LedArray::printData (float* x, float* y, float** dist, bool**  connect, unsigned int n) {
	_fprintData(stdout, x, y, dist, connect, n);
}

void LedArray::printDisplay (float* x, float* y, unsigned int* ledPwm, unsigned int n) {
	_fprintDisplay(stdout, x, y, ledPwm, n);
}

void LedArray::printConnect (float* x, float* y, bool** connect, unsigned int n) {
	_fprintConnect(stdout, x, y, connect, n);
}

void LedArray::fprintData (float* x, float* y, float** dist, bool**  connect, unsigned int n) {
	FILE* fout = fopen("aura_data.txt", "w");
	if (fout != NULL) {
		_fprintData(fout, x, y, dist, connect, n);
		fclose(fout);
	}
}

void LedArray::fprintDisplay (float* x, float* y, unsigned int* ledPwm, unsigned int n) {
	FILE* fout = fopen("aura_display.txt", "w");
	if (fout != NULL) {
		_fprintDisplay(fout, x, y, ledPwm, n);
		fclose(fout);
	}
}

void LedArray::fprintConnect (float* x, float* y, bool** connect, unsigned int n) {
	FILE* fout = fopen("aura_connect.txt", "w");
	if (fout != NULL) {
		_fprintConnect(fout, x, y, connect, n);
		fclose(fout);
	}
}

///////
// MAIN

int main (void) {
	printf("Hello, brave, new World!\n");
	
	
	
	printf("Good-bye, cruel World!\n");
	return 0;
}