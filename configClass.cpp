#include "configClass.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/opencv.hpp" 
#include <stdio.h>
#include <string>
#include <time.h> 
#include <cstdint>

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


  // Variables initialisation
  unsigned int i,j;
  
  // Mapping mode
  char mode[50];
  // Board size
  Size goSize;


  if (pFile!=NULL)
  {
    //// Read first line
    // Read mapping mode
    fscanf(pFile,"%s ",mode);
    // Read mapping size
    fscanf(pFile,"%ix%i\n",&(goSize.height),&(goSize.width));
    
    //// Mode random : assign each spot to a random channel
    if(strcmp(mode,MAPP_MODE_RAND)==0)
    {
      // Declare message
      vector<vector<unsigned char>> msgsOFF = this->scanMsg(pFile);
      vector<vector<unsigned char>> msgsON = this->scanMsg(pFile);
      // Check that sizes are equals
      if(msgsON.size() != msgsOFF.size())
      {
        cout << "Mapping error: Number of messages should be equal !!"<<endl;
        cout << "Check mapping.txt"<<endl;
        cout << "Default mapping is loaded"<<endl;
        fclose (pFile);
        pFile=NULL;
      }
      else
      {
        // Init channels
        for(i=0; i<msgsOFF.size();i++)
        {
          c_channels.push_back(new Channel(i));
          c_channels[i]->setMode(mode);
          // Build ON message
          c_channels[i]->setMsgOn(msgsON[i]);
          // Build Off message
          c_channels[i]->setMsgOff(msgsOFF[i]);
        }

        // initialize random seed
        srand (time(NULL));
        // for each spot
        for(i=0;i<GO_N_SPOTS;i++)
        {
          // specify it in a random channel
          int randIndex = rand() % msgsOFF.size();
          // Assign spot to random channel
          c_channels[randIndex]->addSpot(c_spots[i]);
          // Assign random channel to spot
          c_spots[i]->addChannel(c_channels[randIndex]);
        }
      }
    }

    //// Mode sequencer : assign lines of spots to channel
    else if(strcmp(mode,MAPP_MODE_SEQU)==0)
    {
      // Read messages until end character
      vector<vector<unsigned char>> msgs0 = this->scanMsg(pFile);
      vector<vector<unsigned char>> msgs1;
      unsigned int nChannels=0;
      // For each line
      while(msgs0.size()>0)
      {
        msgs1 = this->scanMsg(pFile);
        if((msgs0.size() != msgs1.size()) && msgs1.size()!=0)
        {
          cout << "Mapping error: Number of messages should be equal !!"<<endl;
          cout << "Check mapping.txt"<<endl;
          cout << "Default mapping is loaded"<<endl;
          fclose (pFile);
          pFile=NULL;
          break;
        }
        else
        {
          // Init channels
          for(i=0; i<msgs0.size();i++)
          {
            // Create new channel
            c_channels.push_back(new Channel(nChannels));
            // Set channel mode 
            c_channels[nChannels]->setMode(mode);
            // Build ON message
            c_channels[nChannels]->setMsgOn(msgs0[i]);
            // Build Off message
            if(msgs1.size()!=0)
              c_channels[nChannels]->setMsgOff(msgs1[i]);

            //Assign spots to channel and vice-versa
            for(j=0;j<(unsigned int)(goSize.width);j++)
            {
              unsigned int spot = goSize.width*nChannels + j;
              c_channels[nChannels]->addSpot(c_spots[spot]);
              c_spots[spot]->addChannel(c_channels[nChannels]);
            }
            // Increase channel number
            nChannels++;
          }
        }
        // Read first message for new line 
        msgs0 = this->scanMsg(pFile);
      }
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
    vector<unsigned char> msg;
    // Init channels
    for(i=0;i<GO_N_SPOTS;i++)
    {
      c_channels.push_back(new Channel(i));
      c_channels[i]->setMode((char*)MAPP_MODE_DEFAULT);
      // Build ON message
      msg.clear();
      msg.push_back(144);
      msg.push_back(36+i); //36+i by default
      msg.push_back(100);
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
    cout << c_channels[i]->getId() << " : ";

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


// private

vector<vector<unsigned char>> configClass::scanMsg(FILE * pFile)
{
  // Initialise messages
  vector<vector<unsigned char>> msgs;

  // Initialise tmps
  vector<unsigned char> bytes[3];
  unsigned char symb;
  unsigned int start,end;
  unsigned int i,j,k;

  // For each 3 bytes of message
  for (i=0; i<3 ; i++)
  {
    // Scan byte and symbol 
    fscanf(pFile,"%i%c",&start ,&symb);
    // Check if first character is end character
    if(start>255)
    {
      msgs.clear();
      return msgs;
    }
    // Store byte in bytes list
    bytes[i].push_back(start);

    // "+" symbol means that a number of bytes is specified just after
    if(symb=='+')
    {
      fscanf(pFile,"%i",&end);
      for (j=start+1;j<=(start+end);j++)
      {
        bytes[i].push_back(j);
      }
    }
    else if(symb==':')
    {
      fscanf(pFile,"%i",&end);
      for (j=start+1;j<=end;j++)
      {
        bytes[i].push_back(j);
      }
    }
  }

  // Now put all messages into the vector
  // For each first byte
  for (i=0; i<bytes[0].size() ; i++)
  {
    // For each second byte
    for (j=0; j<bytes[1].size() ; j++)
    {
      // For each third byte
      for (k=0; k<bytes[2].size() ;k++)
      {
        vector<unsigned char> msg;
        msg.push_back(bytes[0][i]);
        msg.push_back(bytes[1][j]);
        msg.push_back(bytes[2][k]);
        msgs.push_back(msg);
      }
    }
  }
  return msgs;
}
