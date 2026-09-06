#include "Particle.h"
#include <SparkFun_VL53L1X.h>

SYSTEM_MODE(AUTOMATIC);

////////////////////////////////////////
// GLOBAL CONSTANTS
////////////////////////////////////////
const int angleInit  = 90;
const int servoSpeed = 100;         // Delay between steps
const int thresholdDebounce = 10;    // How many cycles above the threshold before we consider it a hit


////////////////////////////////////////
// PINS
////////////////////////////////////////
const pin_t ledPin = D7;
const pin_t servoPin = D2;  // Pin must be PWM

////////////////////////////////////////////////////
// CLOUD VARIABLES
////////////////////////////////////////////////////
int sensorThreshold = 300;  // ~765 when clear
int sensorValue = 0;
int timeout = 10000;
int enable = 1;
int counter = 0;
// int angleOpen = 35;
int angleSize = 15;
int angleClose = 38;


////////////////////////////////////////
// INTERNAL VARIABLES
////////////////////////////////////////
SFEVL53L1X distanceSensor;

Servo myServo;

int aboveThresholdCount = 0;
bool goFlag = false;
unsigned long goStart;


////////////////////////////////////////
// FUNCTIONS
////////////////////////////////////////
void tapOpen() {
    Particle.publish("log", "Opening Tap", PRIVATE);
    digitalWrite(ledPin, HIGH);
    int angleStart = myServo.read();
    int angleOpen = angleClose - 20 + angleSize;
    int angleStop  = angleStart - angleOpen;
    int angle = angleStart;
    while (angle >= angleStop) {
        myServo.write(angle);
        angle--;
        delay(servoSpeed);
    }
    myServo.write(angleStop);
    return;
}

void tapClose() {
    Particle.publish("log", "Closing Tap", PRIVATE);
    int angleStart = myServo.read();
    int angleStop  = angleStart + angleClose;
    int angle = angleStart;
    while (angle <= angleStop) {
        myServo.write(angle);
        angle++;
        delay(servoSpeed);
    }
    angleStart = myServo.read();
    angleStop  = angleInit;
    angle = angleStart;
    while (angle >= angleStop) {
        myServo.write(angle);
        angle--;
        delay(servoSpeed);
    }
    myServo.write(angleInit);
    digitalWrite(ledPin, LOW);
    return;
}


////////////////////////////////////////
// CLOUD FUNCTIONS
////////////////////////////////////////

int setEnable(const char *data) {
    if (String(data) != "") {
        int value = atoi(data);
        if (value > 0) enable = 1;
        if (value == 0)  enable = 0;
    }
    return enable;
}

int setThreshold(const char *data) {
    if (String(data) != "") {
        int value = atoi(data);
        if (value > 20) sensorThreshold = float(value);
    }
    return sensorThreshold;
}

int setTimeout(const char *data) {
    if (String(data) != "") {
        int value = atoi(data);
        if (value > 1000) timeout = value;
    }
    return timeout;
}

int setAngle(const char *data) {
    if (String(data) != "") {
        int value = atoi(data);
        if (value > 1) angleSize = value;
    }
    return angleSize;
}

int resetCounter(const char *data) {
    if (String(data) != "") {
        int value = atoi(data);
        if (value >= 0) counter = value;
    } else {
        counter = 0;
    }
    return counter;
}


////////////////////////////////////////
// SETUP
////////////////////////////////////////
void setup() {
    
    Particle.function("setEnable",      setEnable);
    Particle.function("setThreshold",   setThreshold);
    Particle.function("setTimeout",     setTimeout);
    Particle.function("setAngle",       setAngle);
    Particle.function("resetCounter",   resetCounter);
    
    Particle.variable("Enabled",        enable);
    Particle.variable("sensorValue",    sensorValue);
    Particle.variable("runCounter",     counter);
    Particle.variable("timeout",        timeout);
    Particle.variable("angleSize",      angleSize);
    Particle.variable("sensorThreshold",sensorThreshold);
    
    pinMode(ledPin, OUTPUT);

    // Init sensor
    if (!Wire.isEnabled()) Wire.begin();
    if (distanceSensor.begin() != 0) {
        Particle.publish("err", "Could not find a valid VL53L1X sensor, check wiring!", PRIVATE);
        while (1);
    }
    delay(1000); // let sensor boot up
  
    // Init Servo motor
    myServo.attach(servoPin);
    delay(100);
    myServo.write(angleInit);
    delay(3000);
  
    // Done
    Particle.publish("log", "Ready!", PRIVATE);
}


////////////////////////////////////////
// LOOP
////////////////////////////////////////
void loop() {
    
    // Process sensor
    distanceSensor.startRanging();
    while (!distanceSensor.checkForDataReady()) delay(1);
    sensorValue = distanceSensor.getDistance();
    distanceSensor.clearInterrupt();
    distanceSensor.stopRanging();
    if (sensorValue < sensorThreshold) aboveThresholdCount++;
    
    // Process results
    if (enable && aboveThresholdCount > thresholdDebounce) {
        if (!goFlag) {
            Particle.publish("log", "Sensor: "+String(sensorValue), PRIVATE);
            tapOpen();
            counter++;
            goFlag = true;
        }
        aboveThresholdCount = 0;
        goStart = millis();
    }
    if (goFlag && (millis() > goStart + timeout)) {
        tapClose();
        goFlag = false;
    }
    
    delay(1);
}
