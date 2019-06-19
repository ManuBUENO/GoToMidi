#include "opencv2/core/core.hpp"
#include "typesClass.hpp"

using namespace cv;

//// SPOT

// Constructor
Spot::Spot(unsigned int id)
{
	// Initialize spot variables
	c_id = id;
	c_position = Point2i(-1,-1);
	c_coord = Point2f(-1.0,-1.0);
	c_state = STONE_NONE;
	c_changed = false;
	c_stability=0;
}

// Reset states and stability
void Spot::reset()
{
	c_state = STONE_NONE;
	c_state_old = STONE_NONE;
	c_changed = false;
	c_stability = 0;
}

void Spot::updateState(unsigned int state)
{
	// for now, just update state and check if it changed
	if (c_state==state)
	{
		c_changed=false;
		return;
	}
	else
	{
		c_state_old = c_state;
		c_state = state;
		c_changed = true;
	}
}

void Spot::revertState()
{
	c_state = c_state_old;
	c_changed = false;
}

bool Spot::isChanged() const
{
	return c_changed;
}

void Spot::addChannel(Channel* channel)
{
	c_channels.push_back(channel);
}

// GET

unsigned int Spot::getId() const
{
	return c_id;
}


unsigned int Spot::getState() const
{
	return c_state;
}

cv::Point Spot::getCoord() const
{
	return c_coord;
}

cv::Point2i Spot::getPos() const
{
	return c_position;
}

vector<Channel*> Spot::getChannels() const
{
	return c_channels;
}

// SET

void Spot::setCoord(cv::Point2f coord)
{
	c_coord = coord;
}

void Spot::setPos(cv::Point2i position)
{
	c_position = position;
}




//// CHANNEL

// Constructor
Channel::Channel(unsigned int id)
{
	// Initialize spot variables
	//c_mode = "unknown";
	c_id = id;
	c_state = 0;
	c_changed = false;
}

// Reset states and stability
void Channel::reset()
{
  c_state = 0;
	c_changed = false;
}


void Channel::updateState(unsigned int state)
{
	// for now, just update state and check if it changed
	if (c_state==state)
	{
		c_changed=false;
		return;
	}
	else
	{
		c_state = state;
		c_changed = true;
	}
}

bool Channel::isChanged() const
{
	return c_changed;
}

void Channel::addSpot(Spot* spot)
{
	c_spots.push_back(spot);
}

// GET

unsigned int Channel::getId() const
{
	return c_id;
}

unsigned int Channel::getState() const
{
	return c_state;
}


vector<unsigned char> Channel::getMsg()
{
	vector<unsigned char> msg;
	c_changed=false;
	if(c_state==0)
	{
		msg.push_back(c_msgOff[0]);
		msg.push_back(c_msgOff[1]);
		msg.push_back(c_msgOff[2]);
	}
	else
	{
		msg.push_back(c_msgOn[0]);
		msg.push_back(c_msgOn[1]);
		msg.push_back(c_msgOn[2]);
	}
	return msg;
}


std::vector<Spot*> Channel::getSpots() const
{
	return c_spots;
}

// Set

void Channel::setMode(char* mode)
{
	c_mode=mode;
}


void Channel::setMsgOn(unsigned char *msg)
{
	c_msgOn[0] = msg[0];
	c_msgOn[1] = msg[1];
	c_msgOn[2] = msg[2];
}

void Channel::setMsgOff(unsigned char *msg)
{
	c_msgOff[0] = msg[0];
	c_msgOff[1] = msg[1];
	c_msgOff[2] = msg[2];
}
