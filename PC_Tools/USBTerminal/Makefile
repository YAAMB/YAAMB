CC = g++

all: clibusbwrapper.o cterminal.o cfl_streamoutput.o USBTerminal.cxx USBTerminal

cfl_streamoutput.o: cfl_streamoutput.h cfl_streamoutput.cxx
	$(CC) -c cfl_streamoutput.cxx -Wall -Wextra `fltk-config --cxxflags`

cterminal.o: cterminal.h cterminal.cxx
	$(CC) -c cterminal.cxx  -Wall -Wextra `fltk-config --cxxflags`

clibusbwrapper.o: clibusbwrapper.h clibusbwrapper.cpp
	$(CC) -c clibusbwrapper.cpp  -Wall -Wextra 

USBTerminal.cxx: USBTerminal.f 
	fluid -c USBTerminal.f

USBTerminal: USBTerminal.cxx cterminal.o clibusbwrapper.o cfl_streamoutput.o
	$(CC)  `fltk-config --cxxflags` USBTerminal.cxx  cterminal.o clibusbwrapper.o cfl_streamoutput.o -o USBTerminal `fltk-config --ldflags` -lusb -lfltk_images -lpng
	
clean:
	-rm -f *.o *.~ USBTerminal.cxx USBTerminal.h
	
