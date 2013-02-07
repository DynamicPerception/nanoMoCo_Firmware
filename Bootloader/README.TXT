nanoMoCo uses a slightly modified form of optiboot to allow it to be programmed over the RS485
bus using the standard IDE.

To install bootloader:

 - Need ISP programmer

( using Arduino built-in commands )

1) Copy contents of bootloader (hardware/) into Arduino directory/bundle
2) cd all of the way down into optiboot directory
3) Modify makefile to point to correct port for your ISP programmer
4) omake nanoMoCo_isp

( using avrdude, make, etc. )

1) cd all the way down into optiboot directory
2) make nanoMoCo_isp


