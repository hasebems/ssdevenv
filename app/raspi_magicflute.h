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

class Raspi {

public:
	void eventLoop( msgf::Msgf* tg );
	void init( void );
	void quit( void );

private:
	void initGPIO( void );
	void analyseGPIO( void );

	long formerTime;
	long timeSumming;
	int	timerCount;

	unsigned char soundOn = 0;
	msgf::Msgf* tgptr = 0;
};

#endif /* defined(__ToneGenerator__raspi_magicflute__) */
