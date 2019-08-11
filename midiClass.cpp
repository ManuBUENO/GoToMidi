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
	// Define eighth at 0
	c_clockCount=0;
	// Define clockcount at 0
	c_clockCount=0;
	// Define clockcount at false
	c_tic=false;

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
	  // Ignore sysex, NOT timing, or active sensing messages.
  	c_midiIn->ignoreTypes( true, false, true );

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
  	// Launch thread
    c_threadStatus = STATE_RUNNING;
	  c_thread = thread(&midiClass::midiThread, this);
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
  	if(c_threadStatus != STATE_KILLED)
	  	{
	  	// Kill thread
	  	c_threadStatus = STATE_KILL; 
	  	// Desynchronize thread
	  	c_synchCV.notify_one();
	  	// Wait for thread to end 
	  	c_thread.join();
	  	// State thread as killed
	  	c_threadStatus = STATE_KILLED; 
    }
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

// Manage MIDI control messages. Manages start and stop sequencer,
// Sends a signal to thread each eigth notes
void midiClass::clockSynch(vector< unsigned char > *msgIn )
{
	// If seq is running and message is clock, increase counter
	if((this->getSeqState()==SEQ_STATE_RUNNING) && (msgIn->at(0)==MIDI_STATUS_CLOCK))
	{
		c_clockCount++;
		//cout <<"clock !"<<endl;
		if (c_clockCount%12==0)
		{
			c_tic=true;
			c_synchCV.notify_one();
		}
	}	

	// If message is start, set sequencer state at running
	else if(msgIn->at(0)==MIDI_STATUS_START)
	{
		this->setSeqState(SEQ_STATE_RUNNING);
		//cout <<"GO !"<<endl;
		c_clockCount=0;
		c_eighthCount=0;
	}

	// If message is stop, set sequencer state as ready
	else if(msgIn->at(0)==MIDI_STATUS_STOP)
	{
		//cout <<"STOP !"<<endl;
		this->setSeqState(SEQ_STATE_READY);
		c_clockCount=0;
	}

}

void midiClass::manageSequencer()
{
	unsigned int i;
	vector <unsigned char> message;

	// For each channel
	for(i=0;i<c_channels.size();i++)
  {
  	// Check if moment is activated
  	if((c_channels[i]->getState() & (0x01<<c_eighthCount)) > 0x00)
  	{
  		// Retreive ON message
  		message=c_channels[i]->getMsg(1);

    	// Send this message
    	if(message.size()>0)
				c_midiOut->sendMessage( &message );
  	}
  	// else noteoff ???? 				------------- !!!! -----------
  		//message=c_channels[i]->getMsg(0);
  }

	// Increase counter
	//cout << c_eighthCount <<endl;
	int oldEighthCount = c_eighthCount;
	// Increase counter
  c_eighthCount++;
	// Reset counter if needed
  if(c_eighthCount>=GO_SIZE-1)
  	 c_eighthCount=0;

	// Darken previous column
	// For each channel : bright active spots
	for(i=0;i<c_channels.size();i++)
	{
		// Get spot 
		Spot* spot = c_channels[i]->getSpot(c_eighthCount+1);
		spot->brighten();
		spot = c_channels[i]->getSpot(oldEighthCount+1);
		spot->darken();
	}
}

void midiClass::midiThread()
{
	// Check mode
	// Depending on mode, launch midi thread / callback
  // Only one mode at a time for now. So check only first channels
  char mode[50];
  c_channels[0]->getMode(mode);
  // Random or default mode
  if(strcmp(mode,MAPP_MODE_RAND)==0 || strcmp(mode,MAPP_MODE_DEFAULT)==0)
  {	
  	// Until thread needs to be killed
  	do 
		{
	  	// Sleep a bit  
	    usleep(1000000/MIDIRATE);
	  	// updateChannels
	    this->updateChannels();
	  } while(c_threadStatus != STATE_KILL);
  }
  else if (strcmp(mode,MAPP_MODE_SEQU)==0)
  {
  	//cout << "thread ready" <<endl;
  	// Until thread needs to be killed
  	c_tic=false;
  	do 
		{
			//cout <<"thread waiting"<<endl;
	  	// Wait for signal, check that tic is real or thread needs to be killed
	  	unique_lock<mutex> mlock(c_synchMut);
	  	c_synchCV.wait(mlock,[&](){return (c_tic || c_threadStatus==STATE_KILL);});
	  	c_tic=false;
	  	// Sequencer does his thing if running
	  	if(c_threadStatus==STATE_RUNNING)
	  		this->manageSequencer();

	 } while(c_threadStatus != STATE_KILL);
	}
	//cout << "thread killed" <<endl;
}

void midiClass::midiCallback(double deltaTime, vector< unsigned char > *msg, void *voidPtr)
{
	// Cast pointer
  midiClass* midiClassPtr = (midiClass *) voidPtr;
  // If sequenceer is alive 
	if(midiClassPtr->getSeqState()!= SEQ_STATE_DEAD)
	{
		// Synchronize thread
		midiClassPtr->clockSynch(msg);

		//midiClassPtr->manageSequencer(msg);
	}
}

