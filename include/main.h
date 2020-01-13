// main.h
#ifndef main_h
#define main_h

// ------   User editable definitions   ------ //
#define SPOL_STEPS_SEC      200.0f          // Stepper speed (spool)
#define SPO_MAX_STEP_SPEED  500.0f          // Max reasonable (?)
#define SPOOL_WIDTH_MM       55.0f          // Spool inside width
#define FILAMENT_DIA_MM      1.75f          // Filament diameter
#define HOMING_SPEED        140.0f          // quick & convenient?  6x normal spooling
// ------------------------------------------- //

#define SPOOL_ROTATIONS ((SPOOL_WIDTH_MM / FILAMENT_DIA_MM) - 1)

// Linear slide stepper motor, leadscrew particulars, and stop switch definitions
#define LIN_DEG_STEP        18.0f   // 18 degrees per quarter-step
#define LIN_STOKE_MM        80.0f   // 80 mm stroke (approx)
#define LIN_SCREW_PITCH     0.5f    // 0.5 mm spaced threads
#define LIN_MAX_MM_SEC      25.0f   // 25 mm per second

#define LIN_STEPS_ROT       (360.0f / LIN_DEG_STEP)                             // steps per rotation (360/18 = 20)
#define LIN_STEPS_PER_MM    (LIN_STEPS_ROT / LIN_SCREW_PITCH)                   // steps per mm movement (20/0.5 = 40)
#define LIN_STEPS_FILAMENT  (LIN_STEPS_PER_MM * FILAMENT_DIA_MM)                // steps per filament width (40*1.75 = 70)
#define LIN_MAX_STEP_SPEED  (LIN_STEPS_PER_MM * LIN_MAX_MM_SEC)                 // max. steps per sec (40*25 = 1000)
#define LIN_STROKE_STEPS    ((LIN_STOKE_MM / LIN_SCREW_PITCH) * LIN_STEPS_ROT)  // steps to slew full stroke ((80/0.5)*20 = 3200)
#define HOME_DISTANCE       (LIN_STEPS_PER_MM * 5)                              // how many mm from end-stop to call "home" (40*5 = 200)

// Spool stepper motor and belt reduction definitions
#define SPO_DEG_STEP        1.8f    // 1.8 degrees per step
#define SPO_BELT_RATIO      3.0f    // motor turns per spool rotations

#define SPO_ROT_STEPS       ((360.0f/SPO_DEG_STEP)*SPO_BELT_RATIO)      // motor steps per spool rotation (360/1.8*3 = 600)

// Ratio of spool steps per linear steps needed for setting speed of linear stepper --
// We let the user adjust and the spool stepper speed and the corresponding linear gets calculated using this
#define SPO_LIN_RATIO       ( (double)SPO_ROT_STEPS/(double)LIN_STEPS_FILAMENT ) // (600/70 = 8.5714)

// Hardware pin definitions 
#define spoStepPin  2       // X driver on CNC shield
#define spoDirPin   5       //  "
#define linStepPin  3       // Y driver on CNC shield
#define linDirPin   6       //  "
#define enMotorPin  8       // Active LOW enable of all motor drivers
#define linStopSw  10       // Y end-stop on CNC shield
#define resetAbort A0
#define feedHold   A1
#define startResum A2       // Run/Stop switch (open = Paused; Stopped)

// Motor shield and driver boards I/O for enable (active low)
#define MOTORS_ENABLED      0
#define MOTORS_DISABLED     1

// Switch reading convenience macros
#define LIN_STOP_SWITCH_ACTIVE  !digitalRead(linStopSw)
#define LIN_STOP_SWITCH_INACTIVE  digitalRead(linStopSw)
#define RUN_SWITCH_ACTIVE  !digitalRead(startResum)
#define RUN_SWITCH_INACTIVE  digitalRead(startResum)

#endif
