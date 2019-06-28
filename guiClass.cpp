#include "guiClass.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/opencv.hpp" 
#include <string>
#include <iostream>

#include "configClass.hpp"

using namespace std;
using namespace cv;

guiClass::guiClass(configClass* ptrMainConfig) : c_ptrMainConfig(ptrMainConfig){}

// Create GUI
void guiClass::init(const string& windowName)
{
  // Init variables
  c_windowName = windowName;

  // Init GUI full image
  c_imgFull = Mat(WNDW_H, WNDW_W, CV_8UC3, Scalar(128,128,128));
  
  // Init top and down image
  c_imgTop = Mat(TOP_IMG_H, TOP_IMG_W, CV_8UC3, Scalar(128,128,128));
  c_imgDown = Mat(DOWN_IMG_H, DOWN_IMG_W, CV_8UC3, Scalar(128,128,128));
  
  // Draw text zone
  c_imgDispText = Mat(TEXT_IMG_H, TEXT_IMG_W, CV_8UC3, Scalar(255,255,255));
  c_dispText = "";c_dispText = "";
  c_textPoint = Point2f(TEXT_START_X,TEXT_START_Y);
  
  // Init buttons
  c_imgBtn1 = Mat(BTN1_IMG_H, BTN1_IMG_W, CV_8UC3, Scalar(0,255,0));
  c_stateBtn1 = false;
  putText(c_imgBtn1,"Set board coordinates", Point2f(20,20),
          cv::FONT_HERSHEY_COMPLEX_SMALL, 0.5,
          cv::Scalar(0,0,0), 1, CV_AA);

  
  c_imgBtn2 = Mat(BTN2_IMG_H, BTN2_IMG_W, CV_8UC3, Scalar(0,255,0));
  c_stateBtn2 = false;
  putText(c_imgBtn2,"Update parameters", Point2f(20,20),
          cv::FONT_HERSHEY_COMPLEX_SMALL, 0.5,
          cv::Scalar(0,0,0), 1, CV_AA);

  c_imgBtn3 = Mat(BTN3_IMG_H, BTN3_IMG_W, CV_8UC3, Scalar(0,255,0));
  c_stateBtn3 = false;
  putText(c_imgBtn3,"Run recognition", Point2f(20,20),
          cv::FONT_HERSHEY_COMPLEX_SMALL, 0.5,
          cv::Scalar(0,0,0), 1, CV_AA);

  // Init State
  this->setState(STATE_MAINMENU);

  // Init data

  // Get spots
  c_ptrMainConfig->getSpots(c_spots);

  // Init window
  namedWindow(c_windowName, CV_WINDOW_AUTOSIZE);
  resizeWindow(c_windowName,WNDW_W,WNDW_H);

  // build first screen
  c_imgTop.copyTo(c_imgFull(Rect(TOP_IMG_X,TOP_IMG_Y,c_imgTop.cols, c_imgTop.rows)));
  c_imgDown.copyTo(c_imgFull(Rect(DOWN_IMG_X,DOWN_IMG_Y,c_imgDown.cols, c_imgDown.rows)));
  c_imgDispText.copyTo(c_imgFull(Rect(TEXT_IMG_X,TEXT_IMG_Y,c_imgDispText.cols, c_imgDispText.rows)));
  c_imgBtn1.copyTo(c_imgFull(Rect(BTN1_IMG_X,BTN1_IMG_Y,c_imgBtn1.cols, c_imgBtn1.rows)));
  c_imgBtn2.copyTo(c_imgFull(Rect(BTN2_IMG_X,BTN2_IMG_Y,c_imgBtn2.cols, c_imgBtn2.rows)));
  c_imgBtn3.copyTo(c_imgFull(Rect(BTN3_IMG_X,BTN3_IMG_Y,c_imgBtn3.cols, c_imgBtn3.rows)));

  // Launch thread
  c_thread = thread(&guiClass::guiThread, this);
  // Detach thread since it will be killed anyway
  c_thread.detach();

}

void guiClass::updateTopImg(const Mat& img)//, vector<Point2f> corners)
{
  // Define the destination size
  Size imgSize;
  if(c_state==STATE_CONFIGCOORDS){
    // When configuring coordinates, show full screen
    imgSize = Size(TOP_IMG_W,WNDW_H);
  }
  else{
    imgSize = Size(TOP_IMG_W,TOP_IMG_H);
  }
  // Resize img
  resize(img,c_imgTop,imgSize);

  // Retreive corners corrdinates
  vector <Point2f> corners = c_ptrMainConfig->getCorners();
  if (corners.size()==4)
  {
    int i;
    //Point2f = tmpP;
    vector<Point> vp;
    for(i=0;i<4;i++)
    {
      vp.push_back(Point((int)(corners[i].x*imgSize.width),
                         (int)(corners[i].y*imgSize.height)));
    } 
    const Point* p = &vp[0];
    int n = (int)vp.size();
    polylines(c_imgTop, &p, &n,1, true, Scalar(0,255,0), 1, CV_AA);
  }
  c_imgTop.copyTo(c_imgFull(Rect(TOP_IMG_X,TOP_IMG_Y,c_imgTop.cols, c_imgTop.rows)));
}

