// Include the AccelStepper Library
#include <Arduino.h>
#include "main.h"
#include "AccelStepper.h"
#include "MultiStepper.h"

// Define pin connections
const int spoStepPin = 2; // X driver on CNC shield
const int spoDirPin = 5;  //  "
const int linStepPin = 3; // Y driver on CNC shield
const int linDirPin = 6;  //  "
const int linStopSw = 10; // Y end-stop on CNC shield
const int enMotorPin = 8; // Active LOW enable of all motor drivers

// Create control object instances for each stepper motor and the group, multistepper instance
AccelStepper linStepper(AccelStepper::DRIVER, linStepPin, linDirPin);
AccelStepper spoStepper(AccelStepper::DRIVER, spoStepPin, spoDirPin);
MultiStepper steppers;    // and a MultiStepper instance

bool home()
{
  // bring the linear slide to the stop switch, then just off of it
  bool nearlyHome = false;

  // designate the speed to run at for this linear stepper operation
  linStepper.setSpeed(HOMING_SPEED);
  Serial.println((String)"Linear SLIDE motor speed steps per sec: " + linStepper.speed());

  // if Stop active then try to make it inactive
  if (LIN_STOP_SWITCH_ACTIVE)
  {
    linStepper.move((long)(LIN_STROKE_STEPS / 5.0f)); // 20% of stroke dist.
    Serial.println((String)"Moving " + linStepper.distanceToGo() + " steps to try clear end-stop switch...");

    linStepper.setSpeed(HOMING_SPEED);
    while (linStepper.distanceToGo() != 0)
    {
      linStepper.runSpeed();
    }
    Serial.println("...finished driving in this direction.");
  }

  // if still active try moving the other direction
  if (LIN_STOP_SWITCH_ACTIVE)
  {
    linStepper.move((long)(-LIN_STROKE_STEPS / 5.0f)); // 20% of stroke dist.
    Serial.println((String)"Moving " + linStepper.distanceToGo() + " steps to try clear end-stop switch...");

    linStepper.setSpeed(-HOMING_SPEED);
    while (linStepper.distanceToGo() != 0)
    {
       linStepper.runSpeed();
    }
    Serial.println("...finished driving in this direction.");
  }

  // if still active then report an error and return with error condition
  if (LIN_STOP_SWITCH_ACTIVE)
  {
    Serial.println("ERROR: Failed to release end-stop condition so nothing more can be done !");
    return false;
  }
  delay(1000); // wait 1 sec

  // now move until we just 'hit' the stop (move up to 120% of stroke distance)
  linStepper.move((long)(-LIN_STROKE_STEPS * 1.2f));
  Serial.println((String)"Making up to " + linStepper.distanceToGo() + " steps to try activate end-stop switch...");
  linStepper.setSpeed(-HOMING_SPEED);
  while (linStepper.distanceToGo() != 0)
  {
    linStepper.runSpeed();
    if (LIN_STOP_SWITCH_ACTIVE)
    {
      Serial.println("End-stop was located. :-)");
      nearlyHome = true; // flag success
      break;             // ...and exit while() loop to stop moving further
    }
  }

  delay(1000); // 1.0 second pause

  // then move a safe distance of 5mm away that is designated as "home"
  if (nearlyHome)
  {
    linStepper.move((long)(HOME_DISTANCE));
    Serial.println((String)"Moving away " + (HOME_DISTANCE/LIN_STEPS_PER_MM) + "mm from stop to the HOME position");
    linStepper.setSpeed(HOMING_SPEED);
    while (linStepper.distanceToGo() != 0)
    {
      linStepper.runSpeed();
    }

    Serial.println("Linear slide in HOME position");
  }
  else // didn't 'hit' switch so report an error and return with error status
  {
    Serial.println("ERROR: End-stop was NOT located.  Stopped due to this failure !");
    return false;
  }
  return true; // if we made it here than all is well so return 'true'
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Spooler sketch has started...");

  // set up the switch input for the linear slide stop
  pinMode(linStopSw, INPUT_PULLUP);

  // set up and make motors active
  pinMode(enMotorPin, OUTPUT);
  digitalWrite(enMotorPin, MOTORS_ENABLED);

  // set the maximum speed, acceleration factor, initial speed
  spoStepper.setMaxSpeed(300); // todo: come up with real way to get reasonable values
  spoStepper.setAcceleration(50);
  //spoStepper.setSpeed(STEPS_SEC_SPEED);
  //Serial.println((String)"Spool speed: " + STEPS_SEC_SPEED);

  linStepper.setMaxSpeed(300); // todo: come up with real way to get reasonable values
  linStepper.setAcceleration(50);
  //linStepper.setSpeed(STEPS_SEC_SPEED / SPO_LIN_RATIO);
  //Serial.println((String)"Spool speed: " + (STEPS_SEC_SPEED / SPO_LIN_RATIO));

  // add both steppers to the MultiStepper instance
  steppers.addStepper(linStepper);
  steppers.addStepper(spoStepper);
}

void loop()
{
  long targets[2];  
  
  if (home()) // if able to find the home position, then proceed to spooling filament
  {
    linStepper.setCurrentPosition(0);
    spoStepper.setCurrentPosition(0);
    
    Serial.println("");
    Serial.println((String)"Ratio of motors -> Spool:Linear: " + SPO_LIN_RATIO);

    // Layer #1 of spool wrap - designate the target locations for the two steppers
    targets[0] = (long)((SPOOL_WIDTH_MM * LIN_STEPS_PER_MM)); 
    targets[1] = (long)((SPOOL_WIDTH_MM / FILAMENT_DIA_MM) * SPO_ROT_STEPS);
    steppers.moveTo(targets);
    Serial.println((String)"Linear steps: " + targets[0] + "  &  Rotational steps:" + targets[1]);    

    linStepper.setSpeed(STEPS_SEC_SPEED / SPO_LIN_RATIO);
    Serial.println((String)"Linear speed: " + linStepper.speed() + " (" + ((float)(targets[0]) / linStepper.speed()) + " seconds)");

    spoStepper.setSpeed(STEPS_SEC_SPEED);
    Serial.println((String)"Spool speed: " + spoStepper.speed() + " (" + ((float)(targets[1]) / spoStepper.speed()) + " seconds)");

    // move until both steppers have reached their individual goals
    while (steppers.run())  {
      if (LIN_STOP_SWITCH_ACTIVE) {
        Serial.println("ERROR: Linear slide hit a stop.  Stopped due to this failure !");
        break;
      }
    }
    // first layer moves were completed
    Serial.println("Finished first layer of filament !");
  }

  // wait 10 seconds, then do it all over again
  Serial.println("Waiting 15 seconds before doing it all over again...");
  delay (15000);
}
