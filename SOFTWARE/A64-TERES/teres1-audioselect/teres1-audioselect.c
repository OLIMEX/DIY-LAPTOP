/* teres1-audiocontrol.c - audio input/output manager

   Should be run as root. On start, it reads an A64 specific
   memory address and sets the mixer accordingly. Then the program
   exits, unless a device is given as an argument
   (normally, /dev/input/by-path/platform-sound.6-event).
   In this case, the program runs forever, watching
   the jack plug in/out events.

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
#include <signal.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>

#define SW_AC_IO_BASE 0x01c22c00
#define SUNXI_HMIC_STS 0x01c22c00+0x319

unsigned long SUNXI_PIO_BASE = 0;
void select_mode(char mmsel,char mval){
    switch (mmsel) {
        case 2: // headphones
            printf("Headphones: %d\n", mval);
            if (mval == 0)
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
        case 4: // mic
            printf("Microphone: %d\n", mval);
            if (mval == 0)
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

}


// check specific memory address for the jack status, set mixer accordingly
// return -1 on error, 1 otherwise
int mem_set_mixer() {
    off_t offset = SUNXI_HMIC_STS;
    size_t len = 0x01;
    size_t pagesize = sysconf(_SC_PAGE_SIZE);
    off_t page_base = (offset / pagesize) * pagesize;
    off_t page_offset = offset - page_base;

    int fd = open("/dev/mem", O_SYNC);
    if (fd == -1) {
        perror("Cannot open /dev/mem");
        return -1;
    }
    unsigned char *mem = mmap(NULL, page_offset + len, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, page_base);
    if (mem == MAP_FAILED) {
        perror("Can't map memory");
        return -1;
    }

    if ((int)mem[page_offset] && 0x08)  //headset?
        select_mode(4,1);
    else
        select_mode(4,0);
    if ((int)mem[page_offset] && 0x04)  //headphone?
        select_mode(2,1);
    else
        select_mode(2,0);               

    close(fd);
    return 1;
}


int main (int argc, char **argv)
{
    const char *device = NULL;
    int fd;
    struct input_event ie;

    if (mem_set_mixer() == -1) {
        exit(255);
    }

    if (!argv[1]) {
        exit(0);
    }

    device = argv[1];

    for (;;)
    {
        if((fd = open(device, O_RDONLY)) == -1)
        {
            perror("Could not open input device");
            exit(255);
        }

        while(read(fd, &ie, sizeof(struct input_event))>0)
        {
            //  printf("type %d\tcode %d\tvalue %d\n", ie.type, ie.code, ie.value);
            if (ie.type == 5) {
                select_mode(ie.code,ie.value);
            }
        }

        close(fd);
        usleep(65535u);
    }
    return 0;
}
