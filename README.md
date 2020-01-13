# Spooler
Arduino firmware to operate two stepper motors, in unison, for re-spooling 3d printer filament.  For my brother, Jake.

January 12, 2020

You won't find an .ino file exis as instead there is the ‘main.cpp’ -- within the source directory (‘/src’).  It can simply be renamed to something useful and given the .ino suffix (e.g. “main.cpp” -> “spooler_vX.ino”).  That will make it usable in the Arduino IDE.  It is important to be sure that the corresponding 'main.h' file get copied into the folder where the .ino file ends up, as well.   Changes to 'main.h' will be made alongside any changes in the ‘main.cpp’ so be sure to use both corresponding version together.  ‘main.h’ should never be renamed as it is referenced in the .cpp file as ‘main.h’ and not something else.

The AccelStepper and MultiStepper library files, both headers and source code (.h & .cpp), should be included in the same folder as your .ino file.  So that folder will have everything required to make the project work.
In reality the Accel/Mulit-Stepper library could reside in your Arduino development directory’s library but to ensure that the known working version of that library is used, it is included here.
