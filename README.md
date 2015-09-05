## regurgitator: Find library symbols in registers after making function calls

### What
Register Gurgitator is a utility that calls various stdlib routines and looks at
the registers after making the function call.  Address values in a register
(non-clobbered of course) can provide a vector into stdlib even given a
randomized address space.  More tests can be added, but this version only has a
few tests from stdlib, tests such as "printf" or "malloc".  The results of the
caller-saved registers are displayed after calling each test.

For the motivation see:
https://github.com/enferex/randhack

### Building
Run *make* from the source directory.  The resulting binary can be copied
anywhere.

### Contact
mattdavis9@gmail.com (enferex)
