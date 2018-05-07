#include <cstdio>

using namespace std;

///////////
// PATTERNS

////////////
// INTERFACE

#define NUM_OF_LEDS 32
#define RADIX

float ledX[NUM_OF_LEDS] = {0.0f};
float ledY[NUM_OF_LEDS] = {0.0f};

float distances[NUM_OF_LEDS][NUM_OF_LEDS] = {0.0f};
bool connections[NUM_OF_LEDS][NUM_OF_LEDS] = {true};

unsigned int ledPwm[NUM_OF_LEDS] = {0};

void getXY (float* &x, float* &y, unsigned int n);
void getDist (float* x, float* y, float** &dist, unsigned int n);
void getConnect (float** dist, bool** &connect, unsigned int n);

void printData (float* x, float* y, float** dist, bool**  connect, unsigned int n);
void printDisplay (float* x, float* y, unsigned int* ledPwm, unsigned int n);
void printConnect (float* x, float* y, bool** connect, unsigned int n);

void fprintData (FILE* stream, float* x, float* y, float** dist, bool**  connect, unsigned int n);
void fprintDisplay (FILE* stream, float* x, float* y, unsigned int* ledPwm, unsigned int n);
void fprintConnect (FILE* stream, float* x, float* y, bool** connect, unsigned int n);


//////////
// DISPLAY

#define WIDTH  30
#define HEIGHT 10

char screen[WIDTH][HEIGHT] = {' '};
void drawScreen (void);

///////
// MAIN

int main (void) {
	printf("Hello, brave, new World!\n");
	
	
	
	printf("Good-bye, cruel World!\n");
	return 0;
}