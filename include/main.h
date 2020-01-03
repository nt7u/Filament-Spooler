// main.h

#ifndef main_h
#define main_h

// ------   User editable definitions   ------ //
#define STEPS_SEC_SPEED 200.0f          // Stepper speed (spool)
#define SPOOL_WIDTH_MM  55.0f           // Spool inside width
#define FILAMENT_DIA_MM 1.75f           // Filament diameter
#define HOMING_SPEED    128.0f
// ------------------------------------------- // 

// Motor shield and driver boards I/O for enable (active low)
#define MOTORS_ENABLED      0
#define MOTORS_DISABLED     1

#define LIN_STOP_SWITCH_ACTIVE  !digitalRead(linStopSw)
#define LIN_STOP_SWITCH_INACTIVE  digitalRead(linStopSw)

// Linear slide stepper motor, leadscrew particulars, and stop switch definitions
#define LIN_DEG_STEP        18.0f   // 18 degrees per step
#define LIN_STOKE_MM        80.0f   // 80 mm stroke (approx)
#define LIN_SCREW_PITCH     0.5f    // 0.5 mm spaced threads
#define LIN_MAX_MM_SEC      25.0f   // 25 mm per second

#define LIN_STEPS_ROT       (360.0f/LIN_DEG_STEP)                               // steps per rotation
#define LIN_STEPS_PER_MM    (LIN_STEPS_ROT/LIN_SCREW_PITCH)                     // steps per mm movement
#define LIN_MAX_STEP_SPEED  ((LIN_STEPS_ROT/LIN_SCREW_PITCH)*LIN_MAX_MM_SEC)    // max. steps per sec
#define LIN_STROKE_STEPS    ((LIN_STOKE_MM/LIN_SCREW_PITCH)*LIN_STEPS_ROT)      // steps to slew full stroke
#define HOME_DISTANCE       (LIN_STEPS_PER_MM * 5)                              // how many mm from end-stop to call "home"

// Spool stepper motor and belt reduction definitions
#define SPO_DEG_STEP        1.8f    // 1.8 degrees per step
#define SPO_BELT_RATIO      3.0f    // motor turns per spool rotations

#define SPO_ROT_STEPS       ((360.0f/SPO_DEG_STEP)*SPO_BELT_RATIO)      // motor steps per spool rotation

// Ratio of spool steps per linear steps needed for setting speed of linear stepper --
// We let the user adjust and the spool stepper speed and the corresponding linear gets calculated using this
#define SPO_LIN_RATIO       ( SPO_ROT_STEPS/(FILAMENT_DIA_MM * LIN_STEPS_PER_MM) )

#endif
