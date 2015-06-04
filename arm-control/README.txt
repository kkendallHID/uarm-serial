The HID serial code for controlling the uArm robotic arm.

The code is designed to work with a generic serial terminal.

A few sections of the code can be either included or omitted from
compilation by commenting out or leaving in the lines that look like this
example from Line 80:

#define SUCTION_CODE // Comment out this line to enable the pinch end-effector and disable the suction cup.

If the user wants to use the pincher end-effector instead of the suction cup
these two lines should be changed to:

//#define SUCTION_CODE // Comment out this line to enable the pinch end-effector and disable the suction cup.

There is a second section used for debugging that enables the help portion
of the code. Uncomment the following line to enable the feature but be warned that
the increase in memory required may cause system instability.

// #define DEBUG_CODE  // Uncomment this line to enable debugging. Currently creates system instability. Memory problems.
