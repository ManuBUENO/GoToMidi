Prerequisites:

	- For OS X:
		 	Xcode command line tools
		 	gcc (usually included in Xcode command line tools)
		 	homebrew (check some tuto)

	- OpenCV 2.4.11
			Make sure to install at least OpenCV 2.4.9
			Do not install OpenCV 3.0.0 nor higher

	- A midi API, depending on operating system
			For linux, install Jack (apt-get install jackd) and qjackctl (apt-get install qjackctl)
			For OS X, MIDICore is used. Should be already available

	- RtMidi >=4.0.0
			Get last version from "http://www.music.mcgill.ca/~gary/rtmidi/", and install manually


Configuration

	- For Linux, Jack and qjackctl needs to be configurated (check some tutos)

Compliation:

	Run make
	If compliation fails, check that library and include paths are correct in the makefile.

Run:
	
	./go_board_reader

Troubleshooting:

	"HIGHGUI ERROR: V4L: index 1 is not correct!"
	--> Webcam flux unknown. by default, flux "/dev/video1" is opened.
	--> You can change the /dev/video index in "configClass.hpp" #define CAMERA_INDEX 1 

	MidiOutJack::initialize: JACK server not running?
	--> Midi API jack not running.
	--> Open qjackctl and start it (make sure it is configured)

	Stones positions not matching the grid
	--> The go-board dimensions needs to be updates
	--> Open "configClass.hpp" and update edges and grid dimensions in cm

	Stones are not recognized (blue=no stone ; green= black stone ; red = white stone)
	--> Contrast, brightness, and detection thresholds needs to be updated
	--> open "config.txt", change parameters and click "Update parameters"