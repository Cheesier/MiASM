MiASM
=====
MiASM is a very simple, poorly written "assembler" for the LiU 'lmia' systems target to teach microprogramming.
Its assembly language is similar to the Motorola 68000 assembly language, and with minor modifications it can probably be ported over.

Features
=====
* Any compatible instructions defined in the computer spec file ('.csf')
* Labels
* DAT <value> (takes up 1 word of memory, and loads that with <value>)
* ORG <value> (the following instructions will start at location <value>)
