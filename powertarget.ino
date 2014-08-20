// PowerTarget
// Author: Brian Critchlow
// Based on code by BDub @ Technobly.com
//
// All inputs are pulled high with internal pullups.
// GND D2 to reset and run the timer for all lanes.
// GND A6, A7, D3 and D4 to stop the timer for each
// lane and display the results.
//
// D0-D3 were initially chosen, but because
// individual control of dis/en'abling their
// interrupt handlers was not available, A6, A7
// D3 and D4 were chosen instead.  D0, D1, D2
// A0, A1, A3, A4 are all tied to one interrupt
// handler, so for the one remaining input any
// of these will do.  D2 was chosen to allow
// for the most flexibility of the remaining inputs.
//
// EXTI_ClearITPendingBit() is necessary to clear
// the interrupt Pending register, or interrupts
// will fire immediately after enabling the
// interrupt handlers again.
//
// DWT->CYCCNT was used for the timer instead of
// micros() which wraps this hardware counter and
// returns the number of microseconds associated
// with it; because it's not clean to handle the
// case where micros() wraps at 59.652323 seconds.
// This give us 72x more resolution on the timing
// anyway and it's super easy to deal with wrapping.
// It just works out through subtraction of unsigned
// 32-bit variables; as long as you don't time
// something longer than 59.652323 seconds.
//
//==================================================

#include "application.h"
#include "neopixel/neopixel.h"

#define TRIGGER_GAME_PIN 7
//Set pixel COUNT, PIN and TYPE
#define PIXEL_COUNT 1
#define PIXEL_TYPE WS2812B
#define PIXEL0_PIN A0
#define PIXEL1_PIN A1
#define PIXEL2_PIN A2
#define PIXEL3_PIN A3

#define PIEZO_PIN D0

//Construct pixels
Adafruit_NeoPixel strip0 = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL0_PIN, PIXEL_TYPE);
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL1_PIN, PIXEL_TYPE);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL2_PIN, PIXEL_TYPE);
Adafruit_NeoPixel strip3 = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL3_PIN, PIXEL_TYPE);

//void startRace(void);
void lane1(void);
void lane2(void);
void lane3(void);
void lane4(void);
bool showStartMsg = false;
bool raceEnded = true; // start off assuming the race has not started.
volatile uint32_t timeStart1;
volatile uint32_t timeStart2;
volatile uint32_t timeStart3;
volatile uint32_t timeStart4;
volatile uint32_t timeEnd1;
volatile uint32_t timeEnd2;
volatile uint32_t timeEnd3;
volatile uint32_t timeEnd4;

double tempTime1;
double tempTime2;
double tempTime3;
double tempTime4;

char target1Time[10];
char target2Time[10];
char target3Time[10];
char target4Time[10];

uint32_t startTime;
const uint32_t DISQUALIFIED_TIME = 10 * 1000;  // in milliseconds (10 seconds)

void setup() {
    strip0.begin();
    strip0.setPixelColor(0, 0, 0,0);
    strip0.show(); // Initialize all pixels to 'off'
    strip1.begin();
    strip1.setPixelColor(0, 0, 0,0);
    strip1.show(); // Initialize all pixels to 'off'
    strip2.begin();
    strip2.setPixelColor(0, 0, 0,0);
    strip2.show(); // Initialize all pixels to 'off'
    strip3.begin();
    strip3.setPixelColor(0, 0, 0,0);
    strip3.show(); // Initialize all pixels to 'off'

    //setup pins
    pinMode(D0, OUTPUT); // piezo
    pinMode(D7, OUTPUT); // startRace LED
    //pinMode(D2, INPUT); // startRace trigger
    pinMode(A6, INPUT); // lane1
    pinMode(A7, INPUT); // lane2
    pinMode(D3, INPUT); // lane3
    pinMode(D4, INPUT); // lane4

    //setup actions to perform on interrupts
    //attachInterrupt(D2, startRace, FALLING); // startRace
    attachInterrupt(A6, lane1, FALLING); // lane1
    attachInterrupt(A7, lane2, FALLING); // lane2
    attachInterrupt(D3, lane3, FALLING); // lane3
    attachInterrupt(D4, lane4, FALLING); // lane4

    //Stop listening to interrupts on all lanes
    NVIC_DisableIRQ(EXTI0_IRQn); // A6 "Lane 1"
    NVIC_DisableIRQ(EXTI1_IRQn); // A7 "Lane 2"
    NVIC_DisableIRQ(EXTI4_IRQn); // D3 "Lane 3"
    NVIC_DisableIRQ(EXTI3_IRQn); // D4 "Lane 4"

    //seed for randomizer
    uint32_t seed = millis();
    srand(seed); // void srand(unsigned int seed);

    Spark.function("startround", cloudStartRound);
    Spark.function("reset", cloudReset);

    Serial.begin(115200);
    //while(!Serial.available()) SPARK_WLAN_Loop(); // Waiting for user to open terminal and press ENTER
    // Enter waiting state, waiting for D2 to go low.
    Serial.println("================================");
    Serial.println("Waiting for race to start.");
}

