//
//  raspi_magicflute.cpp
//  ToneGenerator
//
//  Created by 長谷部 雅彦 on 2014/02/22.
//  Copyright (c) 2014年 長谷部 雅彦. All rights reserved.
//


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdbool.h>

#include 	"raspi_magicflute.h"
#include	"raspi_hw.h"

#include	"msgf_audio_buffer.h"

//-------------------------------------------------------------------------
//		Send Message
//-------------------------------------------------------------------------
void Raspi::sendMessageToMsgf( unsigned char msg0, unsigned char msg1, unsigned char msg2 )
{
	unsigned char msg[3];
	msg[0] = msg0; msg[1] = msg1; msg[2] = msg2;
	//	Call MSGF
	rp->GetMsgfTg()->sendMessage( 3, msg );
}
//-------------------------------------------------------------------------
//		Settings
//-------------------------------------------------------------------------
void Raspi::changeTranspose( unsigned char tp )
{
	if ( tp == partTranspose ) return;

	if ( tp > MIDI_CENTER+6 ) partTranspose = MIDI_CENTER-6;
	else if ( tp < MIDI_CENTER-6 ) partTranspose = MIDI_CENTER+6;
	else partTranspose = tp;

	sendMessageToMsgf( 0xb0, 0x0c, partTranspose );
	printf("Note Shift value: %d\n",partTranspose);

	int nsx = partTranspose - MIDI_CENTER;
	if ( nsx < 0 ) nsx += 12; //	0 <= nsx <= 11
	else if ( nsx > 12 ) nsx -= 12;

	const int tCnv[12] = {3,12,4,13,5,6,15,7,9,1,10,2};
	writeMark(tCnv[nsx]);
}
//-------------------------------------------------------------------------
//		GPIO Input
//-------------------------------------------------------------------------
void Raspi::ledOn( int num )
{
	write( gpioOutputVal[num], "1", 2 );
}
//-------------------------------------------------------------------------
void Raspi::ledOff( int num )
{
	write( gpioOutputVal[num], "0", 2 );
}
//-------------------------------------------------------------------------
void Raspi::transposeEvent( int num )
{
	int inc = 1;
	if ( num == 1 ) inc = -1;
	unsigned char tpTemp = partTranspose + inc;
	changeTranspose(tpTemp);
}
//-------------------------------------------------------------------------
void Raspi::changeVoiceEvent( int num )
{
	printf("Change Voice!\n");
}
//-------------------------------------------------------------------------
//static void (*const tFunc[MAX_SW_NUM])( Raspi* rp, int num ) =
//{
//	transposeEvent,
//	transposeEvent,
//	changeVoiceEvent
//};
//-------------------------------------------------------------------------
void Raspi::analyseGPIO( void )
{
	unsigned char note, vel;
	int 	i;
	char	gpioPath[64];
	int		fd_in[MAX_SW_NUM], swNew[MAX_SW_NUM];

	//	open
	for (i=0; i<MAX_SW_NUM; i++){
		sprintf(gpioPath,"/sys/class/gpio/gpio%d/value",i+FIRST_INPUT_GPIO);
		fd_in[i] = open(gpioPath,O_RDWR);
		if ( fd_in[i] < 0 ) exit(EXIT_FAILURE);
	}

	//	read
	for (i=0; i<MAX_SW_NUM; i++){
		char value[2];
		read(fd_in[i], value, 2);
		if ( value[0] == '0' ) swNew[i] = 0;
		else swNew[i] = 1;
	}

	//	close
	for (i=0; i<MAX_SW_NUM; i++){
		close(fd_in[i]);
	}

	//	Switch Event Job
	if ( swNew[0] != swOld[0] ){
		if ( !swNew[0] ){ transposeEvent(num); }
		swOld[0] = swNew[0];
	}
	if ( swNew[1] != swOld[1] ){
		if ( !swNew[1] ){ transposeEvent(num); }
		swOld[1] = swNew[1];
	}
	if ( swNew[2] != swOld[2] ){
		if ( !swNew[0] ){ transposeEvent(num); }
		swOld[2] = swNew[2];
	}

//	for (i=0; i<MAX_SW_NUM; i++ ){
//		if ( swNew[i] != swOld[i] ){
//			if ( !swNew[i] ){
//				//	When push
//				(*tFunc[i])(this,i);
//			}
//			swOld[i] = swNew[i];
//		}
//	}
}
//-------------------------------------------------------------------------
void Raspi::initGPIO( void )
{
	int	fd_exp, fd_dir, i;
	char gpiodrv[64];

	fd_exp = open("/sys/class/gpio/export", O_WRONLY );
	if ( fd_exp < 0 ){
		printf("Can't open GPIO\n");
		exit(EXIT_FAILURE);
	}
	write(fd_exp,"7",2);
	write(fd_exp,"9",2);
	write(fd_exp,"10",2);
	write(fd_exp,"11",2);
	close(fd_exp);

	//	input
	for ( i=FIRST_INPUT_GPIO; i<FIRST_INPUT_GPIO+MAX_SW_NUM; i++ ){
		sprintf(gpiodrv,"/sys/class/gpio/gpio%d/direction",i);
		fd_dir = open(gpiodrv,O_RDWR);
		if ( fd_dir < 0 ){
			printf("Can't set direction(Input)\n");
			exit(EXIT_FAILURE);
		}
		write(fd_dir,"in",3);
		close(fd_dir);
	}

	//	output
	for ( i=7; i<8; i++ ){
		sprintf(gpiodrv,"/sys/class/gpio/gpio%d/direction",i);
		fd_dir = open(gpiodrv,O_RDWR);
		if ( fd_dir < 0 ){
			printf("Can't set direction(Output)\n");
			exit(EXIT_FAILURE);
		}
		write(fd_dir,"out",4);
		close(fd_dir);
	}

	for ( i=7; i<8; i++ ){
		sprintf(gpiodrv,"/sys/class/gpio/gpio%d/value",i);
		gpioOutputVal[0] = open(gpiodrv,O_RDWR);
		if ( gpioOutputVal[0] < 0 ){
			printf("Can't set value\n");
			exit(EXIT_FAILURE);
		}
	}
}

