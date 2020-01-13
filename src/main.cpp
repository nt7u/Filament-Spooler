// Include the AccelStepper Library
#include <Arduino.h>
#include "main.h"
#include "AccelStepper.h"
#include "MultiStepper.h"

// Create control object instances for each stepper motor and the group, multistepper instance
AccelStepper linStepper(AccelStepper::DRIVER, linStepPin, linDirPin);
AccelStepper spoStepper(AccelStepper::DRIVER, spoStepPin, spoDirPin);
MultiStepper steppers;    // and a MultiStepper instance
long targets[2];

float runSpeed = (float)(((double)(SPOL_STEPS_SEC)) / SPO_LIN_RATIO);

bool home() {
  // bring the linear slide to the stop switch, then just off of it
  bool nearlyHome = false;

  // designate the speed to run at for this linear stepper operation
  linStepper.setSpeed(HOMING_SPEED);
  Serial.println((String)"Linear SLIDE motor speed steps per sec: " + linStepper.speed());

  // if Stop active then try to make it inactive
  if (LIN_STOP_SWITCH_ACTIVE) {
    linStepper.move((long)(LIN_STROKE_STEPS / 5.0f)); // 20% of stroke dist.
    Serial.println((String)"Moving " + linStepper.distanceToGo() + " steps to try clear end-stop switch...");

    linStepper.setSpeed(HOMING_SPEED);
    while (linStepper.distanceToGo() != 0) {
      linStepper.runSpeed();
    }
    Serial.println("...finished driving in this direction.");
  }

  // if still active try moving the other direction
  if (LIN_STOP_SWITCH_ACTIVE) {
    linStepper.move((long)(-LIN_STROKE_STEPS / 5.0f)); // 20% of stroke dist.
    Serial.println((String)"Moving " + linStepper.distanceToGo() + " steps to try clear end-stop switch...");

    linStepper.setSpeed(-HOMING_SPEED);
    while (linStepper.distanceToGo() != 0) {
       linStepper.runSpeed();
    }
    Serial.println("...finished driving in this direction.");
  }

  // if still active then report an error and return with error condition
  if (LIN_STOP_SWITCH_ACTIVE) {
    Serial.println("ERROR: Failed to release end-stop condition so nothing more can be done !");
    return false;
  }
  delay(1000); // wait 1 sec

  // now move until we just 'hit' the stop (move up to 120% of stroke distance)
  linStepper.move((long)(-LIN_STROKE_STEPS * 1.2f));
  Serial.println((String)"Making up to " + linStepper.distanceToGo() + " steps to try activate end-stop switch...");
  linStepper.setSpeed(-HOMING_SPEED);
  while (linStepper.distanceToGo() != 0) {
    linStepper.runSpeed();
    if (LIN_STOP_SWITCH_ACTIVE) {
      Serial.println("End-stop was located. :-)");
      nearlyHome = true; // flag success
      break;             // ...and exit while() loop to stop moving further
    }
  }

  delay(1000); // 1.0 second pause

  // then move a safe distance of 5mm away that is designated as "home"
  if (nearlyHome) {
    linStepper.move((long)(HOME_DISTANCE));
    Serial.println((String)"Moving away " + (HOME_DISTANCE/LIN_STEPS_PER_MM) + "mm from stop to the HOME position");
    linStepper.setSpeed(HOMING_SPEED);
    while (linStepper.distanceToGo() != 0) {
      linStepper.runSpeed();
    }

    Serial.println("Linear slide in HOME position");
  }
  else {  // didn't 'hit' switch so report an error and return with error status
    Serial.println("ERROR: End-stop was NOT located.  Stopped due to this failure !");
    return false;
  }
  return true; // if we made it here than all is well so return 'true'
}

void prepMotorsMoves() {
    steppers.moveTo(targets); // set motors to move to absolute position designated in the 'targets' array
    Serial.println((String)"  Linear steps: " + linStepper.distanceToGo() + "\r\n  Rotational steps:" + spoStepper.distanceToGo());    

    if (linStepper.distanceToGo() >= 0)
      linStepper.setSpeed(runSpeed);  // Forward is +
    else
      linStepper.setSpeed(-runSpeed); // Reverse is -

    Serial.println((String)"Linear speed: " + linStepper.speed() + " (" + ((float)(linStepper.distanceToGo()) / linStepper.speed()) + " seconds)");

    spoStepper.setSpeed(SPOL_STEPS_SEC);
    Serial.println((String)"Spool speed: " + spoStepper.speed() + " (" + ((float)(spoStepper.distanceToGo()) / spoStepper.speed()) + " seconds)");
}

