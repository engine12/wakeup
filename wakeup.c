/*      Copyright (C) 2012, Engine12, LLC
 *
 *      Released under the GNU General Public License, version 2,
 *      included herein by reference.
 */
 
#include <stdio.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>


int main(int argc, char **argv)
{
	int i, fd, retval;
	unsigned long data, secs, mins, hrs;
	struct rtc_time rtc_tm;
	const char *rtc = "/dev/rtc0";
	unsigned int deltaT = 0;  //time in seconds
	
	switch (argc) {
		case 2:
			deltaT = atoi(argv[1]);
			break;

		default:
			fprintf(stderr, "usage:  wakeup [seconds]\n");
			return 1;
	}

	if((fd = open(rtc, O_RDONLY)) == -1){
		perror(rtc);
		exit(errno);
	}

	// get the current time
	if (ioctl(fd, RTC_RD_TIME, &rtc_tm) == -1) {
		perror("RTC_RD_TIME ioctl");
		exit(errno);
	}
	
	fprintf(stderr, "\n\nCurrent RTC time is %02d:%02d:%02d.\n",
							rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);
		
	secs = deltaT + rtc_tm.tm_sec;	
	rtc_tm.tm_sec = secs%60;
	
	mins = secs/60 + rtc_tm.tm_min;
	rtc_tm.tm_min = (mins)%60;
	
	rtc_tm.tm_hour = (mins/60 + rtc_tm.tm_hour)%24;

	// Set the alarm -- 24 hr period
	if (ioctl(fd, RTC_ALM_SET, &rtc_tm) == -1) {
		perror("RTC_ALM_SET ioctl");
		exit(errno);
	}

	// Read the current alarm settings
	if (ioctl(fd, RTC_ALM_READ, &rtc_tm) == -1) {
		perror("RTC_ALM_READ ioctl");
		exit(errno);
	}
	
	fprintf(stderr, "RTC Alarm set to %02d:%02d:%02d.\n",
							rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);
		
	// Enable alarm interrupts
	if (ioctl(fd, RTC_AIE_ON, 0) == -1) {
		perror("RTC_AIE_ON ioctl");
		exit(errno);
	}

	// This blocks until the alarm ring causes an interrupt 
	if (read(fd, &data, sizeof(unsigned long)) == -1) {
		perror("read");
		exit(errno);
	}
	
	fprintf(stderr, "RTC Alarm rang.\n");

	// Disable alarm interrupts
	if (ioctl(fd, RTC_AIE_OFF, 0) == -1) {
		perror("RTC_AIE_OFF ioctl");
		exit(errno);
	}

	close(fd);
	return 0;
}