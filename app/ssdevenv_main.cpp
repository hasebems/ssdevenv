// ssdevenv_main.cpp
//
// An example STK program that allows voice playback and control of
// most of the STK instruments.

#include "SKINImsg.h"
#include "WvOut.h"
//#include "Instrmnt.h"
#include "JCRev.h"
//#include "Voicer.h"
#include "Skini.h"
#include "RtAudio.h"
#include "FileWvOut.h"
#include "Messager.h"

//	for MSGF
#ifdef _MSGF_MF_
	#include "msgf_if.h"				//	added by M.H
	#include "msgf_audio_buffer.h"		//	added by M.H
#endif

#if defined(__STK_REALTIME__)
  #include "Mutex.h"
#endif

#if defined(__RASP_APP__)
	#include	"raspi_magicflute.h"
	#include	"raspi_hw.h"
#endif

#include <cstring>
#include <signal.h>
#include <iostream>
#include <algorithm>
#include <cmath>
//-------------------------------------------------------------------------------
using std::min;

bool done;
static void finish(int ignore){ done = true; }

using namespace stk;

//-------------------------------------------------------------------------------
// The TickData structure holds all the class instances and data that
// are shared by the various processing functions.
struct TickData {
	WvOut **wvout;
//	Instrmnt **instrument;
#ifdef _MSGF_MF_
	msgf::Msgf *msgf;		//	added by M.H
#endif
//	Voicer *voicer;
	JCRev reverb;
	Messager messager;
	Skini::Message message;
	StkFloat volume;
	StkFloat t60;
	unsigned int nWvOuts;
	int nVoices;
	int currentVoice;
	int channels;
	int counter;
	bool realtime;
	bool settling;
	bool haveMessage;
	int frequency;

	// Default constructor.
	TickData()
	: wvout(0), //instrument(0), voicer(0),
	volume(1.0), t60(0.75),
	nWvOuts(0), nVoices(1), currentVoice(0), channels(2), counter(0),
	realtime( false ), settling( false ), haveMessage( false ) {}
};
//-------------------------------------------------------------------------------
#define DELTA_CONTROL_TICKS 64 // default sample frames between control input checks


