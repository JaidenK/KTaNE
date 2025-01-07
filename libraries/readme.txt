This folder contains the embedded code for all of the modules. To build the code for a specific module, copy only the relevant folders into your Arduino libraries folder and compile the ES_FrameworkMain sketch.

The following libraries are used by all modules:
- ES_Framework
- KTaNE

Some modules need 3rd party libraries that aren't included here.
The Timer also requires:
- TM1637

You cannot compile with multiple module libraries present at once. This will be apparent due to the conflicts with ES_Configure.h, Board.h, and other repeated files.
