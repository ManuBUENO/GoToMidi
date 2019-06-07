#ifndef DEF_MIDICLASS
#define DEF_MIDICLASS

#include "RtMidi.h"
#include "opencv2/core/core.hpp"

#include "configClass.hpp"

#define MIDIPORTNAME 	"MIDI_GO_OUT"
#define NOTE_ZERO	 	37 //36 //C1
#define NOTE_ATTACK	 	100 

class midiClass
{
    public:
    bool init(configClass *);
    bool updateChannels(std::vector <cv::Vec2i>);
    bool channelsOff();
    //get

    //set

    private:
    //ptr config
    configClass * c_ptrMainConfig;

    RtMidiOut *c_midiOut;
};

#endif