//-------------------------------------------------------------------------------
void usage(char *function) {

	printf("\nuseage: %s flag(s) \n", function);
	printf("        where flag = -s RATE to specify a sample rate,\n");
	printf("        -n NUMBER specifies the number of voices to allocate,\n");
	printf("        -ow <file name> for .wav audio output file,\n");
	printf("        -os <file name> for .snd audio output file,\n");
	printf("        -om <file name> for .mat audio output file,\n");
	printf("        -oa <file name> for .aif audio output file,\n");
#if defined(__STK_REALTIME__)
	printf("        -or for realtime audio output,\n");
#endif
	printf("        -if <file name> to read control input from SKINI file,\n");
#if defined(__STK_REALTIME__)
	printf("        -ip for realtime control input by pipe,\n");
	printf("        -im <port> for realtime control input by MIDI (virtual port = 0, default = 1),\n");
#endif

	exit(0);
}
//-------------------------------------------------------------------------------
int checkArgs(int nArgs, char *args[])
{
	int w, i = 1, j = 0;
	int nWvOuts = 0;
	char flags[2][50] = {""};
	bool realtime = false;

	if (nArgs < 3 || nArgs > 22) usage(args[0]);

	while (i < nArgs) {
		if (args[i][0] == '-') {
			if (args[i][1] == 'o') {
				if ( args[i][2] == 'r' ) realtime = true;
				if ( (args[i][2] == 's') || (args[i][2] == 'w') ||
					(args[i][2] == 'm') || (args[i][2] == 'a') )
					nWvOuts++;
				flags[0][j] = 'o';
				flags[1][j++] = args[i][2];
			}
			else if (args[i][1] == 'i') {
				if ( (args[i][2] != 'p') && (args[i][2] != 'm') &&
					(args[i][2] != 'f') ) usage(args[0]);
				flags[0][j] = 'i';
				flags[1][j++] = args[i][2];
			}
			else if (args[i][1] == 's' && (i+1 < nArgs) && args[i+1][0] != '-' ) {
				Stk::setSampleRate( atoi(args[i+1]) );
				flags[0][j++] = 's';
			}
			else if (args[i][1] == 'n' && (i+1 < nArgs) && args[i+1][0] != '-' ) {
				flags[0][j++] = 'n';
			}
			else usage(args[0]);
		}
		i++;
	}

	// Check for multiple flags of the same type
	for ( i=0; i<=j; i++ ) {
		w = i+1;
		while  (w <= j ) {
			if ( flags[0][i] == flags[0][w] && flags[1][i] == flags[1][w] ) {
				printf("\nError: Multiple command line flags of the same type specified.\n\n");
				usage(args[0]);
			}
			w++;
		}
	}

	// Make sure we have at least one output type
	if ( nWvOuts < 1 && !realtime ) usage(args[0]);

	return nWvOuts;
}
//-------------------------------------------------------------------------------
bool parseArgs(int nArgs, char *args[], WvOut **output, Messager& messager)
{
	int i = 1, j = 0, nWvIns = 0;
	bool realtime = false;
	char fileName[256];

	while (i < nArgs) {
		if ( (args[i][0] == '-') && (args[i][1] == 'i') ) {
			switch(args[i][2]) {

				case 'f':
					strcpy(fileName,args[++i]);
					if ( !messager.setScoreFile( fileName ) ) exit(0);
					nWvIns++;
					break;

				case 'p':
#if defined(__STK_REALTIME__)
					if ( !messager.startStdInput() ) exit(0);
					nWvIns++;
					break;
#else
					usage(args[0]);
#endif

				case 'm':
#if defined(__STK_REALTIME__)
					// Check for an optional MIDI port argument.
					if ((i+1 < nArgs) && args[i+1][0] != '-') {
						int port = atoi(args[++i]);
						if ( !messager.startMidiInput( port-1 ) ) exit(0);
					}
					else if ( !messager.startMidiInput() ) exit(0);
					nWvIns++;
					break;
#else
					usage(args[0]);
#endif

				default:
					usage(args[0]);
					break;
			}
		}
		else if ( (args[i][0] == '-') && (args[i][1] == 'o') ) {
			switch(args[i][2]) {

				case 'r':
#if defined(__STK_REALTIME__)
					realtime = true;
					break;
#else
					usage(args[0]);
#endif

				case 'w':
					if ((i+1 < nArgs) && args[i+1][0] != '-') {
						i++;
						strcpy(fileName,args[i]);
					}
					else strcpy(fileName,"testwav");
					output[j] = new FileWvOut(fileName, 1, FileWrite::FILE_WAV );
					j++;
					break;

				case 's':
					if ((i+1 < nArgs) && args[i+1][0] != '-') {
						i++;
						strcpy(fileName,args[i]);
					}
					else strcpy(fileName,"testsnd");
					output[j] = new FileWvOut(fileName,1, FileWrite::FILE_SND);
					j++;
					break;

				case 'm':
					if ((i+1 < nArgs) && args[i+1][0] != '-') {
						i++;
						strcpy(fileName,args[i]);
					}
					else strcpy(fileName,"testmat");
					output[j] = new FileWvOut(fileName,1, FileWrite::FILE_MAT);
					j++;
					break;

				case 'a':
					if ((i+1 < nArgs) && args[i+1][0] != '-') {
						i++;
						strcpy(fileName,args[i]);
					}
					else strcpy(fileName,"testaif");
					output[j] = new FileWvOut(fileName,1, FileWrite::FILE_AIF );
					j++;
					break;

				default:
					usage(args[0]);
					break;
			}
		}
		i++;
	}

	if ( nWvIns == 0 ) {
#if defined(__STK_REALTIME__)
		if ( !messager.startStdInput() ) exit(0);
#else
		printf("\nError: The -if file input flag must be specified for non-realtime use.\n\n");
		usage(args[0]);
#endif
	}

	return realtime;
}
//-------------------------------------------------------------------------------
//				Message for MSGF
//-------------------------------------------------------------------------------
void processMessage_msgf( TickData* data )
{
	unsigned char value1 = static_cast<unsigned char>(data->message.floatValues[0]);
	unsigned char value2 = static_cast<unsigned char>(data->message.floatValues[1]);

#ifdef _MSGF_MF_
	unsigned char	msg[3];
	msgf::Msgf*	tg = static_cast<msgf::Msgf*>(data->msgf);
	if ( tg == NULL ) return;
#endif

	// If only one instrument, allow messages from all channels to control it.
	//int group = 1;
	//  if ( data->nVoices > 1 ) group = data->message.channel;

	switch( data->message.type ) {

#ifdef _MSGF_MF_
		case __SK_Exit_:
			if ( data->settling == false ) goto settle;
			done = true;
			return;

		case __SK_NoteOn_:
			if ( value2 > 0.0 ) { // velocity > 0
				msg[0] = 0x90; msg[1] = value1; msg[2] = value2;
				tg->sendMessage( 3, msg );
				//data->voicer->noteOn( value1, value2 );
				break;
			}
			// else a note off, so continue to next case

		case __SK_NoteOff_:
			//data->voicer->noteOff( value1, value2 );
			msg[0] = 0x90; msg[1] = value1; msg[2] = 0;
			tg->sendMessage( 3, msg );
			break;

		case __SK_ControlChange_:
			msg[0] = 0xb0; msg[1] = value1; msg[2] = value2;
			tg->sendMessage( 3, msg );
			break;

//		case __SK_AfterTouch_:
//			data->voicer->controlChange( 128, value1 );
//			break;

//		case __SK_PitchChange_:
//			data->voicer->setFrequency( value1 );
//			break;

//		case __SK_PitchBend_:
//			data->voicer->pitchBend( value1 );
//			break;

		case __SK_Volume_:
			msg[2] = static_cast<unsigned char>(value1 * ONE_OVER_128);
			msg[0] = 0xb0; msg[1] = 0x07;
			tg->sendMessage( 3, msg );
			break;

		case __SK_ProgramChange_:
			msg[0] = 0xc0; msg[1] = value1; msg[2] = 0;
			tg->sendMessage( 2, msg );
			break;
#endif
		default: break;

	} // end of switch

	data->haveMessage = false;
	return;

settle:
	// Exit and program change messages are preceeded with a short settling period.
	data->counter = (int) (0.3 * data->t60 * Stk::sampleRate());
	data->settling = true;
}
//-------------------------------------------------------------------------------
//				tick for MSGF	added by M.H
//-------------------------------------------------------------------------------
int tick_msgf(	void *outputBuffer, void *inputBuffer,
		 		unsigned int nBufferFrames, double streamTime,
		 		RtAudioStreamStatus status, void* dataPointer )
{
	TickData* data = (TickData *)dataPointer;
	StkFloat sample;
	StkFloat* samples = (StkFloat *)outputBuffer;
	int counter;
	int nTicks = (int) nBufferFrames;

#ifdef _MSGF_MF_
	//	make a MSGF Audio Buffer instance
	msgf::TgAudioBuffer	abuf;
	msgf::Msgf*	tg = static_cast<msgf::Msgf*>(data->msgf);
	if ( tg == NULL ){ return -1; }
#endif

	while ( nTicks > 0 && !done ) {

		//	to decide sample counts before next message shoude be executed
		if ( !data->haveMessage ) {
			data->messager.popMessage( data->message );
			if ( data->message.type > 0 ) {
				data->counter = static_cast<int> (data->message.time * Stk::sampleRate());
				data->haveMessage = true;
			}
			else {
				data->counter = DELTA_CONTROL_TICKS;
			}
		}
		counter = min( nTicks, data->counter );
		//	the rest count in this loop before next message
		data->counter -= counter;

		if ( counter > 0 ){
			//	MSGF generate Audio
#ifdef _MSGF_MF_
			abuf.obtainAudioBuffer(counter);			//	MSGF IF
			tg->process( abuf );						//	MSGF IF

			for ( int i=0; i<counter; i++ ) {
				sample = data->volume *
						 data->reverb.tick( abuf.getAudioBuffer(static_cast<int>(i)) );

				for ( unsigned int j=0; j<data->nWvOuts; j++ ){
					data->wvout[j]->tick(sample);
				}

				if ( data->realtime ){
					for ( int k=0; k<data->channels; k++ ){
						*samples++ = sample;
					}
				}
				nTicks--;
			}
			//	Release AudioBuffer
			abuf.releaseAudioBuffer();
#endif
			if ( nTicks == 0 ) break;
		}

		// Process control messages.
		if ( data->haveMessage ) processMessage_msgf( data );
	}

#if defined(__RASP_APP__)
	RASP_eventLoop();
#endif

	return 0;
}

