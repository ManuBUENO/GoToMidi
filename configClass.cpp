#include "configClass.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/opencv.hpp" 
#include <stdio.h>
#include <string>
#include <time.h> 

#include "typesClass.hpp"

using namespace std;
using namespace cv;

configClass::configClass()
{
  // Initialize spots
  for(unsigned int i=0;i<GO_SIZE*GO_SIZE;i++)
  {
    c_spots[i] = new Spot(i);
  }
  // Reserve memory
  c_corners.reserve(4); 
  c_params.reserve(PARAM_N);
}

configClass::~configClass()
{
  unsigned int i;
  // Delete spots
  for(i=0;i<GO_SIZE*GO_SIZE;i++)
  {
    delete c_spots[i];
  }

  // Delete channels
  for(unsigned int i=0;i<c_channels.size();i++)
  {
    delete c_channels[i];
  }

}

void configClass::init()
{
	this->loadConfig();
  this->loadMapping();
}


void configClass::saveConfig() const
{
	// OpenConfigFile
	FILE * pFile;
  pFile = fopen (CONF_FILENAME,"w+");
  int i;

  if (pFile!=NULL)
 	{
 		for(i=0;i<4;i++)
 		{
 			fprintf(pFile,"point%d : %f\t%f\n",i,c_corners[i].x,c_corners[i].y);
 		}
 		
 		fprintf(pFile,"contrast : %f\n",c_params[PARAM_CONTR]);
 		fprintf(pFile,"brightness : %f\n",c_params[PARAM_BRIGH]);
 		fprintf(pFile,"threshold_black_stone : %f\n",c_params[PARAM_THRSB]);
 		fprintf(pFile,"threshold_white_stone : %f\n",c_params[PARAM_THRSW]);
    	fclose (pFile);
    }
    else
    {
    	cout<< "Config error: config.txt does not exist" <<endl;
    } 
}


void configClass::loadConfig()
{

	// OpenConfigFile
	FILE * pFile;
	pFile = fopen (CONF_FILENAME,"r");
	int i;
	float x,y,tmp;
	char bin[50];

	//clear corners
	c_corners.clear();
	c_params.clear();
  
	if (pFile!=NULL)
	  {
 		for(i=0;i<4;i++)
 		{
 			fscanf(pFile,"%s : %f\t%f\n",bin,&x,&y);
 			c_corners.push_back(Point2f(x,y));	
 			//cout << x << endl;
 		}
 		for(i=0;i<PARAM_N;i++)
 		{
 			fscanf(pFile,"%s : %f\n",bin,&tmp);
 			//cout << tmp << endl;
 			c_params.push_back(tmp);
 			
 		}
    	fclose (pFile);
  }
  // slse default values 
  else
  {
  	c_corners.push_back(Point2f(0.1,0.1));
  	c_corners.push_back(Point2f(0.1,0.9));
  	c_corners.push_back(Point2f(0.9,0.9));
  	c_corners.push_back(Point2f(0.9,0.1));
  	c_params.push_back(1.0);
  	c_params.push_back(0.0);
  	c_params.push_back(0.2);
  	c_params.push_back(0.6);
  }	  
}

void configClass::loadMapping()
{
  // OpenConfigFile
    FILE * pFile;
    pFile = fopen (MAPP_FILENAME,"r");

    unsigned int i,j;
    // Mapping mode
    char mode[50];
    // Index of first channel
    unsigned int channelIndexStart=36;
    // Number of channels
    unsigned int channelIndexNumber;
    // Midi message info when stone is present 
    char midiStatus_spotStone[50];
    unsigned int midiData_spotStone;
    // Midi message info when stone is absent
    char midiStatus_spotEmpty[50];
    unsigned int midiData_spotEmpty;

    unsigned char msg[3];
    
  
    if (pFile!=NULL)
    {
      //// Read first parameter
      fscanf(pFile,"%s ",mode);
      
      //// Mode random : assign each spot to a random channel
      if(strcmp(mode,MAPP_MODE_RAND)==0)
      {
        // Read channels parameters
        fscanf(pFile,"%i %i ",&channelIndexStart,&channelIndexNumber);
        // Read Midi parameters for spot with stone
        fscanf(pFile,"%s %i ",midiStatus_spotStone,&midiData_spotStone);
        // Read Midi parameters for spot withOUT stone
        fscanf(pFile,"%s %i ",midiStatus_spotEmpty,&midiData_spotEmpty);

        // Init channels
        for(i=0; i<channelIndexNumber;i++)
        {
          c_channels.push_back(new Channel(i));
          c_channels[i]->setMode(mode);
          // Build ON message
          msg[0] = 144;
          msg[1] = channelIndexStart+i;
          msg[2] = midiData_spotStone;
          c_channels[i]->setMsgOn(msg);
          // Build Off message
          msg[2] = midiData_spotEmpty;
          c_channels[i]->setMsgOff(msg);
        }

        // initialize random seed
        srand (time(NULL));
        // for each spot
        for(i=0;i<GO_N_SPOTS;i++)
        {
          // specify it in a random channel
          int randIndex = rand() % channelIndexNumber;
          // Assign spot to random channel
          c_channels[randIndex]->addSpot(c_spots[i]);
          // Assign random channel to spot
          c_spots[i]->addChannel(c_channels[randIndex]);
        }
      }

      //// Mode sequencer : TBD
      else if(strcmp(mode,MAPP_MODE_SEQU)==0)
      {
        // TBD
      }

      // Mode unknown: go to default
      else
      {
        fclose (pFile);
        // Specify pFile as null to enter default condition
        pFile=NULL;
      }
    }
    // By default, assign each spot to a channel incrementally
    if (pFile==NULL)
    {
      // Init channels
      for(i=0;i<GO_N_SPOTS;i++)
      {
        c_channels.push_back(new Channel(i));
        c_channels[i]->setMode((char*)"note");
        // Build ON message
        msg[0] = 144;
        msg[1] = channelIndexStart+i; //36+i by default
        msg[2] = 100;
        c_channels[i]->setMsgOn(msg);
        // Build Off message
        msg[0] = 128;
        c_channels[i]->setMsgOff(msg);
        // Assign spot to the new channel
        c_channels[i]->addSpot(c_spots[i]);
        // Assign channel channel to spot
        c_spots[i]->addChannel(c_channels[i]);
      }
    }

    //debug: print mapping
    for(i=0;i<c_channels.size();i++)
    {
      cout << c_channels[i]->getId() + channelIndexStart << " : ";

      vector <Spot*> spots = c_channels[i]->getSpots();

      for(j=0;j<spots.size();j++)
      {
        cout << spots[j]->getId() << " ";
      }
      cout << endl;
    }
}


// GET
vector <Point2f> configClass::getCorners() const
{
  return c_corners;
}

vector <float> configClass::getParams() const
{
	return c_params;
}

void configClass::getSpots(Spot** spots) const
{
  for (unsigned int i=0;i<GO_SIZE*GO_SIZE;i++)
  {
    spots[i]=c_spots[i];
  }
}

vector <Channel*>  configClass::getChannels() const
{
  return c_channels;
}

// SET
void configClass::setCorners(vector <Point2f> corners)
{
  c_corners = corners;
}

void configClass::clearCorners()
{
	c_corners.clear();
}
