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
	#define Buffer_Size 128
	#define Sync_Field_Size 32		//32 Bytes
	
    srand(time(NULL));
	int sync_field_index, payload_index;
    unsigned char sync_field_buffer[Sync_Field_Size] = {0};
	unsigned char output_buffer[Buffer_Size];
	for(int k = 0; k < Buffer_Size; k++) output_buffer[k] = rand() % (256)+1;
    const char * payload = "SJSU_CMPE245_JASON_8103";
	
    char prefixes[2] = {0xA0, 0x50};
    int corrupt_num = (Sync_Field_Size * corruption) / 100;
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

	// Corrupt Sync field
    for(int k = 0; k < corrupt_num; k++)	corrupted_record [k] = {-1};
    for(int i = 0; i < corrupt_num; i++)
    {
    	run:
		rand_byte = rand() % (Sync_Field_Size);
    	for(int j = 0; j < corrupt_num; j++)
    	{
			if(corrupted_record[j] == rand_byte)	goto run;
    	}
		corrupted_record[i] = rand_byte;
		sync_field_buffer[rand_byte] = rand() % (Sync_Field_Size * 8);
    }
	
    int payload_length = strlen(payload);
    sync_field_index = rand() % (Buffer_Size - (Sync_Field_Size + payload_length));
    payload_index = Sync_Field_Size + sync_field_index;
    for(int i = sync_field_index; i < sync_field_index + Sync_Field_Size; i++)	//insert sync field + payload at random offset
    {
    	output_buffer[i] = sync_field_buffer[i - sync_field_index];
    	output_buffer[i + Sync_Field_Size] = payload [i - sync_field_index];
    }
	

    // Send output
    for(int i = 0; i < Buffer_Size; i++)
    {
    	mySwitch.send(output_buffer[i], 8);
		printf("Transmit Byte: (0x%02X) = %d\n", output_buffer[i], output_buffer[i]);
    }

    for(int i = 0; i < Buffer_Size / 4; i++)
    {
    	mySwitch.send(4, 8); // 4 EOT( End of Transmission)
    	printf("Transmit Byte: (EOT) = 4\n");
    }
    
    return 0;
}