//-------------------------------------------------------------------------------
//				initialize
//-------------------------------------------------------------------------------
int init( int argc, char *argv[], TickData& data )
{
	// If you want to change the default sample rate (set in Stk.h), do
	// it before instantiating any objects!  If the sample rate is
	// specified in the command line, it will override this setting.
	Stk::setSampleRate( 44100.0 );

	// Depending on how you compile STK, you may need to explicitly set
	// the path to the rawwave directory.
	Stk::setRawwavePath( "../rawwaves/" );

	// By default, warning messages are not printed.  If we want to see
	// them, we need to specify that here.
	Stk::showWarnings( true );

	// Check the command-line arguments for errors and to determine
	// the number of WvOut objects to be instantiated (in utilities.cpp).
	data.nWvOuts = checkArgs( argc, argv );
	data.wvout = (WvOut **) calloc( data.nWvOuts, sizeof(WvOut *) );

	//	Create MSGF
#ifdef _MSGF_MF_
	data.msgf = new msgf::Msgf;						//	added by M.H
	unsigned char msg[3] = { 0xc0, 0, 0 };			//	added by M.H
	msg[1] = 0;										//	added by M.H
	data.msgf->sendMessage( 2, msg );				//	added by M.H
#endif

#if defined(__RASP_APP__)
	RASP_init();
#endif

	return 0;
}

