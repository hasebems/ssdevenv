#
#	ssdevenv Makefile
#
.SILENT:
PROGRAMS = ssdev_msgf_mf
RM = /bin/rm

### add user app from here
OTHER_SRC_PATH1 = ../Musical_Sound_Generator_Framework/audio_engine
OTHER_SRC_PATH2 = ../Musical_Sound_Generator_Framework/msgf_general
OTHER_SRC_PATH3 = ../Musical_Sound_Generator_Framework/magic_flute
### to here

STK_SRC_PATH = ../stk
APP_SRC_PATH = ../app

### mix user app
vpath %.cpp $(STK_SRC_PATH):$(APP_SRC_PATH):$(OTHER_SRC_PATH1):$(OTHER_SRC_PATH2):$(OTHER_SRC_PATH3)
###

OBJECT_PATH = Release
vpath %.o $(OBJECT_PATH)

### mix user app
INCLUDE_PATH = ../stk_include:$(APP_SRC_PATH):$(OTHER_SRC_PATH1):$(OTHER_SRC_PATH2):$(OTHER_SRC_PATH3)
vpath %.h $(INCLUDE_PATH)
###

OBJECTS = Stk.o Noise.o Envelope.o \
	Modulate.o SineWave.o FileRead.o FileWrite.o \
	FileWvIn.o FileLoop.o FileWvOut.o \
	OnePole.o Fir.o \
	Delay.o DelayL.o DelayA.o \
	JCRev.o \
	Voicer.o Sphere.o \
	Messager.o Skini.o utilities.o \
	RtMidi.o RtAudio.o Thread.o Mutex.o Socket.o TcpServer.o ssdevenv_main.o

### add user app from here
OTHER_OBJECTS = mf_instrument.o mf_voice_context.o mf_note.o msgf_configuration.o \
	msgf_osc_pipe.o msgf_amp_pipe.o \
	msgf_if.o msgf_audio_buffer.o msgf_eg.o msgf_eg2seg.o msgf_eg4seg.o \
	msgf_event_info.o msgf_instrument_for_check.o msgf_instrument.o \
	msgf_level_check.o msgf_lfo.o msgf_note.o msgf_part.o msgf_voice_context.o
### to here

CC       = g++
DEFS     =   -DHAVE_GETTIMEOFDAY  -D__LINUX_ALSA__　-D__LITTLE_ENDIAN__
CFLAGS   = -O3 -Wall
CFLAGS  += -I../stk_include -I$(APP_SRC_PATH)
LIBRARY = -lpthread -lasound -lm

### add user app from here
DEFS_OTHER   = $(DEFS) -D_MSGF_MF_
CFLAGS_OTHER = $(CFLAGS) -I$(OTHER_SRC_PATH1) -I$(OTHER_SRC_PATH2) -I$(OTHER_SRC_PATH3)
### to here

all : $(PROGRAMS)

ssdevenv: $(OBJECTS)
		$(CC) $(LDFLAGS) $(CFLAGS) $(DEFS) -o $@ $(OBJECT_PATH)/*.o $(LIBRARY)

### add user app from here
ssdev_msgf_mf: $(OBJECTS) $(OTHER_OBJECTS)
		$(CC) $(LDFLAGS) $(CFLAGS_OTHER) $(DEFS_OTHER) -o $@ $(OBJECT_PATH)/*.o $(LIBRARY)
### to here

#%.o : %.cpp $(OBJECT_PATH)/.placeholder
#		echo "--------"
#		echo "compiling" $@
#		$(CC) $(CFLAGS) $(DEFS) -c $(<) -o $(OBJECT_PATH)/$@

### add user app from here
%.o : %.cpp $(OBJECT_PATH)/.placeholder
		echo "--------"
		echo "compiling" $@
		$(CC) $(CFLAGS_OTHER) $(DEFS_OTHER) -c $(<) -o $(OBJECT_PATH)/$@
### to here

$(OBJECT_PATH)/.placeholder:
		mkdir -vp $(OBJECT_PATH)
		touch $(OBJECT_PATH)/.placeholder

$(OBJECTS) : Stk.h

clean :
		$(RM) -f $(OBJECT_PATH)/*.o
		$(RM) -f $(PROGRAMS) *.exe
		$(RM) -fR *~ *.dSYM

distclean: clean
		$(RM) Makefile

strip :
		strip $(PROGRAMS)

# Project specific objects:
