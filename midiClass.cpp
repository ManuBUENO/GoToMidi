#include "RtMidi.h"
#include <cstdlib>
#include <unistd.h>
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

	// Define thread as killed
	c_threadStatus = STATE_KILLED;
	// Define sequencer as dead
	c_seqState = SEQ_STATE_DEAD;

  //// Init Midi classes

  // Init MidiOut
  try {
    c_midiOut = new RtMidiOut();
  }
  catch ( RtMidiError &error ) {
    error.printMessage();
    return false;}


  // Open MidiOut virtual port
  try {
    c_midiOut->openVirtualPort(MIDIPORTOUT);
  }
  catch ( RtMidiError &error ) {
    error.printMessage();
    return false;
  }

  // Depending on mode, set MidiIn
  // Only one mode at a time for now. So check only first channels
  // Check mode
  char mode[50];
  c_channels[0]->getMode(mode);
  // Sequencer mode: need for midiIn
  if(strcmp(mode,MAPP_MODE_SEQU)==0)
  {
    // Init MidiIn
	  try {
	    c_midiIn = new RtMidiIn();
	  }
	  catch ( RtMidiError &error ) {
	    error.printMessage();
	    return false;}

	  // Open MidiIn virtual port
	  try {
	    c_midiIn->openVirtualPort(MIDIPORTIN);
	  }
	  catch ( RtMidiError &error ) {
	    error.printMessage();
	    return false;
	  }

	  // Set midiIn callback to get messages
	  c_midiIn->setCallback(&midiClass::midiCallback, (void*)this);
	  // Ignore sysex, timing, or active sensing messages.
  	c_midiIn->ignoreTypes( true, true, true );

  }
	return true;
}

void midiClass::start()
{
  // Depending on mode, launch midi thread / callback
  // Only one mode at a time for now. So check only first channels
  // Check mode
  char mode[50];
  c_channels[0]->getMode(mode);
  // Random or default mode
  if(strcmp(mode,MAPP_MODE_RAND)==0 || strcmp(mode,MAPP_MODE_DEFAULT)==0)
  {
    // Launch thread
    c_threadStatus = STATE_RUNNING;
	  c_thread = thread(&midiClass::midiThread, this);
  }
  else if(strcmp(mode,MAPP_MODE_SEQU)==0)
  {
  	//cout << "Ready !" <<endl;
    // Activate midi callback
    this->setSeqState(SEQ_STATE_READY);
  } 
}

void midiClass::stop()
{
	// Depending on mode, stop midi thread / callback
  // Only one mode at a time for now. So check only first channels
  // Check mode
  char mode[50];
  c_channels[0]->getMode(mode);
  // Random or default mode: kill thread if it is running
  if(strcmp(mode,MAPP_MODE_RAND)==0 || strcmp(mode,MAPP_MODE_DEFAULT)==0)
  {
  	if(c_threadStatus != STATE_KILLED)
  	{
	  	// Kill thread
	  	c_threadStatus = STATE_KILL; 
	  	// Wait for thread to end 
	  	c_thread.join();
	  	// State thread as killed
	  	c_threadStatus = STATE_KILLED; 
  	}
  }
  else if(strcmp(mode,MAPP_MODE_SEQU)==0)
  {
    // Deactivate callback
    this->setSeqState(SEQ_STATE_DEAD);
  } 
}

bool midiClass::updateChannels()
{
	// For all changed channels
  unsigned int i;
  vector <unsigned char> message;
  for(i=0;i<c_channels.size();i++)
  {
  	// Check if channel changed
    if(c_channels[i]->isChanged())
    {
    	// Retreive message to send (according to channel state)
    	message=c_channels[i]->getMsg(c_channels[i]->getState());

    	// Send this message
    	if(message.size()>0)
				c_midiOut->sendMessage( &message );
		}
	}
	return true;
}

int midiClass::getSeqState()
{
	return c_seqState;
}

// Set seqState with mutex
void midiClass::setSeqState(int state)
{
	c_stateMut.lock();
	c_seqState=state;
	c_stateMut.unlock();
} 


void midiClass::manageSequencer(vector< unsigned char > *msgIn )
{
	unsigned int i;
	vector <unsigned char> message;
	// Manage start
	if(c_seqState==SEQ_STATE_READY)
	{
		if(msgIn->at(1)==METRO_TIC)
			 this->setSeqState(0);
	}

	if(c_seqState>SEQ_STATE_READY)
	{
		// For each channel
  	for(i=0;i<c_channels.size();i++)
	  {
	  	// Check if moment is activated
	  	if((c_channels[i]->getState() & (0x01<<c_seqState)) > 0x00)
	  	{
	  		if(msgIn->at(0) == (MIDI_NOTEON + METRO_CHAN) )
	  		{
	  			// Retreive ON message
    			message=c_channels[i]->getMsg(1);
	  		}
	  		else if (msgIn->at(0) == (MIDI_NOTEOFF + METRO_CHAN) )
	  		{
	  			//Send noteOFF
	  			message=c_channels[i]->getMsg(0);
	  		}
	    	// Send this message
	    	if(message.size()>0)
					c_midiOut->sendMessage( &message );
	  	}
	  }

		// At each metronome NoteOff
		if(msgIn->at(0) == (MIDI_NOTEOFF + METRO_CHAN) )
		{
			//cout << c_seqState+1 <<endl;
			int oldSeqState = c_seqState;
			// Increase counter
		  this->setSeqState(c_seqState+1);
	  	// Reset counter if needed
		  if(c_seqState>=GO_SIZE-1)
		  	 this->setSeqState(0);

			// Darken previous column
			// For each channel : bright active spots
	  	for(i=0;i<c_channels.size();i++)
	  	{
	  		// Get spot 
	  		Spot* spot = c_channels[i]->getSpot(c_seqState+1);
	  		spot->brighten();
	  		spot = c_channels[i]->getSpot(oldSeqState+1);
				spot->darken();
			}
		}
	}
}

void midiClass::midiThread()
{
  // Loop
  do 
  {
  	// Depending on mode, launch midi thread / callback
	  // Only one mode at a time for now. So check only first channels
	  // Check mode
	  char mode[50];
	  c_channels[0]->getMode(mode);
	  // Random or default mode
	  if(strcmp(mode,MAPP_MODE_RAND)==0 || strcmp(mode,MAPP_MODE_DEFAULT)==0)
	  {
	  	// Sleep a bit  
	    usleep(1000000/MIDIRATE);
	  	// updateChannels
	    this->updateChannels();
	  }
	 /* else if (strcmp(mode,MAPP_MODE_SEQU)==0)
	  {
	  	this->manageSequencer();
	  }*/
  } while(c_threadStatus != STATE_KILL);
}

void midiClass::midiCallback(double deltaTime, vector< unsigned char > *msg, void *voidPtr)
{
	// Cast pointer
  midiClass* midiClassPtr = (midiClass *) voidPtr;
	if(midiClassPtr->getSeqState()!= SEQ_STATE_DEAD)
	{
		// Read msg
		// If msg==tic & ON
			// read clock

			//if ready
				//If c_clock=0,
					//c_clock=clock
				//else
					//c_period=(clock-c_clock/4
					//c_clock=clock
			    //state=0 //go
		  // else if state>running



		midiClassPtr->manageSequencer(msg);
	}
}

