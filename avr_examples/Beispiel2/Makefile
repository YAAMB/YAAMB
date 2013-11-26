# Hey Emacs, this is a -*- makefile -*-

# AVR-GCC Makefile template, derived from the WinAVR template (which
# is public domain), believed to be neutral to any flavor of "make"
# (GNU make, BSD make, SysV make)
# new splitted Makefile (26.11.2013)

MCU = atmega32
FORMAT = ihex
TARGET = main
SRC_DIR = ../../common/avr/src/
SRC = $(TARGET).c $(SRC_DIR)lcd.c $(SRC_DIR)cmdline.c $(SRC_DIR)ps2.c
ASRC = 
OPT = 2

# Place -I options here
CINCS = -I../../common/avr/include

include ../../common/Makefile.master