void slewMotors() {
  Serial.println("Slewing motors...");
  while (steppers.run()) {
//    Serial.print((String)"Pos0:" + linStepper.currentPosition() + "  Pos1:" + spoStepper.currentPosition() + "     \r");
    while (RUN_SWITCH_INACTIVE) {
      __asm__("nop\n\t");   // do nothing while paused
    }

    if (LIN_STOP_SWITCH_ACTIVE) {
      Serial.println("ERROR: Linear slide hit a stop.  Stopped due to this failure !");
      break;
    }
  }
}

void phase1() {
  // Layer #1 of spool wrap - designate the target locations for the two steppers
  spoStepper.setCurrentPosition(0); // make spool move relative
  targets[0] = (long)((SPOOL_ROTATIONS - 0.25) * SPO_ROT_STEPS);
  targets[1] = (long)((SPOOL_ROTATIONS - 0.25) * LIN_STEPS_FILAMENT);
  prepMotorsMoves();  // set target destination
  slewMotors();       // loop to generate movement
  Serial.println("--- Finished layer 1 of filament ---");
}

void phase2() {    
  spoStepper.setCurrentPosition(0); // make spool move relative    
  targets[0] = (long)(0.25 * SPO_ROT_STEPS);  // 1/4 rotation further
  targets[1] = (long)((SPOOL_ROTATIONS - 0.5) * LIN_STEPS_FILAMENT);
  prepMotorsMoves();  // set target destination
  slewMotors();       // loop to generate movement
  Serial.println("--- Transitioned for layer 2 start ---");
}

void phase3() {
  spoStepper.setCurrentPosition(0); // make spool move relative    
  targets[0] = (long)((SPOOL_ROTATIONS - 0.75) * SPO_ROT_STEPS);
  targets[1] = (long)(0.25 * LIN_STEPS_FILAMENT);
  prepMotorsMoves();  // set target destination
  slewMotors();       // loop to generate movement
  Serial.println("--- Finished layer 2 of filament ---");
}

void phase4() {
  // Layer #4 of spool wrap - return the two steppers to the home position
  spoStepper.setCurrentPosition(0); // make spool move relative    
  targets[0] = (long)(0.25 * SPO_ROT_STEPS);  // 1/4 rotation further
  targets[1] = 0L;
  prepMotorsMoves();  // set target destination
  slewMotors();       // loop to generate movement
  Serial.println("--- Transitioned for layer 1 start ---");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Spooler sketch has started...");

  // set up the switch inputs
  pinMode(linStopSw, INPUT_PULLUP);  // for the linear slide stop
  pinMode(startResum, INPUT_PULLUP); // for the run / stop switch

  // set up and make motors active
  pinMode(enMotorPin, OUTPUT);
  digitalWrite(enMotorPin, MOTORS_ENABLED);

  // set the maximum speed, acceleration factor, initial speed
  spoStepper.setMaxSpeed(SPO_MAX_STEP_SPEED); // todo: come up with real way to get reasonable values
  spoStepper.setAcceleration(50);
  linStepper.setMaxSpeed(LIN_MAX_STEP_SPEED); // todo: come up with real way to get reasonable values
  linStepper.setAcceleration(50);

  // add both steppers to the MultiStepper instance
  steppers.addStepper(spoStepper);  // = steppers[0] as we added it next - it's on X-axis drive
  steppers.addStepper(linStepper);  // = steppers[1] as we added it first - it's on Y-axis drive
  
  //*****************************
  //*** HOME THE LINEAR SLIDE ***
  //*****************************
  while (1) {
    if (home())
      break;
  }
  linStepper.setCurrentPosition(0);   // Zero out as this is "home"
  spoStepper.setCurrentPosition(0);   // Zero out as this is "home"

  Serial.println("");
  Serial.println("Home location was found.  Starting spooling operation.");
  Serial.println((String)"Ratio of motors -> Spool:Linear: " + SPO_LIN_RATIO);
  Serial.println("");
  Serial.println("--- Starting layer 1 of filament ---");
}

void loop() {
  phase1();
  //delay(3000); // 3.0 second pause
  phase2();
  //delay(3000); // 3.0 second pause
  phase3();
  //delay(3000); // 3.0 second pause
  phase4();
  //delay(3000); // 3.0 second pause
}
