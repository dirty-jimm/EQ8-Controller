# EQ8-Controller
 Command Line Program for interfacing directly with EQ8 Motor Drivers

Capability checklist
    - Send serial transmission to mount            [√]
    - Recieve serial response from mount           [√]

    - Set speed (slow) via interrupt parameter     []
    - String to long function                      []
    - Turn (degrees) function.                     []



Updates
    March 4th
    {
    Notes from David:
    Ben and David are developing the (micro movement) PID.
    Will either output a PID angular velocity value, possibly x,y coords instead, will require testing.
    Made some minor fixes, deployed to redpitaya - code compiles, however serial comms is broken, seems to be writing garbled strings, might be
    memory/pointer reference issues. 
    }