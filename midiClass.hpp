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
    bool updateNotes(std::vector <cv::Vec3i>);
    bool notesOff();
    //get

    //set

    private:
    RtMidiOut *c_midiOut;
};

#endif









