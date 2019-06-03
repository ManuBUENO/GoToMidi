#include "RtMidi.h"
#include <cstdlib>
#include "midiClass.hpp"
#include "configClass.hpp"

using namespace std;
using namespace cv;

bool midiClass::init()
{
  // RtMidiOut constructor
  try {
    c_midiOut = new RtMidiOut();
  }
  catch ( RtMidiError &error ) {
    error.printMessage();
    return false;}
  // Open virtual port
  try {
    c_midiOut->openVirtualPort("MIDI GO OUT");
  }
  catch ( RtMidiError &error ) {
    error.printMessage();
    return false;
  }
	return true;
}

bool midiClass::updateChannels(std::vector <cv::Vec2i> notes)
{
	vector <unsigned char> message;
	for (unsigned int i=0;i<notes.size();i++)
	{
		message.clear();
		//define message type
		message.push_back( 144 );// Note On: 144
		//define note
		unsigned char note = (unsigned char)NOTE_ZERO;
		note+= (unsigned char)notes[i][0];
        message.push_back(note);
		//Attacks defines muting .. 127 mutes ; 0 unmutes
		if(notes[i][1]>0){
			message.push_back(0);
		}
		else{
			message.push_back(127);// Note Off: 128
		}
		//send message
		try {
			c_midiOut->sendMessage( &message );
 		}
		catch ( RtMidiError &error ) {
  		error.printMessage();
  		cout << "Virtual Midi port down" <<endl;
  		return false;
  		}
	}
	return true;
}

bool midiClass::channelsOff()
{
	vector <unsigned char> message;
	for(unsigned int i=0;i<GO_SIZE*GO_SIZE;i++)
	{
		message.clear();
		//note off
		message.push_back( (unsigned char) 128 );// Note Off: 128
		message.push_back( (unsigned char) NOTE_ZERO+i );// Note Off: 128
		message.push_back( (unsigned char) 90); // attack
		try {
			c_midiOut->sendMessage( &message );
 		}
		catch ( RtMidiError &error ) {
  		error.printMessage();
  		cout << "Virtual Midi port down" <<endl;
  		return false;
  	}
  }
  return true;
}
