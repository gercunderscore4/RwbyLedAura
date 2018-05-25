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

#define REGISTER_SIZE 8
#define SER0_OFFSET 0x00
#define SER1_OFFSET 0x08
#define SER2_OFFSET 0x10
#define SER3_OFFSET 0x18
#define SER4_OFFSET 0x20
#define SER5_OFFSET 0x28
#define SER6_OFFSET 0x30
#define SER7_OFFSET 0x38
#define LED_COUNT 24

// yeah, it's backwards
#define LED_ON  LOW
#define LED_OFF HIGH

uint8_t data[LED_COUNT];

void setLeds (uint8_t* data) {
    digitalWrite(RCLK, LOW);
    for (int i = 0; i < REGISTER_SIZE; i++)  {
        digitalWrite(SER0, data[SER0_OFFSET + i]);
        digitalWrite(SER1, data[SER1_OFFSET + i]);
        digitalWrite(SER2, data[SER2_OFFSET + i]);
        digitalWrite(SER3, data[SER3_OFFSET + i]);
        //digitalWrite(SER4, data[SER4_OFFSET + i]);
        //digitalWrite(SER5, data[SER5_OFFSET + i]);
        //digitalWrite(SER6, data[SER6_OFFSET + i]);
        //digitalWrite(SER7, data[SER7_OFFSET + i]);
        digitalWrite(SRCLK, HIGH);
        digitalWrite(SRCLK, LOW);
    }
    digitalWrite(RCLK, HIGH);
}

void clearLeds (void) {
    digitalWrite(RCLK, LOW);
    for (int i = 0; i < REGISTER_SIZE; i++)  {
        digitalWrite(SER0, LED_OFF);
        //digitalWrite(SER1, LED_OFF);
        //digitalWrite(SER2, LED_OFF);
        //digitalWrite(SER3, LED_OFF);
        //digitalWrite(SER4, LED_OFF);
        //digitalWrite(SER5, LED_OFF);
        //digitalWrite(SER6, LED_OFF);
        //digitalWrite(SER7, LED_OFF);
        digitalWrite(SRCLK, HIGH);
        digitalWrite(SRCLK, LOW);
    }
    digitalWrite(RCLK, HIGH);
}

/******************************************************************************
 * MAIN                                                                       *
 ******************************************************************************/

void setup (void) {
    pinMode(SER0,  OUTPUT);
    pinMode(SER1,  OUTPUT);
    pinMode(SER2,  OUTPUT);
    pinMode(SER3,  OUTPUT);
    pinMode(SER4,  OUTPUT);
    pinMode(SER5,  OUTPUT);
    pinMode(SER6,  OUTPUT);
    pinMode(SER7,  OUTPUT);
    pinMode(RCLK,  OUTPUT);
    pinMode(SRCLK, OUTPUT);

    digitalWrite(SER0,  LOW);
    digitalWrite(SER1,  LOW);
    digitalWrite(SER2,  LOW);
    digitalWrite(SER3,  LOW);
    digitalWrite(SER4,  LOW);
    digitalWrite(SER5,  LOW);
    digitalWrite(SER6,  LOW);
    digitalWrite(SER7,  LOW);
    digitalWrite(RCLK,  LOW);
    digitalWrite(SRCLK, LOW);
    
    for (int i = 0; i < LED_COUNT; i++) {
        data[i] = LED_OFF;
    }

    clearLeds();
    delay(3000);
}

void loop (void) {
    for (int i = 0; i < 24; i++) {
        data[i] = (data[i] == LED_ON) ? LED_OFF : LED_ON;
        setLeds(data);
        delay(1000);
    }
    delay(3000);
}
