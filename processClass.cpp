#include "processClass.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/opencv.hpp" 
#include <stdio.h>

#include "configClass.hpp"
#include "typesClass.hpp"

using namespace std;
using namespace cv;

processClass::processClass(configClass * ptrMainConfig) :
c_ptrMainConfig(ptrMainConfig) {}


// Initialize variables, mapping and states
void processClass::init()
{
  // init variables
  c_iterSinceValid=0;

  // Get channels
  c_channels = c_ptrMainConfig->getChannels();

  // Get spots
  c_ptrMainConfig->getSpots(c_spots);

}

// Revert variables to initial state
void processClass::reset()
{
  // Reset stones
  for(unsigned int i=0;i<GO_N_SPOTS;i++)
  {
    c_spots[i]->reset();
  }
  // Reset channels
  for(unsigned int i=0;i<c_channels.size();i++)
  {
    c_channels[i]->reset();
  }
  // Reset counters
  c_iterSinceValid=0;
} 


// Process Raw Camera image into analysable image 
Mat processClass::preprocess(const Mat& img) const
{
    Mat fix;
    vector <Point2f> cornersSrc;
    vector <Point2f> cornersDst;

    // Get parameters
    vector <float> params;
    params = c_ptrMainConfig->getParams();

    //// Transform image geometry to match board only
    // Get board corners coordinates
    cornersSrc = c_ptrMainConfig->getCorners();
    int i;
    // Transform coordiantes in pixels
    for(i=0;i<4;i++)
    {
      cornersSrc[i].x = float(img.cols) *cornersSrc[i].x;
      cornersSrc[i].y = float(img.rows) *cornersSrc[i].y;
    }

    // Set goal coordinates (dst image corners)
    cornersDst.push_back(Point2f(0,0));
    cornersDst.push_back(Point2f(0,DOWN_IMG_H-1));
    cornersDst.push_back(Point2f(DOWN_IMG_W-1,DOWN_IMG_H-1));
    cornersDst.push_back(Point2f(DOWN_IMG_W-1,0));

    // Compute homography matrix
    Mat h = findHomography(cornersSrc, cornersDst);
    // Apply homography matrix to transfom image geometry
    warpPerspective(img,fix, h, Size(DOWN_IMG_W,DOWN_IMG_H));
    
    // get blue composant
    Mat fixBGR[3];
    Mat fixB;
    split(fix,fixBGR);
    fixB=fixBGR[0];

    //Apply gaussian blur
    GaussianBlur(fixB, fixB, Size(3, 3), 0);
    // change contrast and brightness
    fixB.convertTo(fixB, -1, 1 , params[PARAM_BRIGH]);
    fixB.convertTo(fixB, -1, params[PARAM_CONTR], 0);
    fixB.convertTo(fix,CV_8UC3);

    //convert back to BGR
    cvtColor(fix,fix,CV_GRAY2BGR);
    return fix;
}


void processClass::computeCoordinates(Size imgSize)
{
  int i,j;
  float x,y;
  // determine board edge thickness
  vector <float> edges;
  // compute edges
  edges.push_back(float(GO_EDGEL_RATIO*imgSize.width));
  edges.push_back(float(GO_EDGEB_RATIO*imgSize.height));
  edges.push_back(float(GO_EDGER_RATIO*imgSize.width));
  edges.push_back(float(GO_EDGET_RATIO*imgSize.height));

  //For each spot
  for(i=0;i<GO_SIZE;i++)
  {
    for(j=0;j<GO_SIZE;j++)
    {
      // Compute coordinates
      x=edges[0]+(imgSize.width-edges[0]-edges[2])*i/8;
      y=edges[3]+(imgSize.height-edges[1]-edges[3])*j/8;
      c_spots[i+GO_SIZE*j]->setCoord(Point((int)x,(int)y));
    }
  }

}

