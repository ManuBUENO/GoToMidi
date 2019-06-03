#include "configClass.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/opencv.hpp" 
#include <stdio.h>


using namespace std;
using namespace cv;

void configClass::init()
{
	// pointer to main GUI
	//c_ptrMainGUI = ptrMainGUI;
	c_corners.reserve(GO_SIZE*GO_SIZE); 
	c_params.reserve(PARAM_N);
  c_params.reserve(PARAM_N);
	this->loadConfig();
  this->loadMapping();
}


void configClass::saveConfig()
{
	// OpenConfigFile
	FILE * pFile;
  	pFile = fopen (CONF_FILENAME,"w+");
  	//vector <Point2f> corners = c_ptrMainGUI->getCorners();
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
    // Else default values 
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
    unsigned int total,start;
    unsigned int spot;//index;
    //float x,y,tmp;
    char mode[50],type[50];

    if (pFile!=NULL)
    {
      //// Read first line
      fscanf(pFile,"%s %s %i %i",mode,type, &total, &start);
      //cout << mode<< " " <<type<< " " << tracksTot<< " " <<tracksStart <<endl;
      // Init mapping size
      c_mapping.resize(total);

      // Mode random : assign each spot to a random channel
      if(strcmp(mode,MAPP_MODE_RAND)==0)
      {
        // for each spot
        for(spot=0;spot<GO_SIZE*GO_SIZE;spot++)
        {
          // specify it in a random channel
          int index = rand() % total;
          c_mapping[index].push_back(spot);
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
      // Init mapping size
      c_mapping.resize(GO_SIZE*GO_SIZE);
      for(spot=0;spot<GO_SIZE*GO_SIZE;spot++)
      {
        // Assign spot to its note 
        c_mapping[spot].push_back(spot);
      }
    }

    //debug: print mapping
    for(unsigned int index=0;index<c_mapping.size();index++)
    {
      cout << 36+index << " : ";
      for(spot=0;spot<c_mapping[index].size();spot++)
      {
        cout << c_mapping[index][spot] << " ";
      }
      cout << endl;
    }
}


// GET
vector <Point2f> configClass::getCorners()
{
  return c_corners;
}

vector <float> configClass::getParams()
{
	return c_params;
}

vector < vector<int> > configClass::getMapping()
{
  return c_mapping;
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
