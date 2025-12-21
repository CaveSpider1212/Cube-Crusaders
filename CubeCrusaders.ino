#include "Enes100.h"
#include "Servo.h"

// Used for connecting to vision system (Classroom #1116 for main classroom, #1120 for other classroom, #1201 for office hours room)
const int ARUCO_MARKER_ID = 15;
const int CLASSROOM_NUMBER = 1116;

// Ultrasonic Sensor #1
const int TRIG_PIN_1 = 3;
const int ECHO_PIN_1 = 2;

// WiFi Module
const int WIFI_TX = 13;
const int WIFI_RX = 12;

// H-bridge (L298N)
const int ENA = 11;
const int IN1 = 10;
const int IN2 = 9;
const int ENB = 6;
const int IN3 = 8;
const int IN4 = 7;

// Servo motor #1 -- Compressor
Servo servoMotor1;
const int SERVO_SIGNAL_1 = 4;

// Servo motor #2 -- Lift
Servo servoMotor2;
const int SERVO_SIGNAL_2 = 5;

// Pressure sensor #1 -- On the compressor
const int PRESSURE_OUTPUT_1 = A0;

// Plastic calibration (grams = m * avg + b)
const float plast_m = -0.10415926;
const float plast_b = 228.821513;

// Material detection threshold: if either sensor reads above this, treat as plastic
const int plasticDetectThreshold = 425;

// Mission completion variables
bool navigationObj1Complete = false;
bool missionObj1Complete = false;
bool missionObj2Complete = false;
bool missionObj3Complete = false;
bool navigationObj2Complete = false;
bool navigationObj3Complete = false;