void guiClass::updateDownImg(const Mat& img)
{
  resize(img,c_imgDown,c_imgDown.size());
  int i,brightness;
  int state; 
  
  Scalar color(0,0,0);
  for(i=0;i<GO_N_SPOTS;i++)
  {
    brightness = c_spots[i]->isBright()?255:150;
    state =c_spots[i]->getState();
    if(state==STONE_NONE){
      color = Scalar(brightness,0,0);}
    else if(state==STONE_BLACK){
      color = Scalar(0,brightness,0);}
    else{
      color = Scalar(0,0,brightness);}
    circle( c_imgDown,
            c_spots[i]->getCoord(),
            5,
            color,
            3,8,0);
  }
  c_imgDown.copyTo(c_imgFull(Rect(DOWN_IMG_X,DOWN_IMG_Y,c_imgDown.cols, c_imgDown.rows)));
}

void guiClass::clearImgs()
{
  Mat greyScr(TOP_IMG_H+DOWN_IMG_H, TOP_IMG_W, CV_8UC3, Scalar(128,128,128));
  greyScr.copyTo(c_imgFull(Rect(TOP_IMG_X,TOP_IMG_Y,greyScr.cols, greyScr.rows)));
}

void guiClass::writeTxt(string txt,bool newLine)
{
  if(txt.size()==0)
    return;
  c_dispText.insert(c_dispText.size(),txt);
  for(unsigned int i=0;i<txt.size();i++)
  {
    putText(c_imgDispText,txt.substr(i,1),
          c_textPoint,
          cv::FONT_HERSHEY_COMPLEX_SMALL,
          0.5,
          cv::Scalar(0,0,0),
          1,
          CV_AA);
    c_textPoint.x= c_textPoint.x+TEXT_GAP_X;
    if(c_textPoint.x>=TEXT_IMG_W-2*TEXT_GAP_X-TEXT_START_X)
    {
      c_textPoint.y = c_textPoint.y+TEXT_GAP_Y;
      c_textPoint.x=TEXT_START_X;
    }
  }
  if(newLine){
    c_dispText.insert(c_dispText.size(),"\n");
    c_textPoint.y = c_textPoint.y+TEXT_GAP_Y;
    c_textPoint.x=TEXT_START_X;
  }
  c_imgDispText.copyTo(c_imgFull(Rect(TEXT_IMG_X,TEXT_IMG_Y,c_imgDispText.cols, c_imgDispText.rows)));
}

void guiClass::clearTxt()
{
  c_dispText.clear();
  c_textPoint = Point2f(TEXT_START_X,TEXT_START_Y);
  c_imgDispText = Mat(TEXT_IMG_H, TEXT_IMG_W, CV_8UC3, Scalar(255,255,255));
  c_imgDispText.copyTo(c_imgFull(Rect(TEXT_IMG_X,TEXT_IMG_Y,c_imgDispText.cols, c_imgDispText.rows)));
}

