#include "RtMidi.h"
#include <cstdlib>
#include "midiClass.hpp"
#include "configClass.hpp"

using namespace std;
using namespace cv;

midiClass::midiClass(configClass* ptrMainConfig) :
c_ptrMainConfig(ptrMainConfig) {}

bool midiClass::init()
{
	// Get channels
  c_channels = c_ptrMainConfig->getChannels();


  //// Check modes


  //// Init Midi classes

  // RtMidiOut constructor
  try {
    c_midiOut = new RtMidiOut();
  }
  catch ( RtMidiError &error ) {
    error.printMessage();
    return false;}


  // Open virtual ports
  try {
    c_midiOut->openVirtualPort("MIDI_GO_OUT");
  }
  catch ( RtMidiError &error ) {
    error.printMessage();
    return false;
  }


  //// Launch thread if needed



	return true;
}

bool midiClass::updateChannels()
{

	// MODE RANDOM
	// For all changed channels
  unsigned int i;
  vector <unsigned char> message;
  for(i=0;i<c_channels.size();i++)
  {
    if(c_channels[i]->isChanged())
    {
    	// Retreive message
    	message=c_channels[i]->getMsg();

    	// Send message
			try {
			c_midiOut->sendMessage( &message );
			}
			catch ( RtMidiError &error ) {
			error.printMessage();
			cout << "Virtual Midi port down" <<endl;
			return false;
			}
		}
  }
	
	return true;
}