//-------------------------------------------------------------------------
//		event Loop
//-------------------------------------------------------------------------
#define		AVERAGE_TIMER_CNT		100		//	This times
//-------------------------------------------------------------------------
void Raspi::eventLoop( void )
{
	struct	timeval tstr;
	long	crntTime, diff;

	//	Time Measurement
	gettimeofday(&tstr, NULL);
	crntTime = tstr.tv_sec * 1000 + tstr.tv_usec/1000;

	//	Main Task
	analyseGPIO();

	//	Analyse Processing Time
	diff = crntTime - formerTime;
	timeSumming += diff;
	formerTime = crntTime;
	timerCount++;

	//	Measure Main Loop Time & Make heartbeats
	if ( timerCount >= AVERAGE_TIMER_CNT ){
		printf("---Loop Interval value(100times): %d [msec]\n",timeSumming);
		timeSumming = 0;
		timerCount = 0;
		ledOn(0);
	}
	if ( timerCount > (AVERAGE_TIMER_CNT/10) ) ledOff(0);
}
//-------------------------------------------------------------------------
//			Initialize
//-------------------------------------------------------------------------
void Raspi::init( msgf::Msgf* tg )
{
	struct	timeval tstr;
	long	crntTime;

	//	Set MSGF Pointer
	tgptr = tg;

	//--------------------------------------------------------
	//	Initialize GPIO
	initGPIO();

	//--------------------------------------------------------
	//	Initialize I2C device
	initI2c();
	//initLPS331AP();
	//	initSX1509();	//	GPIO Expander
	//initMPR121();
	//if ( useFullColorLed == true ) initBlinkM();
	initAda88();
	//initADS1015();	//	AD Converter
	//if ( useAccelSensor == true ) initADXL345();

	//--------------------------------------------------------
	//	initialize Display
	writeMark(3);		// "C"
	ledOff(0);


	//--------------------------------------------------------
//	sendMessageToMsgf( 0xb0, 0x0b, 0 );
	soundOn = 0;
	timerCount = 0;
	timeSumming = 0;
	tgptr = 0;
	for ( int i=0; i<MAX_SW_NUM; i++ ) swOld[i] = 1;

	changeTranspose( tg, 0 );

	//	Time Measurement
	gettimeofday(&tstr, NULL);
	formerTime = tstr.tv_sec * 1000 + tstr.tv_usec/1000;
}
//-------------------------------------------------------------------------
//			Quit
//-------------------------------------------------------------------------
void Raspi::quit( void )
{
	quitI2c();
}
