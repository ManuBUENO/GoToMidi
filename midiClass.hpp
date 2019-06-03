#ifndef DEF_MIDICLASS
#define DEF_MIDICLASS

#include "RtMidi.h"
#include "opencv2/core/core.hpp"

#define MIDIPORTNAME 	"MIDI_GO_OUT"
#define NOTE_ZERO	 	36 //36 //C1
#define NOTE_ATTACK	 	100 

class midiClass
{
    public:
    bool init();
    bool updateChannels(std::vector <cv::Vec2i>);
    bool channelsOff();
    //get

    //set

    private:
    RtMidiOut *c_midiOut;
};

#endif









