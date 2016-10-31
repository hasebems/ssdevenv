//
//  raspi_magicflute.h
//  ToneGenerator
//
//  Created by 長谷部 雅彦 on 2014/02/22.
//  Copyright (c) 2014年 長谷部 雅彦. All rights reserved.
//

#ifndef __ToneGenerator__raspi_magicflute__
#define __ToneGenerator__raspi_magicflute__

#include	"msgf_if.h"


//-------------------------------------------------------------------------
//		Macros
//-------------------------------------------------------------------------
#define			FIRST_INPUT_GPIO	9
#define			MAX_SW_NUM			3
#define			MAX_LED_NUM			1
#define			MIDI_CENTER			64
#define			AUTO_DISPLAY_DISABLE	(-1)

//-------------------------------------------------------------------------
//		Class define
//-------------------------------------------------------------------------
class Raspi {

public:
	Raspi():
		partTranspose(MIDI_CENTER),
		formerTime(0),
		timeSumming(0),
		timerCount(0),
		voiceNum(0),
		autoDisplayChangeCount(AUTO_DISPLAY_DISABLE),
		tgptr(0) {}

	void eventLoop( void );
	void init( msgf::Msgf* tg );
	void quit( void );

	msgf::Msgf* GetMsgfTg( void ){ return tgptr; }

private:
	void sendMessageToMsgf( unsigned char msg0, unsigned char msg1, unsigned char msg2 );
	void displayTranspose( void );
	void changeTranspose( unsigned char tp );
	void ledOn( int num );
	void ledOff( int num );
	void transposeEvent( int num );
	void changeVoiceEvent( int num );

	void initGPIO( void );
	void analyseGPIO( void );

	static void (Raspi::*swJumpTable[])( int num );

	//-------------------------------------------------------------------------

	unsigned char partTranspose;
	int	swOld[MAX_SW_NUM];
	int	gpioOutputVal[MAX_LED_NUM];

	long 	formerTime;
	long 	timeSumming;
	int		timerCount;
	int		voiceNum;
	int		autoDisplayChangeCount;

	unsigned char soundOn;
	msgf::Msgf* tgptr;
};

#endif /* defined(__ToneGenerator__raspi_magicflute__) */
