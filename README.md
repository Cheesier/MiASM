MiASM
=====
MiASM is a very simple, poorly written "assembler" for the LiU 'lmia' systems targeted to teach microprogramming.

Its assembly language is similar to the Motorola 68000 assembly language, and with minor modifications it can probably be ported over.

Features
=====
* Any compatible instructions defined in the computer spec file ('.csf')
* Labels
* ``DAT [value]`` (takes up 1 word of memory, and loads that with [value])
* ``ORG [value]`` (the following instructions will be placed at address [value])

Using
=====
Some simple usage instructions are in the 'example.miasm' file, consider this the only documentation.

Compiling is simple, just run ```g++ -o miasm miasm.cpp``` inside the main MiASM directory.

Running it is just a matter of running it like any other program, ```./miasm [flags] filename```.

Support
=====
This assembler will most likely never be updated, NEVER rely on it for production code (duh).
