#ifndef DEF_MIDICLASS
#define DEF_MIDICLASS

#include "RtMidi.h"
#include "opencv2/core/core.hpp"

#include "configClass.hpp"

#define MIDIPORTNAME 	"MIDI_GO_OUT"

class midiClass
{
    public:

    midiClass(configClass *);
    bool init();
    bool updateChannels();
    
    //get

    //set

    private:
    //ptr config
    configClass * c_ptrMainConfig;
    RtMidiOut *c_midiOut;
    RtMidiIn *c_midiIn;

    //List of channels
    std::vector <Channel *> c_channels;
};

#endif









