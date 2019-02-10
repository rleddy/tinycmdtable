# tinycmdtable


This is an arduino library for very simple command handling.

There reason it exists is that someone wanted work do be done on the Nano, which has just 2K of memory. We might have used Johnny-five, but we were looking at the possibility of the Nano running untethered with steppers and the like.

Commands need to be able to start and stop processes which might involve a number of different pins, schedules, and more. So, there may be cases where the code in a sketch goes through a number of operations without interacting with a main host. We came up with something as small as possible for managing the intermittent host communication required for the Arduino control.

In an attempt to simplify using the library, we have included a node.js utility which generates much of the sketch from a commanded definition file. The code for this can be found in the *extras* folder. An example of a definition file that the tools in *extras* uses is in example-files/example1_config.json. 

In order to generate code that goes into the sketch, run the following command:

> node genCode.js example-files/example1_config.json

Code files will be produced in the working directory of the script. You will see: sketchFile.txt and mcuConfig.js.

The .txt file contains code that can be cut and pasted in to the Arduino sketch that uses this library: tinyCmdTable. That is, you should have the following at the top of your sketch:

```
#include <tinyCmdTable.h>
```

The node script, *example.j* includes the tables from mcuConfig.js, copied and pasted.  And, it loads in the config and the script files in order to run.

> node example example-files/example1_config.json example-files/example1.json

That is an example of program that does serial communication  with the sketch.

 