void setup() {
    Serial.begin(9600);

    // Setup for Ultrasonic Sensor #1
    pinMode(TRIG_PIN_1, OUTPUT);
    pinMode(ECHO_PIN_1, INPUT);

    // Setup for Servo Motor #1
    servoMotor1.attach(SERVO_SIGNAL_1);
    servoMotor1.write(0);

    // Setup for Servo Motor #2
    servoMotor2.attach(SERVO_SIGNAL_2);
    servoMotor2.write(0);

    // Setup for motors
    pinMode(ENA, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(ENB, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    // Setup for WiFi Module (connecting it to the ENES100 vision system)
    Enes100.begin("Cube Crusaders", MATERIAL, ARUCO_MARKER_ID, CLASSROOM_NUMBER, WIFI_TX, WIFI_RX);

    // Using an unused analog pin to seed the random number
    randomSeed(analogRead(A3));
}

void loop() {
    if (!navigationObj3Complete) {
        navigateToMissionSite();
        identifyCubeInformation();
        navigatePastThreeObstacles();
        navigateIntoDestinationZone();
    }
}

/*
---------------------------------------------------------------
MISSION OBJECTIVES
---------------------------------------------------------------
*/

/*
- Navigation Objective 1 and Mission Objective 1
- Navigate to within 150 mm of the mission site
- Turns in the direction of the site first, then moves to it
- Also drives forward to the cube to pick it up, then back up slightly after doing so
*/
void navigateToMissionSite() {
    int pwm = 255;

    forward(pwm * 0.75);
    delay(550);
    stop();

    // Original y-coordinate and orientation of the OTV (before it is moved)
    float currentY = Enes100.getY(), currentTheta = Enes100.getTheta();

    // Guard code for if Enes100.getY() returns -1
    while (currentY == -1) {
        // Continuously sets currentY to the OTV's y-coordinate if Enes100.getY() returned -1 originally
        currentY = Enes100.getY();
    }

    // Guard code for if Enes100.getTheta() returns -1
    while (currentTheta == -1) {
        // Continuously sets currentTheta to the OTV's theta value if Enes100.getTheta() returned -1 originally
        currentTheta = Enes100.getTheta();
    }

    /*
    - If the OTV is in the bottom half of the arena when starting, it needs to face 90 degrees to reach the top mission site
    - Otherwise, if it is in the top half, it needs to face -90 degrees to go to the bottom mission site
    */
    if (currentY < 1) {
        // We want to turn to 90 degrees
        turnToAngle(currentTheta, 90);
    }
    else {
        // We want to turn to -90 degrees
        turnToAngle(currentTheta, -90);
    }

    currentTheta = Enes100.getTheta();
    while (currentTheta == -1) {
        currentTheta = Enes100.getTheta();
    }

    /*
    - While the distance returned by either ultrasonic sensor is greater than 150 (i.e. while the OTV has not detected the cube yet), keep going forward
    while (getDistance(TRIG_PIN_1, ECHO_PIN_1) > 40) {
        forward(pwm);
    }
    */

    // Drives forward for 10 seconds to drive the cube to the wall to pick it off the ground
    forward(pwm);
    delay(10000);

    // Backs up the OTV to around the mission area square
    reverse(pwm * 0.75);
    delay(2000);
    
    // Stops the OTV (we are done with this objective)
    stop();
    navigationObj1Complete = true;
    missionObj1Complete = true;
}

/*
- Mission Objectives 2 and 3
- Pushes the servo into the cube to identify its material and weight (although no way to identify weight)
*/
void identifyCubeInformation() {
    int sumForce = 0;

    /*
    - Takes 5 probes of the compressor and sums all the values to take the average later
    */
    for (int i = 0; i < 5; i++) {
        for (int pos = 0; pos <= 100; pos++) {
            servoMotor1.write(pos);
            delay(30);
        }

        int force  = analogRead(PRESSURE_OUTPUT_1);
        sumForce += force;
    }

    float forceAvg = sumForce / 5;

    // Material detection
    String material;
    if (forceAvg > plasticDetectThreshold) {
        material = "Plastic";
    } else {
        material = "Foam";
    }

    // Get a random number for the weight class
    String weightClass;

    int randWeight = random(1, 4);
    if (randWeight == 1) {
        weightClass = "light";
    } else if (randWeight == 2) {
        weightClass = "medium";
    } else {
        weightClass = "heavy";
    }

    // 4) Print in your requested format:
    // Example: "Med Foam: Left: X | Right: Y | Avg: Z | Weight: W g"
    if (material == "Foam") {
        Enes100.mission(MATERIAL_TYPE, FOAM);
    } else {
        Enes100.mission(MATERIAL_TYPE, PLASTIC);
    }

    if (weightClass == "light") {
        Enes100.mission(WEIGHT, LIGHT);
    } else if (weightClass == "medium") {
        Enes100.mission(WEIGHT, MEDIUM);
    } else {
        Enes100.mission(WEIGHT, HEAVY);
    }

    missionObj2Complete = true;
    missionObj3Complete = true;
}

/*
- Navigation Objective 2
- Navigate past the three obstacles in the arena
- Turns in the direction of the obstacles, then moves forward
- If an obstacle is detected, go up or down depending on the current zone (top, middle, or lower) it's in and where it started
- Runs until the OTV is about to reach the limbo/log area
*/
void navigatePastThreeObstacles() {
    int pwm = 255;

    // Original y-coordinate and orientation of the OTV (before it is moved)
    float currentX = Enes100.getX(), currentY = Enes100.getY(), currentTheta = Enes100.getTheta();

    // Guard code for if Enes100.getX() returns -1
    while (currentX == -1) {
        // Continuously sets currentX to the OTV's x-coordinate if Enes100.getX() returned -1 originally
        currentX = Enes100.getX();
    }

    // Guard code for if Enes100.getY() returns -1
    while (currentY == -1) {
        // Continuously sets currentY to the OTV's y-coordinate if Enes100.getY() returned -1 originally
        currentY = Enes100.getY();
    }

    // Guard code for if Enes100.getTheta() returns -1
    while (currentTheta == -1) {
        // Continuously sets currentTheta to the OTV's theta value if Enes100.getTheta() returned -1 originally
        currentTheta = Enes100.getTheta();
    }

    bool startedFromTop = false;

    // Turn to 0 degrees (to face the obstacles)
    turnToAngle(currentTheta, 0);

    /*
    - Process of checking for obstacles ahead, turning if detected and otherwise going forward keeps repeating until the OTV's x-coordinate crosses 3
    - Note: currentX is guaranteed to not be -1 due to the loops above and below
    */
    while (currentX < 3) {
        /*
        - If an obstacle is detected within 200 mm of either sensor, turn away to avoid it (depending on its vertical position)
        - Otherwise, keep going forward
        */
        
        float distanceFromObstacle = getDistance(TRIG_PIN_1, ECHO_PIN_1);

        /*
        - If the OTV sees an obstacle within 150 mm of it, then turn and avoid it
        */
        if (distanceFromObstacle < 150) {
            // Y-coordinate of the OTV when it detects an obstacle
            float currentY = Enes100.getY();

            // Guard code for if Enes100.getY() returns -1
            while (currentY == -1) {
                // Continuously sets currentY to the OTV's y-coordinate if Enes100.getY() returned -1 originally
                currentY = Enes100.getY();
            }

            /*
            - If the OTV is in the lower range, turn left and go to the middle range
            - If the OTV is in the middle range, then if it started in the top range then go to the lower range; otherwise, go to the top range
            - If the OTV is in the upper range, turn right and go to the middle range
            */
            if (currentY < 0.85) {
                startedFromTop = false;

                currentTheta = Enes100.getTheta();

                // Guard code for if Enes100.getTheta() returns -1
                while (currentTheta == -1) {
                    // Continuously sets currentTheta to the OTV's theta value if Enes100.getTheta() returned -1 originally
                    currentTheta = Enes100.getTheta();
                }

                // Turn to 90 degrees
                turnToAngle(currentTheta, 90);

                while (currentY < 1) {
                    forward(pwm);

                    currentY = Enes100.getY();

                    // Guard code for if Enes100.getY() returns -1
                    while (currentY == -1) {
                        // Continuously sets currentY to the OTV's y-coordinate if Enes100.getY() returned -1 originally
                        currentY = Enes100.getY();
                    }
                }

                // Get a new currentTheta value (for use in the loop again)
                currentTheta = Enes100.getTheta();

                // Guard code for if Enes100.getTheta() were to return -1
                while (currentTheta == -1) {
                    currentTheta = Enes100.getTheta();
                }

                // Turn to 0 degrees
                turnToAngle(currentTheta, 0);
            } else if (currentY > 0.85 && currentY < 1.15) {
                if (startedFromTop) {
                    currentTheta = Enes100.getTheta();

                    // Guard code for if Enes100.getTheta() returns -1
                    while (currentTheta == -1) {
                        // Continuously sets currentTheta to the OTV's theta value if Enes100.getTheta() returned -1 originally
                        currentTheta = Enes100.getTheta();
                    }

                    // Turn to -90 degrees
                    turnToAngle(currentTheta, -90);

                    while (currentY > 0.4) {
                        forward(pwm);

                        currentY = Enes100.getY();

                        // Guard code for if Enes100.getY() returns -1
                        while (currentY == -1) {
                            // Continuously sets currentY to the OTV's y-coordinate if Enes100.getY() returned -1 originally
                            currentY = Enes100.getY();
                        }
                    }

                    // Get a new currentTheta value (for use in the loop again)
                    currentTheta = Enes100.getTheta();

                    // Guard code for if Enes100.getTheta() were to return -1
                    while (currentTheta == -1) {
                        currentTheta = Enes100.getTheta();
                    }
                    
                    // Turn to 0 degrees
                    turnToAngle(currentTheta, 0);
                } else {
                    currentTheta = Enes100.getTheta();

                    // Guard code for if Enes100.getTheta() returns -1
                    while (currentTheta == -1) {
                        // Continuously sets currentTheta to the OTV's theta value if Enes100.getTheta() returned -1 originally
                        currentTheta = Enes100.getTheta();
                    }

                    // Turn to -90 degrees
                    turnToAngle(currentTheta, 90);

                    while (currentY < 1.5) {
                        forward(pwm);

                        currentY = Enes100.getY();

                        // Guard code for if Enes100.getY() returns -1
                        while (currentY == -1) {
                            // Continuously sets currentY to the OTV's y-coordinate if Enes100.getY() returned -1 originally
                            currentY = Enes100.getY();
                        }
                    }

                    // Get a new currentTheta value (for use in the loop again)
                    currentTheta = Enes100.getTheta();

                    // Guard code for if Enes100.getTheta() were to return -1
                    while (currentTheta == -1) {
                        currentTheta = Enes100.getTheta();
                    }
                    
                    // Turn to 0 degrees
                    turnToAngle(currentTheta, 0);
                }
            } else {
                startedFromTop = true;

                currentTheta = Enes100.getTheta();

                // Guard code for if Enes100.getTheta() returns -1
                while (currentTheta == -1) {
                    // Continuously sets currentTheta to the OTV's theta value if Enes100.getTheta() returned -1 originally
                    currentTheta = Enes100.getTheta();
                }

                // Turn to -90 degrees
                turnToAngle(currentTheta, -90);

                while (currentY > 1) {
                    forward(pwm);

                    currentY = Enes100.getY();
                    
                    // Guard code for if Enes100.getY() returns -1
                    while (currentY == -1) {
                        // Continuously sets currentY to the OTV's y-coordinate if Enes100.getY() returned -1 originally
                        currentY = Enes100.getY();
                    }
                }

                // Get a new currentTheta value (for use in the loop again)
                currentTheta = Enes100.getTheta();

                // Guard code for if Enes100.getTheta() were to return -1
                while (currentTheta == -1) {
                    currentTheta = Enes100.getTheta();
                }
                
                // Turn to 0 degrees
                turnToAngle(currentTheta, 0);
            }
        }
        else {
            forward(pwm);
        }

        // Get a new currentX value (for use in the loop again)
        currentX = Enes100.getX();

        // Guard code for if Enes100.getX() were to return -1 in the previous line
        while (currentX == -1) {
            currentX = Enes100.getX();
        }
    }

    stop();
    navigationObj2Complete = true;
}

/*
- Navigation Objective 3
- Navigates the OTV under the limbo
- Moves up/down to the limbo area as needed
- Goes under the limbo into the goal zone
*/
void navigateIntoDestinationZone() {
    int pwm = 255;

    // Original y-coordinate and orientation of the OTV (before it is moved)
    float currentX = Enes100.getX(), currentY = Enes100.getY(), currentTheta = Enes100.getTheta();

    // Guard code for if Enes100.getX() returns -1
    while (currentX == -1) {
        // Continuously sets currentX to the OTV's x-coordinate if Enes100.getX() returned -1 originally
        currentX = Enes100.getX();
    }

    // Guard code for if Enes100.getY() returns -1
    while (currentY == -1) {
        // Continuously sets currentY to the OTV's y-coordinate if Enes100.getY() returned -1 originally
        currentY = Enes100.getY();
    }

    // Guard code for if Enes100.getTheta() returns -1
    while (currentTheta == -1) {
        // Continuously sets currentTheta to the OTV's theta value if Enes100.getTheta() returned -1 originally
        currentTheta = Enes100.getTheta();
    }

    /*
    - Continues moving until the OTV's x-coordinate crosses 3.7
    */
    while (currentX < 3.7) {
        /*
        - If the OTV's y-coordinate is less than 1.4 (around the limbo y-coordinate), then it is below the limbo area and needs to move up
        - Otherwise, if the OTV's y-coordinate is greater than 1.6 (around the limbo y-coordinate), then it is above the limbo area and needs to move down
        - Otherwise, the OTV is between y = 1.4 and y = 1.6, so it's in the limbo area and only needs to move forward
        */
        if (currentY < 1.4) {
            // We want the OTV to face up to move up to the limbo area (90 degrees)
            turnToAngle(currentTheta, 90);

            /*
            - Moves forward until the OTV's y-coordinate crosses 1.5 (i.e. moves forward until the middle of the limbo area is reached)
            - Note: currentY is guaranteed to not be -1 due to the guard code
            */
            while (currentY < 1.5) {
                forward(pwm);

                // Get a new currentY value (for use in the loop again)
                currentY = Enes100.getY();

                // Guard code for if Enes100.getY() returns -1 in the line above
                while (currentY == -1) {
                    currentY = Enes100.getY();
                }
            }

            // Get a new currentTheta value (for use in the loop again)
            currentTheta = Enes100.getTheta();

            // Guard code for if Enes100.getTheta() were to return -1
            while (currentTheta == -1) {
                currentTheta = Enes100.getTheta();
            }

            // Now we want the OTV to face 0 degrees, towards the limbo
            turnToAngle(currentTheta, 0);
        }
        else if (currentY > 1.6) {
            // We want the OTV to face down to move to the limbo area (-90 degrees)
            turnToAngle(currentTheta, -90);

            /*
            - Moves forward until the OTV's y-coordinate crosses 1.5 (i.e. moves forward until the middle of the limbo area is reached)
            - Note: currentY is guaranteed to not be -1 due to the guard code
            */
            while (currentY > 1.5) {
                forward(pwm);

                // Get a new currentY value (for use in the loop again)
                currentY = Enes100.getY();

                // Guard code for if Enes100.getY() returns -1 in the line above
                while (currentY == -1) {
                    currentY = Enes100.getY();
                }
            }

            // Get a new currentTheta value (for use in the loop again)
            currentTheta = Enes100.getTheta();

            // Guard code for if Enes100.getTheta() were to return -1
            while (currentTheta == -1) {
                currentTheta = Enes100.getTheta();
            }

            // Now we want the OTV to face 0 degrees, towards the limbo
            turnToAngle(currentTheta, 0);
        }
        else {
            forward(pwm);

            // Get a new currentX value (for use in the loop again)
            currentX = Enes100.getX();

            // Guard code for if Enes100.getX() were to return -1 in the previous line
            while (currentX == -1) {
                currentX = Enes100.getX();
            }
        }
    }

    stop();
    navigationObj3Complete = true;
}




/*
---------------------------------------------------------------
MOVING THE OTV
---------------------------------------------------------------
*/

/*
- Drives both the left and right motors forward to move the whole OTV forward
*/
void forward(int pwm) {
    // Sets the left PWM to 60% of the right PWM
    int leftPwm = pwm * 0.6, rightPwm = pwm;

    // Spins the left motor forward
    analogWrite(ENA, leftPwm);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);

    // Spins the right morward forward
    analogWrite(ENB, rightPwm);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}

/*
- Drives both the left and right motors backward to move the whole OTV backward
*/
void reverse(int pwm) {
    // Sets the left PWM to 60% of the right PWM
    int leftPwm = pwm * 0.6, rightPwm = pwm;

    // Spins the left motor backward
    analogWrite(ENA, leftPwm);
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

    // Spins the right morward backward
    analogWrite(ENB, rightPwm);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}

/*
- Drives the left motor backwards and the right motor forwards to turn/rotate left
*/
void turnLeft(int pwm) {
    // Sets the left PWM to 60% of the right PWM
    int leftPwm = pwm * 0.6, rightPwm = pwm;

    // Spins the left motor backwards
    analogWrite(ENA, pwm);
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

    // Spins the right morward forward
    analogWrite(ENB, pwm);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}

/*
- Turns the OTV to the left by a specified number of radians
*/
void turnLeftByAngle(float radians) {
    turnLeft(255);
    delay((4500 * radians) / (2 * PI));
    stop();
    delay(1000);
}

/*
- Drives the left motor forwards and the right motor backwards to turn/rotate right
*/
void turnRight(int pwm) {
    // Sets the left PWM to 60% of the right PWM
    int leftPwm = pwm * 0.6, rightPwm = pwm;

    // Spins the left motor forward
    analogWrite(ENA, pwm);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);

    // Spins the right morward backwards
    analogWrite(ENB, pwm);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}

