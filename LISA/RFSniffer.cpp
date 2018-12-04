/*
  RFSniffer

  Usage: ./RFSniffer [<pulseLength>]
  [] = optional

  Hacked from http://code.google.com/p/rc-switch/
  by @justy to provide a handy RF code sniffer
*/

#include "RCSwitch.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

RCSwitch mySwitch;

 
int main(int argc, char *argv[])
{
    // This pin is not the first pin on the RPi GPIO header!
    // Consult https://projects.drogon.net/raspberry-pi/wiringpi/pins/
    // for more information.
    int PIN = 2;

    if(wiringPiSetup() == -1)
    {
        printf("wiringPiSetup failed, exiting...");
        return 1;
    }

    int pulseLength = 0;
	
	// Added by Jason
	int confidence = 0;
	if (argc == 1)
	{
		printf("Usage: %s [pulselength] [corruption]\n", argv[0]);
        printf("pulselength\t- Pulselength in microseconds\n");
        printf("confidence\t- The percentage of confidence level\n");
        return -1;
	}
	
	// Added & Modified by Jason
    pulseLength = atoi(argv[1]);
    if (argc >= 3) confidence = atoi(argv[2]);
	
    mySwitch = RCSwitch();
    mySwitch.setPulseLength(pulseLength); // Modified by Jason
    mySwitch.enableReceive(PIN);  // Receiver on interrupt 0 => that is pin #2

	
	// Added by Jason
	#define Buffer_Size 128
	#define Sync_Field_Size 32		//32 Bytes
	int temp_match_percent[Buffer_Size - Sync_Field_Size] = {0};
	unsigned char input_buffer[Buffer_Size];
	char prefixes[2] = {0xA0, 0x50};
	unsigned char sync_field_buffer[Sync_Field_Size] = {0};
	int input_buffer_count, match_num, match_index = 0;
	int index;

    while(1)
    {
        if (mySwitch.available())
        {
			// Modified by Jason
            unsigned char value = (unsigned char)mySwitch.getReceivedValue();

			// Omitted by Jason
            // if (value == 0) {
            //     printf("Unknown encoding\n");
            // } else {    
            //     // printf("Received %i\n", mySwitch.getReceivedValue() );
            //     printf("Received %i\n", value);
            // }

			
			// Added by Jason
            if (input_buffer_count < Buffer_Size)
            {
                if (value != 4)
                {
                    printf("Received Byte: (%c) = %u\n", value, value);
                }
                input_buffer[input_buffer_count] = value;
                input_buffer_count++;
            }
            else
            {
				input_buffer_count = 0;

				    // Create Sync field for matching
					for(int i = 0; i < 2; i++)
					{
						for(int j = 0; j < 16; j++)
						{
							index = i * 16 + j;
							sync_field_buffer[index] = prefixes[i] + j;
						}
					}
					
					for(int i = 0; i < Buffer_Size - Sync_Field_Size; i++)
					{
						for(int j = 0; j < Sync_Field_Size; j++)
						{
							if(input_buffer[i + j] == sync_field_buffer[j]) match_num++;
						}
						temp_match_percent[i] = match_num;
						if(temp_match_percent[match_index] < match_num)	match_index = i;
						match_num = 0;
					}
					
					int input_index = match_index + Sync_Field_Size;
					double match_percent = ( (double)temp_match_percent[match_index] / Sync_Field_Size ) * 100;

					if(match_percent >= confidence)
					{
						printf("Success: Payload found with confidence level %d%% at index %d\n", confidence, input_index);
						printf("Payload: ");
						for(int i=match_index + Sync_Field_Size; i< match_index + Sync_Field_Size + 23; i++)	printf("%c", input_buffer[i]);
						printf("\n");
					}
					else	printf("Failed: Cannot find payload with confidence level %d%%\n", confidence);				
            }

            fflush(stdout);
            mySwitch.resetAvailable();
        }

        usleep(500);
    }

    exit(0);
}
