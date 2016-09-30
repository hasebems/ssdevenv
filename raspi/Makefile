.SILENT:
PROGRAMS = ssdevenv
RM = /bin/rm

MSGF_AE_SRC_PATH = ../Musical_Sound_Generator_Framework/audio_engine
MSGF_MG_SRC_PATH = ../Musical_Sound_Generator_Framework/msgf_general
MSGF_MF_SRC_PATH = ../Musical_Sound_Generator_Framework/magic_flute
STK_SRC_PATH = ../stk
APP_SRC_PATH = ../app
vpath %.cpp $(MSGF_AE_SRC_PATH):$(MSGF_MG_SRC_PATH):$(MSGF_MF_SRC_PATH):$(STK_SRC_PATH):$(APP_SRC_PATH)

OBJECT_PATH = Release
vpath %.o $(OBJECT_PATH)

INCLUDE_PATH = ../stk_include:$(MSGF_AE_SRC_PATH):$(MSGF_MG_SRC_PATH):$(MSGF_MF_SRC_PATH):$(APP_SRC_PATH)
vpath %.h $(INCLUDE_PATH)

OBJECTS = Stk.o Noise.o Envelope.o \
	Modulate.o SineWave.o FileRead.o FileWrite.o \
	FileWvIn.o FileLoop.o FileWvOut.o \
	OnePole.o Fir.o \
	Delay.o DelayL.o DelayA.o \
	JCRev.o \
	Voicer.o Sphere.o \
	Messager.o Skini.o utilities.o \
	RtMidi.o RtAudio.o Thread.o Mutex.o Socket.o TcpServer.o \
	mf_instrument.o mf_voice_context.o mf_note.o msgf_configuration.o \
	msgf_osc_pipe.o msgf_amp_pipe.o \
	msgf_if.o msgf_audio_buffer.o msgf_eg.o msgf_eg2seg.o msgf_eg4seg.o \
	msgf_event_info.o msgf_instrument_for_check.o msgf_instrument.o \
	msgf_level_check.o msgf_lfo.o msgf_note.o msgf_part.o msgf_voice_context.o \
	ssdevenv_main.o

CC       = g++
DEFS     =   -DHAVE_GETTIMEOFDAY  -D__LINUX_ALSA__　-D__LITTLE_ENDIAN__
DEFS    += -D_MSGF_MF_
CFLAGS   = -O3 -Wall
CFLAGS  += -I../stk_include -I$(MSGF_AE_SRC_PATH) -I$(MSGF_MG_SRC_PATH) -I$(MSGF_MF_SRC_PATH) -I$(APP_SRC_PATH)
LIBRARY = -lpthread -lasound -lm

all : $(PROGRAMS)

ssdevenv: $(OBJECTS)
		$(CC) $(LDFLAGS) $(CFLAGS) $(DEFS) -o ssdevenv $(OBJECT_PATH)/*.o $(LIBRARY)

%.o : %.cpp $(OBJECT_PATH)/.placeholder
		$(CC) $(CFLAGS) $(DEFS) -c $(<) -o $(OBJECT_PATH)/$@

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