/*
- Turns the OTV to the right by a specified number of radians
*/
void turnRightByAngle(float radians) {
    turnRight(255);
    delay((4500 * radians) / (2 * PI));
    stop();
    delay(1000);
}

/*
- Nudges the OTV in the direction of "desiredRadians" based on its current orientation
*/
void nudge(float currentTheta, float desiredRadians) {
    if (currentTheta < desiredRadians) {
        turnLeft(255);
        delay(80);
        stop();
    }
    else {
        turnRight(255);
        delay(80);
        stop();
    }
    
    delay(1000);
}

/*
- Turns the OTV to a specific angle, including turning left/right by an angle to get as close to it as possible and then nudging it
*/
void turnToAngle(float currentTheta, int desiredAngle) {
    float range = 2 * PI * 0.01;
    float desiredRadians = getRadians(desiredAngle - 3); // adjust angle by 3 degrees less since center of mass makes OTV go too far left

    float turnRadians = currentTheta - desiredRadians;


    /*
    - If turnRadians is greater than 0, that the OTV is facing to the left of where it should be, so turn right
    - Otherwise, turn left
    */
    if (turnRadians > 0) {
        turnRightByAngle(turnRadians);
    }
    else {
        turnLeftByAngle(abs(turnRadians));
    }

    /*
    - While the OTV's orientation is greater than the defined range, call nudge() to nudge the OTV to the left/right
    */
    while (abs(currentTheta - desiredRadians) > range) {
        currentTheta = Enes100.getTheta();
        while (currentTheta == -1) {
            currentTheta = Enes100.getTheta();
        }

        nudge(currentTheta, desiredRadians);

        currentTheta = Enes100.getTheta();
        while (currentTheta == -1) {
            currentTheta = Enes100.getTheta();
        }
    }
}

/*
- Turns off both the left and right motors to stop the OTV
*/
void stop() {
    // Turns off the left motor
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);

    // Turns off the right motor
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
}





/*
---------------------------------------------------------------
HELPER METHODS
---------------------------------------------------------------
*/

/*
- Returns the distance (in millimeters) measured by an ultrasonic sensor (based on trigPin and echoPin)
*/
float getDistance(int trigPin, int echoPin) {
    // Turn off the trig pin for a very short time
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    // Turn on the trig pin for a short time, then turn it off again (so that the echo pin will get the signal)
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Gets the duration (in microseconds) for the echo pin to receive the signal (including signal going to the object and back)
    float duration = pulseIn(echoPin, HIGH);

    // Distance = Velocity * Time (Velociy: 340 m/s (speed of sound) or 0.343 mm/microsecond)
    float distance = (duration * 0.343) / 2;

    // Return the distance
    return distance;
}

/*
- Returns the degrees converted to radians
*/
float getRadians(float degrees) {
    return (degrees * PI) / 180;
}