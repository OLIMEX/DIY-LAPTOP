/* teres1-debug.c - debug mode switch

   Copyright (C) 2018 Chris Boudacoff

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
#include <unistd.h>
#include <string.h>
#include <linux/input.h>
#include <unistd.h>
#include <sys/types.h>

#define DEBUGEN		361


void export_gpio(int gpio)
{
  int fd;
  char buf[255];

  fd = open("/sys/class/gpio/export", O_WRONLY);
  sprintf(buf, "%d", gpio);
  write(fd, buf, strlen(buf));
  close(fd);

  sprintf(buf, "/sys/class/gpio/gpio%d/direction", gpio);
  fd = open(buf, O_WRONLY);
  write(fd, "out", 3);
  close(fd);

}

void set_gpio(int gpio, int value)
{
  char buf[255];
  sprintf(buf, "/sys/class/gpio/gpio%d/value", gpio);
  int fd = open(buf, O_WRONLY);
  sprintf(buf, "%d", value);
  write(fd, buf, 1);
  close(fd);

}

void usage(void)
{
  extern char *program_invocation_short_name;
  printf("USAGE:\n");
  printf(" sudo %s on|off\n", program_invocation_short_name);
  printf("\n");
}

int main (int argc, char **argv)
{
int mode;

uid_t uid=getuid(), euid=geteuid();


if (uid!=0 || uid!=euid || !argv[1]) {
	 usage();
    exit(0);
}else{
	
	
  if (strcmp(argv[1],"off")==0) mode = 1;
  else if (strcmp(argv[1],"on")==0) mode = 0;
  else { usage(); exit(0); }
  export_gpio(DEBUGEN);
  set_gpio(DEBUGEN, mode);
  printf("Debuging via serial cable on headphone port is %s\r\n",argv[1]);
  return 0;
}
}
