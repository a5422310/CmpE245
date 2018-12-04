/*
Usage: ./codesend decimalcode [protocol] [pulselength]
decimalcode - As decoded by RFSniffer
protocol    - According to rc-switch definitions
pulselength - pulselength in microseconds

 'codesend' hacked from 'send' by @justy
 
 - The provided rc_switch 'send' command uses the form systemCode, unitCode, command
   which is not suitable for our purposes.  Instead, we call 
   send(code, length); // where length is always 24 and code is simply the code
   we find using the RF_sniffer.ino Arduino sketch.

(Use RF_Sniffer.ino to check that RF signals are being produced by the RPi's transmitter 
or your remote control)
*/
#include "RCSwitch.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h> // Added by Jason
     
int main(int argc, char *argv[]) {
    
    // This pin is not the first pin on the RPi GPIO header!
    // Consult https://projects.drogon.net/raspberry-pi/wiringpi/pins/
    // for more information.
    int PIN = 0;
    
    // Parse the first parameter to this command as an integer
    int protocol = 0; // A value of 0 will use rc-switch's default value
    int pulseLength = 0;
    // Added by Jason
    int corruption = 0;

    // If no command line argument is given, print the help text
    if (argc == 1) {
	// Omitted by Jason
        // printf("Usage: %s decimalcode [protocol] [pulselength]\n", argv[0]);
        // printf("decimalcode\t- As decoded by RFSniffer\n");
        // printf("protocol\t- According to rc-switch definitions\n");
        // printf("pulselength\t- pulselength in microseconds\n");

	// Added by Jason
        printf("Usage: %s [pulselength] [corruption]\n", argv[0]);
        printf("pulselength\t- Pulselength in microseconds\n");
        printf("corruption\t- The percentage of corruption\n");
        return -1;
    }

    // Change protocol and pulse length accroding to parameters
    // Omitted by Jason
    // int code = atoi(argv[1]);
    // if (argc >= 3) protocol = atoi(argv[2]);
    // if (argc >= 4) pulseLength = atoi(argv[3]);

    // Added by Jason
    pulseLength = atoi(argv[1]);
    if (argc >= 3) corruption = atoi(argv[2]);
    
    if (wiringPiSetup () == -1) return 1;
    // Omitted by Jason
    // printf("sending code[%i]\n", code);
    RCSwitch mySwitch = RCSwitch();
    if (protocol != 0) mySwitch.setProtocol(protocol);
    if (pulseLength != 0) mySwitch.setPulseLength(pulseLength);
    mySwitch.enableTransmit(PIN);
    // Omitted by Jason
    // mySwitch.send(code, 24);

    // Added by Jason
    srand(time(NULL));
    unsigned char sync_field_buffer[32] = {0};
    char prefixes[2] = {0xA0, 0x50};
    int corrupt_num = (32 * corruption) / 100;
    int corrupted_record [corrupt_num];
    int rand_byte = 0;
    int index;

    // Create Sync field
    for(int i = 0; i < 2; i++)
    {
	for(int j = 0; j < 16; j++)
	{
	    index = i * 16 + j;
	    sync_field_buffer[index] = prefixes[i] + j;
	}
    }

    for(int k = 0; k < corrupt_num; k++)	corrupted_record [k] = {-1};
    for(int i = 0; i < corrupt_num; i++)
    {
    	run:
	rand_byte = rand() % (32);
    	for(int j = 0; j < corrupt_num; j++)
    	{
	    if(corrupted_record[j] == rand_byte)	goto run;
    	}
	corrupted_record[i] = rand_byte;
	sync_field_buffer[rand_byte] = rand() % (32 * 8);
    }

    for(int i = 0; i < 32; i++)
    {
    	mySwitch.send(sync_field_buffer[i], 8);
	printf("Transmit Sync Byte: (0x%02X) = %d\n", sync_field_buffer[i], sync_field_buffer[i]);
    }




    // Send Payload w/offset
    uint c = 0;
    uint payloadByte = 0;
    char payloadString[MAX_PAYLOAD_BYTES] = "SJSU_CMPE245_JASON_8103";
    while (payloadString[c] != '\0')
    {
    	payloadByte = (unsigned)payloadString[c];
		mySwitch.send(payloadByte, 8); // 8 bits (1 byte) length
		printf("Transmit Payload Byte: (%c) = %d\n", payloadString[c], payloadByte);
		c++;
    }

    // mySwitch.setPulseLength(500);
    mySwitch.setPulseLength(pulseLength);
    // Send remaining bytes to fill up 128 byte buffer on reciever end
    for(uint i = (unsigned)strlen(payloadString); i < MAX_PAYLOAD_BYTES+10; i++)
    {
    	mySwitch.send(4, 8); // 4 EOT (end of transmission)
    	printf("Transmit Payload Byte: (EOT) = 4\n");
    }
    
    return 0;
}
