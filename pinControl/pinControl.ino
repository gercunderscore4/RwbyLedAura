// switches
#define SW0 11
#define SW1 12

#define SW_DEBOUNCE 500

// sensors
#define SEN_R A0
#define SEN_L A1

#define SEN_THRESH 100
#define SEN_TRIGGER_TIME 2000

// pins
#define SER0 7
#define SER1 6
#define SER2 5
#define SER3 4
#define SER4 3
#define SER5 2
#define SER6 1
#define SER7 0
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
#define LED_COUNT 32

// yeah, it's backwards
#define LED_ON  LOW
#define LED_OFF HIGH

uint8_t data[LED_COUNT];
uint8_t pwm[LED_COUNT];

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

void clearLedData (uint8_t* data) {
    for (int i = 0; i < LED_COUNT; i++) {
        data[i] = LED_OFF;
    }
}

void clearLeds (uint8_t* data) {
    clearLedData(data);
    setLeds(data);
}

/******************************************************************************
 * ALGORITHMS                                                                 *
 ******************************************************************************/

void linearLedLoop (uint8_t* data) {
    for (int j = 0; j < LED_COUNT; j++) {
        clearLedData(data);
        data[j] = LED_ON;
        setLeds(data);
    }
}

void binaryLedLoop (uint8_t* data) {
    for (int j = 0; j < LED_COUNT; j++) {
        if (data[j] == LED_ON) {
            data[j] = LED_OFF;
        } else {
            data[j] = LED_ON;
            break;
        }
    }
    setLeds(data);
}

void randomLeds (uint8_t* data) {
    for (int i = 0; i < LED_COUNT; i++) {
        data[i] = random(2) ? LED_ON : LED_OFF;
    }
    setLeds(data);
}

void dynamicRandomLeds (uint8_t* data, long int oneIn) {
    for (int i = 0; i < LED_COUNT; i++) {
        if (random(oneIn) == 0) {
            //data[i] = (data[i] == LED_ON) ? LED_OFF : LED_ON;
            data[i] = random(2) ? LED_OFF : LED_ON;
        }
    }
    setLeds(data);
}

void basicSensorTriggerLoop (uint8_t* data) {
    int sen_r_val = 0;
    int sen_l_val = 0;
    int sen_time = 0;
    while (true) {
        sen_r_val = analogRead(SEN_R);
        sen_l_val = analogRead(SEN_L);
        if ((sen_r_val < SEN_THRESH) ||
            (sen_l_val < SEN_THRESH)) {
            sen_time = millis();
        }

        if (millis() < (sen_time + SEN_TRIGGER_TIME)) {
            dynamicRandomLeds(data, 8);
        } else {
            clearLeds(data);
        }

        delay(10);
    }
}

/******************************************************************************
 * MAIN                                                                       *
 ******************************************************************************/

void setup (void) {
    pinMode(SW0,  INPUT);
    pinMode(SW1,  INPUT);

    pinMode(SEN_R,  INPUT);
    pinMode(SEN_L,  INPUT);

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
    
    clearLeds(data);
}

void loop (void) {
    //dealy(100);
    //randomLeds(data);
    //binaryLedLoop(data);
    //randomLeds(data, 50);
    //dyanmicRandomLeds(data, 20, 5);
    basicSensorTriggerLoop(data);
}