//-------------------------------------------------------------------------------
//				main
//-------------------------------------------------------------------------------
int main( int argc, char *argv[] )
{
	TickData data;

#if defined(__STK_REALTIME__)
	RtAudio dac;
#endif

	if ( init( argc, argv, data ) == -1 ){
		goto cleanup;
	}

	// Parse the command-line flags, instantiate WvOut objects, and
	// instantiate the input message controller (in utilities.cpp).
	try {
		data.realtime = parseArgs( argc, argv, data.wvout, data.messager );
	}
	catch (StkError &) {
		goto cleanup;
		//return -1;
	}

	// If realtime output, allocate the dac here.
#if defined(__STK_REALTIME__)
	if ( data.realtime ) {
		RtAudioFormat format = ( sizeof(StkFloat) == 8 ) ? RTAUDIO_FLOAT64 : RTAUDIO_FLOAT32;
		RtAudio::StreamParameters parameters;
		parameters.deviceId = dac.getDefaultOutputDevice();
		parameters.nChannels = data.channels;
		unsigned int bufferFrames = RT_BUFFER_SIZE;
		try {
			dac.openStream( &parameters, NULL, format,
							(unsigned int)Stk::sampleRate(), &bufferFrames,
							&tick_msgf, (void *)&data );
		}
		catch ( RtAudioError& error ) {
			error.printMessage();
			goto cleanup;
			//return -1;
		}
	}
#endif

	// Set the reverb parameters.
	data.reverb.setT60( data.t60 );
	data.reverb.setEffectMix(0.2);

	// Install an interrupt handler function.
	(void) signal(SIGINT, finish);

	// If realtime output, set our callback function and start the dac.
#if defined(__STK_REALTIME__)
	if ( data.realtime ) {
		try {
			dac.startStream();
		}
		catch ( RtAudioError &error ) {
			error.printMessage();
			goto cleanup;
		}
	}
#endif

	//------------------
	// Setup finished.
	//	Main Loop
	//------------------
	while ( !done ) {
#if defined(__STK_REALTIME__)
	  if ( data.realtime )
		  // Periodically check "done" status.
		  Stk::sleep( 200 );
//	  else
#endif
		// Call the "tick" function to process data.
		//tick( NULL, NULL, 256, 0, 0, (void*)&data );
	}


	// Shut down the output stream.
#if defined(__STK_REALTIME__)
	if ( data.realtime ) {
		try {
			dac.closeStream();
		}
		catch ( RtAudioError& error ) {
			error.printMessage();
		}
	}
#endif

cleanup:
#if defined(__RASP_APP__)
	RASP_quit();
#endif

#ifdef _MSGF_MF_
	if ( data.msgf ){			//	added by M.H
		delete data.msgf;		//	added by M.H
	}
#endif

	for ( int i=0; i<(int)data.nWvOuts; i++ ) delete data.wvout[i];
	free( data.wvout );

//	delete data.voicer;

//	for ( int i=0; i<data.nVoices; i++ ) delete data.instrument[i];
//	free( data.instrument );

	std::cout << "\nStk demo finished ... goodbye.\n\n";
	return 0;
}