void loop() {
  //wait for race to end
  if(!raceEnded) {
    //wait for all lanes to have times (this includes any empty lanes, which will timeout at 20 seconds)
    if((timeEnd1 != timeStart1 && timeEnd2 != timeStart2 && timeEnd3 != timeStart3 && timeEnd4 != timeStart4) || (millis() - startTime) > DISQUALIFIED_TIME) {

        Serial.println("Round Finished!");
        Serial.println("= Shot Times in seconds =");

        //output lane times
        Serial.print("Target 1: ");
        tempTime1 = (double)(timeEnd1 - timeStart1)/72000000.0;
        if(tempTime1 != 0.0) {
            Serial.println(tempTime1,8);
            sprintf(target1Time, "%f", tempTime1);
            Spark.publish("round/target1",target1Time);
        } else Serial.println("MISS");
        delay(50);
        Serial.print("Target 2: ");
        tempTime2 = (double)(timeEnd2 - timeStart2)/72000000.0;
        if(tempTime2 != 0.0) {
            Serial.println(tempTime2,8);
            sprintf(target2Time, "%f", tempTime2);
            Spark.publish("round/target2",target2Time);
        } else Serial.println("MISS");
        delay(50);
        Serial.print("Target 3: ");
        tempTime3 = (double)(timeEnd3 - timeStart3)/72000000.0;
        if(tempTime3 != 0.0) {
            Serial.println(tempTime3,8);
            sprintf(target3Time, "%f", tempTime3);
            Spark.publish("round/target3",target3Time);
        } else Serial.println("MISS");
        delay(50);
        Serial.print("Target 4: ");
        tempTime4 = (double)(timeEnd4 - timeStart4)/72000000.0;
        if(tempTime4 != 0.0) {
            Serial.println(tempTime4,8);
            sprintf(target4Time, "%f", tempTime4);
            Spark.publish("round/target4",target4Time);
        } else Serial.println("MISS");
        delay(50);



        //cleanup
        raceEnded = true; // prevents results from being displayed over and over
        EXTI_ClearITPendingBit(EXTI_Line5); // D2 "startRace"
        NVIC_EnableIRQ(EXTI9_5_IRQn); // D2
        digitalWrite(TRIGGER_GAME_PIN, LOW); //lower trigger led
        // Enter waiting state, waiting for D2 to go low.
        Serial.println("================================");
        Serial.println("Waiting for round to start.");
    }
  }

  if(showStartMsg) {
    int randomDelay = random(500,2000);
    delay(randomDelay);
    strip0.setPixelColor(0, 0, 255,0);
    strip0.show(); // Initialize all pixels to 'off'
    strip1.setPixelColor(0, 0, 255,0);
    strip1.show(); // Initialize all pixels to 'off'
    strip2.setPixelColor(0, 0, 255,0);
    strip2.show(); // Initialize all pixels to 'off'
    strip3.setPixelColor(0, 0, 255,0);
    strip3.show(); // Initialize all pixels to 'off'


    NVIC_DisableIRQ(EXTI9_5_IRQn); // stop listening for an interrupt on D2
    digitalWrite(PIEZO_PIN, HIGH);
    timeStart1 = DWT->CYCCNT; // get the start time for each lane
    timeStart2 = timeStart1; // set the start time for each line the same
    timeStart3 = timeStart1; // set the start time for each line the same
    timeStart4 = timeStart1; // set the start time for each line the same
    timeEnd1 = timeStart1; // set the end time to the start time temporarily
    timeEnd2 = timeStart1; // set the end time to the start time temporarily
    timeEnd3 = timeStart1; // set the end time to the start time temporarily
    timeEnd4 = timeStart1; // set the end time to the start time temporarily
    EXTI_ClearITPendingBit(EXTI_Line0); // Reset the interrupt for A6 "Lane 1"
    EXTI_ClearITPendingBit(EXTI_Line1); // Reset the interrupt for A7 "Lane 2"
    EXTI_ClearITPendingBit(EXTI_Line4); // Reset the interrupt for D3 "Lane 3"
    EXTI_ClearITPendingBit(EXTI_Line3); // Reset the interrupt for D4 "Lane 4"
    NVIC_EnableIRQ(EXTI0_IRQn); // Start listening for an interrupt on A6 "Lane 1"
    NVIC_EnableIRQ(EXTI1_IRQn); // Start listening for an interrupt on A7 "Lane 2"
    NVIC_EnableIRQ(EXTI4_IRQn); // Start listening for an interrupt on D3 "Lane 3"
    NVIC_EnableIRQ(EXTI3_IRQn); // Start listening for an interrupt on D4 "Lane 4"
    delay(500);
    digitalWrite(PIEZO_PIN, LOW);

    startTime = millis(); // Capture the rough start time, for disqualification timer
    Serial.println("================================");
    Serial.println("================================");
    Serial.println("================================");
    Serial.println("================================");
    Serial.println("================================");
    Serial.println("================================");
    Serial.println("================================");
    Serial.println("================================");
    Serial.println("================================");
    Serial.println("================================");
    Serial.println("================================");
    Serial.println("================================");
    Serial.println("================================");
    Serial.println("================================");
    Serial.println("================================");
    Serial.println("Round Started!");
    Serial.print("Shot Delay is:");
    Serial.println(randomDelay);
    showStartMsg = false;
    raceEnded = false;

  }
}

