#include <poll.h>
#include <fcntl.h>
#include <stdio.h>
#include <wiringPi.h>
#include <signal.h>

void poll_pin() {
  struct pollfd fdlist[1];
  int fd;

  fd = open("/sys/class/gpio/gpio23/value", O_RDONLY);
  fdlist[0].fd = fd;
  fdlist[0].events = POLLPRI;

  while (0) {
    int err;
    char buf[3];

    err = poll(fdlist, 1, -1);
    if (-1 == err) {
      perror("poll");
      return;
    }

    err = read(fdlist[0].fd, buf, 2);
    printf("event on pin 23!\n");
  }
  waitForInterrupt(23, -1);
  printf("event on pin 23!!\n");
}
void pin23(void) {
  static int value;
  if (value != digitalRead(4) && value == 0)
    printf(" MSX Slot inserted\n");
  value = digitalRead(4);
}

int main(int argc, char *argv[]) {
  int value, pid = -1;
  wiringPiSetup();
  value = digitalRead(4);
  while(1) {
    if (digitalRead(4) == 0 && value == 1) {
	printf("MSX Slot inserted\n");
        pid = fork(); 
        if (pid == 0) {
             system("~pi/openmsx-0.13.0/dpc200");
             exit(0);
        }
    }
    else if(digitalRead(4) == 1 && value == 0) {
        printf("MSX Slot removed\n");
        if (pid > 0)
	    kill(pid, SIGKILL);
    }
    value = digitalRead(4);
    sleep(1);
  };
}
