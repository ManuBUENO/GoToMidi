#ifndef DEF_MIDICLASS
#define DEF_MIDICLASS

#include "RtMidi.h"
#include "opencv2/core/core.hpp"
#include <thread>
//#include <mutex>


#include "configClass.hpp"

#define MIDIPORTOUT 	"MIDI_GO_OUT"
#define MIDIPORTIN      "MIDI_GO_IN"

#define MIDI_NOTEON     144
#define MIDI_NOTEOFF    128

#define SEQ_STATE_DEAD  -2
#define SEQ_STATE_READY -1

class midiClass
{
    public:

    midiClass(configClass *);
    bool init();
    void start();
    void stop();
    bool updateChannels();
   
    //get
    int getSeqState();
    //set
    void setSeqState(int);

    private:

    void manageSequencer(std::vector< unsigned char > *msg );
    void midiThread();
    static void midiCallback(double, std::vector< unsigned char > *, void *);

    // ptr config
    configClass * c_ptrMainConfig;

    // Midi ports
    RtMidiOut *c_midiOut;
    RtMidiIn *c_midiIn;

    // Midi Thread
    std::thread c_thread;
    unsigned int c_threadStatus;

    // List of channels
    std::vector <Channel *> c_channels;

    // Sequencer state and mutex
    int c_seqState;
    std::mutex c_stateMut;
};

#endif









