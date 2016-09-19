
PROGRAMS = ssdevenv
RM = /bin/rm

MSGF_SRC_PATH = ../Musical_Sound_Generator_Framework/audio_engine:../Musical_Sound_Generator_Framework/msgf_general:../Musical_Sound_Generator_Framework/magic_flute
STK_SRC_PATH = ../stk
APP_SRC_PATH = ../app
vpath %.cpp $(MSGF_SRC_PATH):$(STK_SRC_PATH):$(APP_SRC_PATH)

OBJECT_PATH = Release
vpath %.o $(OBJECT_PATH)

INCLUDE_PATH = ../stk_include:$(MSGF_SRC_PATH):$(APP_SRC_PATH)
vpath %.h $(INCLUDE_PATH)

OBJECTS =       Stk.o Noise.o Envelope.o \
                Modulate.o SineWave.o FileRead.o FileWrite.o \
                FileWvIn.o FileLoop.o FileWvOut.o \
                OnePole.o Fir.o \
                Delay.o DelayL.o DelayA.o \
                JCRev.o \
                Voicer.o Sphere.o \
                Messager.o Skini.o utilities.o \
                RtMidi.o RtAudio.o Thread.o Mutex.o Socket.o TcpServer.o \
				mf_Instrument.o mf_voice_context.o mf_note.o msgf_configuration.o \
				msgf_osc_pipe.o msgf_amp_pipe.o \
				msgf_if.o msgf_audio_buffer.o msgf_eg.o msgf_eg2seg.o msgf_eg4seg.o \
				msgf_event_info.o msgf_instrument_for_check.o msgf_instrument.o \
				msgf_level_check.o msgf_lfo.o msgf_note.o msgf_part.o msgf_voice_context.o \
				ssdevenv_main.o

CC       = g++
DEFS     =   -DHAVE_GETTIMEOFDAY  -D__LINUX_ALSA__
DEFS    += -D__LITTLE_ENDIAN__
CFLAGS   = -O3 -Wall
CFLAGS  += -I$(INCLUDE_PATH)
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


(END)