void lane1() {
    NVIC_DisableIRQ(EXTI0_IRQn); // stop listening for an interrupt on A6
    timeEnd1 = DWT->CYCCNT; //get the lane time (instead of using micros();)
    strip0.setPixelColor(0, 255, 0,0);
    strip0.show();
}

void lane2() {
    NVIC_DisableIRQ(EXTI1_IRQn); // stop listening for an interrupt on A7
    timeEnd2 = DWT->CYCCNT; //get the lane time (instead of using micros();)
    strip1.setPixelColor(0, 255, 0,0);
    strip1.show();
}

void lane3() {
    NVIC_DisableIRQ(EXTI4_IRQn); // stop listening for an interrupt on D3
    timeEnd3 = DWT->CYCCNT; //get the lane time (instead of using micros();)
    strip2.setPixelColor(0, 255, 0,0);
    strip2.show();
}

void lane4() {
    NVIC_DisableIRQ(EXTI3_IRQn); // stop listening for an interrupt on D4
    timeEnd4 = DWT->CYCCNT; //get the lane time (instead of using micros();)
    strip3.setPixelColor(0, 255, 0,0);
    strip3.show();
}

int random(int minTime, int maxTime) {
    // int rand(void); included by default from newlib
    return rand() % (maxTime-minTime+1) + minTime;
}

int cloudStartRound(String args) {
    showStartMsg = true; //indicate that the race has started
    return 0;
}
//void startRace() {
//    showStartMsg = true; //indicate that the race has started
//}
int cloudReset(String args) {
    strip0.setPixelColor(0, 0, 0,0);
    strip0.show(); // Initialize all pixels to 'off'
    strip1.setPixelColor(0, 0, 0,0);
    strip1.show(); // Initialize all pixels to 'off'
    strip2.setPixelColor(0, 0, 0,0);
    strip2.show(); // Initialize all pixels to 'off'
    strip3.setPixelColor(0, 0, 0,0);
    strip3.show(); // Initialize all pixels to 'off'
    return 0;
}
