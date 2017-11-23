/* teres1-audiocontrol.c - audio input/output manager

   Copyright (C) 2017 Chris Boudacoff

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, version 2 of the License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include <linux/input.h>


void usage(void)
{
  extern char *program_invocation_short_name;
  printf("USAGE:\n");
  printf("   %s /dev/input/eventX\n", program_invocation_short_name);
  printf("\n");
}

int main (int argc, char **argv)
{
  const char *device = NULL;
  int fd;
  struct input_event ie;

  if (!argv[1])
  {
    usage();
    exit(0);
  }

  device = argv[1];

  if((fd = open(device, O_RDONLY)) == -1)
  {
    perror("Could not open input device");
    exit(255);
  }



  while(read(fd, &ie, sizeof(struct input_event)))
  {
	  printf("type %d\tcode %d\tvalue %d\n", ie.type, ie.code, ie.value);
    switch (ie.type)
    {
		
    case 5: // pluged
      switch (ie.code) {
		  
      case 2: // headphones
        printf("Headphones: %d\n", ie.value);
        if (ie.value == 0)
        {
			system("amixer -c 0 set \"External Speaker\" on > /dev/null");
			system("amixer -c 0 set \"Headphone\" off > /dev/null");
        }
        else
        {
			system("amixer -c 0 set \"External Speaker\" off > /dev/null");  
			system("amixer -c 0 set \"Headphone\" on > /dev/null");
		}
        break;
        
      case 4: // mice
        printf("Microphone: %d\n", ie.value);
                if (ie.value == 0)
        {
			system("amixer -c 0 sset \"RADC input Mixer MIC1 boost\" unmute > /dev/null" );
			system("amixer -c 0 sset \"LADC input Mixer MIC1 boost\" unmute > /dev/null");
			system("amixer -c 0 sset \"RADC input Mixer MIC2 boost\" mute > /dev/null");
			system("amixer -c 0 sset \"LADC input Mixer MIC2 boost\" mute > /dev/null");
			system("amixer -c 0 sset \"ADCL Mux\" ADC > /dev/null");
			system("amixer -c 0 sset \"ADCR Mux\" ADC > /dev/null");
			system("amixer -c 0 sset \"AIF1 AD0L Mixer ADCL\" ADC > /dev/null");
			system("amixer -c 0 sset \"AIF1 AD0R Mixer ADCR\" ADC > /dev/null");						
        }
        else
        {
			system("amixer -c 0 sset \"RADC input Mixer MIC2 boost\" unmute > /dev/null");
			system("amixer -c 0 sset \"LADC input Mixer MIC2 boost\" unmute > /dev/null");
			system("amixer -c 0 sset \"RADC input Mixer MIC1 boost\" mute > /dev/null");
			system("amixer -c 0 sset \"LADC input Mixer MIC1 boost\" mute > /dev/null");
			system("amixer -c 0 sset \"ADCL Mux\" ADC > /dev/null");
			system("amixer -c 0 sset \"ADCR Mux\" ADC > /dev/null");	
			system("amixer -c 0 sset \"AIF1 AD0L Mixer ADCL\" ADC > /dev/null");
			system("amixer -c 0 sset \"AIF1 AD0R Mixer ADCR\" ADC > /dev/null");								
		}
        break;
      }
      break;
    }
  }

  return 0;
}