// Scan analysable image to find stones
void processClass::scanBoard(const Mat& boardImg)
{
  Mat img;
  // Get parameters
  vector <float> params;
  params = c_ptrMainConfig->getParams();
  // Copy and grey img
  boardImg.copyTo(img);
  cvtColor(img,img,CV_BGR2GRAY);
  
  //For each board spots
  unsigned int i;
  for(i=0;i<GO_N_SPOTS;i++)
  {
    //retreive spot coordinate
    Point pt(c_spots[i]->getCoord());
    //define rect around it
    Rect roi(pt.x-ROI_RAD,pt.y-ROI_RAD,ROI_RAD,ROI_RAD);
    //extract rect from imag
    Mat moi(boardImg,roi);
    //get rect sum
    int result = (int)cv::sum(moi)[0];
    //set result depending on sum
    if (result<(float(ROI_MAX)*params[PARAM_THRSB]))
    {
      c_spots[i]->updateState(STONE_BLACK);
    }
    else if(result>=(float(ROI_MAX)*params[PARAM_THRSW]))
    {
      c_spots[i]->updateState(STONE_WHITE);
    }
    else
    {
     c_spots[i]->updateState(STONE_NONE);
    }
  }
}

// Checks if board changes are logical (discard hand for example)
void processClass::checkBoardChanges()
{
  // Find all changed spots
  vector <Spot*> changedSpots;
  unsigned int i;
  for(i=0;i<GO_N_SPOTS;i++)
  {
    if(c_spots[i]->isChanged())
      changedSpots.push_back(c_spots[i]);
  }

  // Too much changes
  if(changedSpots.size()>1 && c_iterSinceValid<FRAMERATE)
  {
    // Too much stones changed, not possible
    // Revert all spots to previous state
    // After 1 second, validate spots anyway. Timing not accurate
    c_iterSinceValid++;
    for(i=0;i<changedSpots.size();i++)
    {
      changedSpots[i]->revertState();
    }  
  }

  // No change or one stone changed: OK
  // Also OK if counter >framerate
  else
  {
    c_iterSinceValid=0;
  }
}

// Depending on mapping, define midi channels states
void processClass::computeChannelStates()
{
  // Init variables
  unsigned int i,j,sum;
  vector <Channel*> impactedChannels;

  // WARNING MAPPING MODE TO IMPLEMENT

   // Find all impacted channels
  for(i=0;i<GO_N_SPOTS;i++)
  {
    if(c_spots[i]->isChanged())
    {
      //cout << "Spot " <<c_spots[i]->getId()<< " changed"<<endl;
      // Get channels from spot
      vector <Channel*> spotChannels = c_spots[i]->getChannels();
      // Add them to impacted channels list
      impactedChannels.insert(impactedChannels.end(),spotChannels.begin(),spotChannels.end());
    }
  }

  // CASE RANDOM
  {
    // For each impacted channels
    for(i=0;i<impactedChannels.size();i++)
    {
      
      // Get spots from channel
      vector <Spot*> channelSpots = impactedChannels[i]->getSpots();
      // Sum spot states
      sum=0;
      for(j=0;j<channelSpots.size();j++)
      {
       // cout << "spot " << channelSpots[j]->getId()<<" state = "<< channelSpots[j]->getState()<<endl;
        sum += channelSpots[j]->getState();
      }

      // No stone and channel ON --> channel OFF
      if(sum==0 && impactedChannels[i]->getState()==1)
      {
        impactedChannels[i]->updateState(0); // channel OFF
        //cout << "channel " <<impactedChannels[i]->getId()<<" OFF"<<endl;
      }
      // Stones present and channel off --> channel ON
      else if (sum>0 && impactedChannels[i]->getState()==0)
      {
        impactedChannels[i]->updateState(1); // channel ON
        //cout << "channel " <<impactedChannels[i]->getId()<<" ON"<<endl;
      }
    }
  }
}

void processClass::channelsOff()
{
  for(unsigned int i=0;i<c_channels.size();i++)
  {
    c_channels[i]->updateState(0);
  }
}


bool processClass::isMoving(const Mat& imageOld, const Mat& imageNew) const
{
  Mat imgOld,imgNew;
  imageOld.copyTo(imgOld);
  imageNew.copyTo(imgNew);
  //compute difference between first frame and current frame
  absdiff(imgOld, imgNew, imgNew);
  // binarize difference
  threshold(imgNew, imgNew, 20, 1, THRESH_BINARY);
  // determine purcentage of changed pixels
  float result = (float)(100*cv::sum(imgNew)[0]/(imgNew.rows*imgNew.cols));
  
  //If more than 0.5% of pixels changed, consider movement
  if(result>0.5){
    return true;
  }
  return false;
}



