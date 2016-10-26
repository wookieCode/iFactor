*****************************************************
* iFactor Readme - written by Jeff Henderson 4/2014 *
*****************************************************

***********************
* System Requirements *
***********************

	-i386 compatible machine.  Preferably x86-64.
	-GNU MP library
	-POSIX conformant OS.  Tested in Ubuntu.
	-g++ greater than version 4.6.  This is needed for c++11 support.  Tested with g++ 4.6.3.

iFactor is written in C/C++11 and was tested and complied on gcc/g++ version 4.3.  Because of iFactors use of C++11, a gcc/g++ version of at least 4.6 is needed.   iFactor also uses the GNU MP library for arbritrary precision integers.  This allows us to bypass the number limitations on built in types.  iFactor tries to make use of multi core systems.  It has only been tested on POSIX systems with the -pthread compiler option.

****************************
* Compilation instructions *
****************************

	$ make
	$ ./iFactor

iFactor comes with a Makefile to better help manage compiler options.  Once system requirements are met, all one needs to do is type make.  This will produce a binary called iFactor.

*********
* Usage *
*********

There are 2 basic usage forms:

	1) ./iFactor numberToFactor

This takes numberToFactor and attempts to factor it.  It will display the number you entered and, once found, 2 factors.  iFactor will also display the approximate time it took to factor in milliseconds.

	2) ./iFactor -k keyFile

In this form, iFactor attempts to break RSA given the Public Key.  The public key is expected to be found in keyFile.  keyFile should only contain 2 lines.  The first line should conatain N.  The second line should contain e.  Upon suscessful factorization of N, iFactor will display the same output as form 1, as well as N,e,d.  It will also create a file called privateKey.txt.  This will have the same form as keyFile only d, instead of e, will be placed on the second line.

Options:
	-t numberOfThreads
This option can be placed at the end of either of the basic usage forms.  It allows the user to control how many threads iFactor uses.  The default value is 1.  It is best to match the number of threads used with the number of concurrent processes your machine can use.  For examlpe, on a pentium you should use -t 1.  On an Intel core 2 duo you should use -t 2.  On a quad core i7 with hyperthreading, you should use -t 8.


*********
* Notes *
*********

This program was written and tested on an Ubuntu virtual machine using virtual box on a Macbook Air with a 1.6ghz Intel i5 processor with 4gb of ram.  The option -t 2 was used for all testing.
