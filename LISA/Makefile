CC=arm-linux-gnueabihf-gcc
CFLAGS=

CXX=arm-linux-gnueabihf-g++
CXXFLAGS=-DRPI

LDFLAGS=

RM=rm -f

all: clean codesend RFSniffer

codesend: RCSwitch.o codesend.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -lwiringPi -lwiringPiDev -lcrypt
	
RFSniffer: RCSwitch.o RFSniffer.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -lwiringPi -lwiringPiDev -lcrypt

clean:
	$(RM) *.o send codesend RFSniffer RFModule Transmitter
