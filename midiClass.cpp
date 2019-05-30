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

bool midiClass::updateNotes(std::vector <cv::Vec3i> notes)
{
	vector <unsigned char> message;
	for (unsigned int i=0;i<notes.size();i++)
	{
		message.clear();
		//define message type
		if(notes[i][2]>0){
			message.push_back( 144 );// Note On: 144
		}
		else{
			message.push_back( 128 );// Note Off: 128
		}
		//define note
		unsigned char note = (unsigned char)NOTE_ZERO;
		note+= (unsigned char)notes[i][0];
		note+= (unsigned char) (GO_SIZE*notes[i][1]);
		message.push_back(note);
		//cout << (int)note <<endl;
		//define attack
		message.push_back(NOTE_ATTACK);
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

bool midiClass::notesOff()
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