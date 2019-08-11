# Compliation is OS-dependant
# Get OS
UNAME := $(shell uname)

ifeq ($(UNAME),Linux)
	# Set Linux options
	# Please change paths depending on where your libraries are.
	INCLUDE= `pkg-config rtmidi opencv --cflags`
	MACRO= 
	FRAMEWORK=
	LIBSPATH=
	LIBS=`pkg-config jack rtmidi opencv --libs`

else ifeq ($(UNAME),Darwin)
	# Set OSX options
	# Please change paths depending on where your libraries are.
	INCLUDE=-I/usr/local/Cellar/opencv@2/2.4.13.7_3/include/
	MACRO=-D__MACOSX_CORE__ -D__RTMIDI_DEBUG__
	FRAMEWORK=-framework coreMIDI -framework CoreAudio -framework CoreFoundation
	LIBSPATH=-L/usr/local/Cellar/opencv@2/2.4.13.7_3/lib
	LIBS=-lopencv_calib3d -lopencv_contrib -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_gpu -lopencv_highgui -lopencv_imgproc -lopencv_legacy -lopencv_ml -lopencv_nonfree -lopencv_objdetect -lopencv_ocl -lopencv_photo -lopencv_stitching -lopencv_superres -lopencv_ts -lopencv_video -lopencv_videostab
endif

OPTION=-Wall

# Compile binary
gb : main.cpp guiClass.o configClass.o processClass.o midiClass.o typesClass.o
	g++ -std=c++11 ${OPTION} ${MACRO} ${INCLUDE} -o goToMidi main.cpp RtMidi.cpp guiClass.o configClass.o processClass.o midiClass.o typesClass.o ${LIBSPATH} ${LIBS} ${FRAMEWORK}

guiClass.o : guiClass.hpp guiClass.cpp
	g++ -std=c++11 ${OPTION} ${MACRO} ${INCLUDE} -c guiClass.cpp

configClass.o : configClass.hpp configClass.cpp
	g++ -std=c++11 ${OPTION} ${MACRO} ${INCLUDE} -c configClass.cpp

processClass.o : processClass.hpp processClass.cpp
	g++ -std=c++11 ${OPTION} ${MACRO} ${INCLUDE} -c processClass.cpp

midiClass.o : midiClass.hpp midiClass.cpp
	g++ -std=c++11 ${OPTION} ${MACRO} ${INCLUDE} -c midiClass.cpp

typesClass.o : typesClass.hpp typesClass.cpp
	g++ -std=c++11 ${OPTION} ${MACRO} ${INCLUDE} -c typesClass.cpp

clean :
	rm *.o goToMidi
