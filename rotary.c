#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <wiringPi.h>

#define  RoAPin    0  // GPIO-Pin 17, Pinnummer 11
#define  RoBPin    1  // GPIO-Pin 18, Pinnummer 12

unsigned char flag;
unsigned char Last_RoB_Status;
unsigned char Current_RoB_Status;

#define  RoAPinHdg    2  
#define  RoBPinHdg    3  

unsigned char flagHdg;
unsigned char Last_RoB_StatusHdg;
unsigned char Current_RoB_StatusHdg;

#define ToFromPin 4

void rotaryDialFreq(void)
{
	Last_RoB_Status = digitalRead(RoBPin);

	while(!digitalRead(RoAPin)){
		Current_RoB_Status = digitalRead(RoBPin);
		flag = 1;
	}

	if(flag == 1){
		flag = 0;
		if((Last_RoB_Status == 0)&&(Current_RoB_Status == 1)){
			system("echo \"frq+\" | nc 127.0.0.1 1235 -w 0");
		}
		if((Last_RoB_Status == 1)&&(Current_RoB_Status == 0)){
			system("echo \"frq-\" | nc 127.0.0.1 1235 -w 0");
		}

	}
}

void rotaryDialHdg(void)
{
	Last_RoB_StatusHdg = digitalRead(RoBPinHdg);

	while(!digitalRead(RoAPinHdg)){
		Current_RoB_StatusHdg = digitalRead(RoBPinHdg);
		flagHdg = 1;
	}

	if(flagHdg == 1){
		flagHdg = 0;
		if((Last_RoB_StatusHdg == 0)&&(Current_RoB_StatusHdg == 1)){
			system("echo \"hdg+\" | nc 127.0.0.1 1235 -w 0");
		}
		if((Last_RoB_StatusHdg == 1)&&(Current_RoB_StatusHdg == 0)){
			system("echo \"hdg-\" | nc 127.0.0.1 1235 -w 0");
		}

	}
}


int main(void)
{
	if(wiringPiSetup() < 0){
		fprintf(stderr, "Unable to setup wiringPi:%s\n",strerror(errno));
		return 1;
	}

	pinMode(RoAPin, INPUT);
	pinMode(RoBPin, INPUT);

	pinMode(RoAPinHdg, INPUT);
	pinMode(RoBPinHdg, INPUT);

	pinMode(ToFromPin, INPUT);


	while(1){
		rotaryDialFreq();
		rotaryDialHdg();
        if (digitalRead(ToFromPin) == 0){
           system("echo \"tofr\" | nc -u 127.0.0.1 1235 -w 0");
        }
        delay(10);
	}

	return 0;
}