void guiClass::toogleButton(int btnID)
{
  switch(btnID)
  {
    case BTN1_ID:
    {
      if(!c_stateBtn1)
      {
        // Go to configuration mode
        this->setState(STATE_CONFIGCOORDS);
        // Clear corners
        c_ptrMainConfig->clearCorners();
        // Change appearance
        c_imgBtn1 = Mat(BTN1_IMG_H, BTN1_IMG_W, CV_8UC3, Scalar(0,100,0));
        putText(c_imgBtn1,"Set board coordinates", Point2f(20,20),
                cv::FONT_HERSHEY_COMPLEX_SMALL, 0.5,
                cv::Scalar(0,0,0), 1, CV_AA);
      }
      else
      {
        //clear screen
        this->clearImgs();
        // Save config
        this->setState(STATE_SAVECONFIG);
        //change appearance
        c_imgBtn1 = Mat(BTN1_IMG_H, BTN1_IMG_W, CV_8UC3, Scalar(0,255,0));
        putText(c_imgBtn1,"Set board coordinates", Point2f(20,20),
                cv::FONT_HERSHEY_COMPLEX_SMALL, 0.5,
                cv::Scalar(0,0,0), 1, CV_AA);
      }
      // toogle button state
      c_stateBtn1=!c_stateBtn1;
      // update screen
      c_imgBtn1.copyTo(c_imgFull(Rect(BTN1_IMG_X,BTN1_IMG_Y,c_imgBtn1.cols, c_imgBtn1.rows)));
 
      break;
    }
      
    case BTN2_ID:
    {
    if(!c_stateBtn2)
      {
        // Go to configuration mode
        this->setState(STATE_CONFIGPARAMS);
        //change appearance
        c_imgBtn2 = Mat(BTN2_IMG_H, BTN2_IMG_W, CV_8UC3, Scalar(0,100,0));
        putText(c_imgBtn2,"Update parameters", Point2f(20,20),
                cv::FONT_HERSHEY_COMPLEX_SMALL, 0.5,
                cv::Scalar(0,0,0), 1, CV_AA);
      }
      else
      {
        //save config
        this->setState(STATE_SAVECONFIG);
        //change appearance
        c_imgBtn2 = Mat(BTN2_IMG_H, BTN2_IMG_W, CV_8UC3, Scalar(0,255,0));
        putText(c_imgBtn2,"Update parameters", Point2f(20,20),
                cv::FONT_HERSHEY_COMPLEX_SMALL, 0.5,
                cv::Scalar(0,0,0), 1, CV_AA);
      }
      // toogle button state
      c_stateBtn2=!c_stateBtn2;
      // update screen
      c_imgBtn2.copyTo(c_imgFull(Rect(BTN2_IMG_X,BTN2_IMG_Y,c_imgBtn2.cols, c_imgBtn2.rows)));
      break;
    }
    case BTN3_ID:
    {
    if(!c_stateBtn3)
      {
        // Go to running mode
        this->setState(STATE_STARTING);
        //change appearance
        c_imgBtn3 = Mat(BTN3_IMG_H, BTN3_IMG_W, CV_8UC3, Scalar(0,0,200));
        putText(c_imgBtn3,"Stop recognition", Point2f(20,20),
                cv::FONT_HERSHEY_COMPLEX_SMALL, 0.5,
                cv::Scalar(0,0,0), 1, CV_AA);
      }
      else
      {
        //Go to main menu state
        this->setState(STATE_STOPPING);
        //change appearance
        c_imgBtn3 = Mat(BTN3_IMG_H, BTN3_IMG_W, CV_8UC3, Scalar(0,255,0));
        putText(c_imgBtn3,"Run recognition", Point2f(20,20),
                cv::FONT_HERSHEY_COMPLEX_SMALL, 0.5,
                cv::Scalar(0,0,0), 1, CV_AA);
      }
      // toogle button state
      c_stateBtn3=!c_stateBtn3;
      // update screen
      c_imgBtn3.copyTo(c_imgFull(Rect(BTN3_IMG_X,BTN3_IMG_Y,c_imgBtn3.cols, c_imgBtn3.rows)));
      break;
    }
    default:
      break;
  }
}

// GET

const vector <Point2f> guiClass::getCorners() const
{
  return c_ptrMainConfig->getCorners();
}

int guiClass::getState() const
{
  return c_state;
}

// SET


void guiClass::setCorners(vector <Point2f> corners)
{
  c_ptrMainConfig->setCorners(corners);
}

void guiClass::setState(int state)
{
  // lock before modifying
  c_stateMut.lock();
  c_state = state;
  c_stateMut.unlock();

  // Clear old text
  this->clearTxt();
  // Write appropriate text
  switch(c_state)
  {
    case(STATE_MAINMENU):
    {
      this->writeTxt("Welcome to go-board recogniser !",true);
      this->writeTxt("",true);
      this->writeTxt("You can change the square coordinates, click button",true);
      this->writeTxt("Change bottom image parameters in ./config.txt, then click update",true);
      this->writeTxt("Start recognition to get some rythm",true);
      this->writeTxt("Peace",true);  
      break;
    }
    case(STATE_CONFIGCOORDS):
    {
      this->writeTxt("Time to set new board coordinates !",true);
      this->writeTxt("",true);
      this->writeTxt("Draw a square around the board",true);
      this->writeTxt("To do so, click on its 4 corners, starting up-left, and going counter clockwise",true);
      this->writeTxt("Peace",true);  
      break;
    }
    case(STATE_RUNNING):
    {
      this->writeTxt("Poum Tsi Poum Poum tsi",true);
      break;
    }
  }
}


//private

void guiClass::updateScreen() const
{
  imshow(c_windowName,c_imgFull);
}

void guiClass::guiThread()
{
  // Loop for update screen
  while(c_state != STATE_KILL)
  {
    // Get keypress 
    int keyCode = waitKey(1000/FRAMERATE);

    // If escape is pressed, kill thread
    if(keyCode==27)
    {
      this->setState(STATE_KILL);
    }
    // Update screen
    this->updateScreen();
  }
  // Say that thread is killed 
  this->setState(STATE_KILLED);
}


