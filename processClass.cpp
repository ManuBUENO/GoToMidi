#include "processClass.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/opencv.hpp" 
#include <stdio.h>

#include "configClass.hpp"
#include "guiClass.hpp"

using namespace std;
using namespace cv;

void processClass::init(guiClass* ptrMainGUI, configClass * ptrMainConfig)
{
  // init variables
  c_stones = Mat::zeros(Size(GO_SIZE,GO_SIZE),CV_8U);
  c_coords.reserve(GO_SIZE*GO_SIZE);
  c_stones.copyTo(c_lastValidStones);
  c_iterSinceValid=0;

  // init pointers
  c_ptrMainConfig = ptrMainConfig;
  c_ptrMainGUI = ptrMainGUI;

  //set first stones
  c_ptrMainGUI->setStones(c_stones);
}

Mat processClass::preprocess(Mat image)
{
    Mat fix;
    Mat imageTmp;
    image.copyTo(imageTmp);
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
      cornersSrc[i].x = float(imageTmp.cols) *cornersSrc[i].x;
      cornersSrc[i].y = float(imageTmp.rows) *cornersSrc[i].y;
    } 
    // Set goal coordinates (dst image corners)
    cornersDst.push_back(Point2f(0,0));
    cornersDst.push_back(Point2f(0,DOWN_IMG_H-1));
    cornersDst.push_back(Point2f(DOWN_IMG_W-1,DOWN_IMG_H-1));
    cornersDst.push_back(Point2f(DOWN_IMG_W-1,0));

    // Compute homography matrix
    Mat h = findHomography(cornersSrc, cornersDst);
    // Apply homography matrix to transfom image geometry
    warpPerspective(imageTmp,fix, h, Size(DOWN_IMG_W,DOWN_IMG_H));
    
    // get blue composant
    Mat fixBGR[3];
    Mat fixB;
    split(fix,fixBGR);
    fixB=fixBGR[0];
    //convert image to gray
    //cvtColor(fixBGR[0],fix,CV_BGR2GRAY);

    //Apply gaussian blur
    GaussianBlur(fixB, fixB, Size(3, 3), 0);
    // change contrast and brightness
    fixB.convertTo(fixB, -1, params[PARAM_CONTR], params[PARAM_BRIGH]);
    fixB.convertTo(fix,CV_8UC3);

    //convert back to BGR
    cvtColor(fix,fix,CV_GRAY2BGR);
    return fix;
    
}

void processClass::scanBoard(Mat boardImg)
{
  Mat img;

  // Get parameters
  vector <float> params;
  params = c_ptrMainConfig->getParams();
  // Copy and grey img
  boardImg.copyTo(img);
  cvtColor(img,img,CV_BGR2GRAY);
  
  //For each board spots
  unsigned int i,j;
  for(i=0;i<GO_SIZE;i++)
  {
    for(j=0;j<GO_SIZE;j++)
    {
      //retreive spot coordinate
      Point pt(c_coords[i+j*GO_SIZE]);
      //define rect around it
      Rect roi(pt.x-ROI_RAD,pt.y-ROI_RAD,ROI_RAD,ROI_RAD);
      //extract rect from imag
      Mat moi(boardImg,roi);
      //get rect sum
      int result = (int)cv::sum(moi)[0];
      //set result depending on sum
      if (result<(float(ROI_MAX)*params[PARAM_THRSB]))
      {
        c_stones.at<char>(i,j)=STONE_BLACK;
      }
      else if(result>=(float(ROI_MAX)*params[PARAM_THRSW]))
      {
        c_stones.at<char>(i,j)=STONE_WHITE;
      }
      else
      {
       c_stones.at<char>(i,j)=STONE_NONE;
      }
    }
  }
  //Give data to gui
  c_ptrMainGUI->setStones(c_stones);
}

vector <Vec3i> processClass::getBoardChanges()
{
  //// Compare last valid stones to new stones 
  vector <Vec3i> boardChange;
  // vector format: x?1:GO_SIZE-1 ;y?1:GO_SIZE-1; stone?0:2
  Mat stones_diff;
  compare(c_stones, c_lastValidStones, stones_diff, CMP_NE);

  //Find differences between now and then
  Mat nonZeroCoordinates;//(c_stones.size(),CV_8UC1);
  findNonZero(stones_diff,nonZeroCoordinates);

  //One stone changed
  if(nonZeroCoordinates.total()==1)
  {
    int x=nonZeroCoordinates.at<Point>(0).x;
    int y=nonZeroCoordinates.at<Point>(0).y;
    //store change data
    boardChange.push_back(Vec3i(x,y,c_stones.at<char>(y,x)));

    //validate stones
    c_stones.copyTo(c_lastValidStones);
    c_iterSinceValid=0;
    //cout<<c_stones<<endl;
   // cout<< "stone  ["<< boardChange[0][0] <<";"<< boardChange[0][1] << "] changed to " << boardChange[0][2] <<endl;
  }
  else if(nonZeroCoordinates.total()==0)
  {
    //No change but validate stones
    c_stones.copyTo(c_lastValidStones);
    c_iterSinceValid=0;
  }
  else
  {
    // Too much stones changed, not possible
    // No change, unvalidate new c_stones
    c_iterSinceValid +=1;
    //After 1 second, validate board anywway
    if(c_iterSinceValid>=FRAMERATE)
    {
      unsigned int i;
      for(i=0;i<nonZeroCoordinates.total();i++)
      {
        //store change data
        int x=nonZeroCoordinates.at<Point>(i).x;
        int y=nonZeroCoordinates.at<Point>(i).y;
        boardChange.push_back(Vec3i(x,y,c_stones.at<char>(y,x)));
       // cout<< "stone  ["<< boardChange[i][0] <<";"<< boardChange[i][1] << "] changed to " << boardChange[i][2] <<endl;
        // validate board
        c_iterSinceValid=0;
        c_stones.copyTo(c_lastValidStones);
      }
    }
  }
  return boardChange; 
}


void processClass::computeCoordinates(Size imgSize)
{
  int i,j;
  float x,y;
  // determine board edge thickness
 // Point2f edge(float(GO_EDGEX_RATIO)*imgSize.width,float(GO_EDGEY_RATIO)*imgSize.height);
  vector <float> edges;
  // compute edges
  edges.push_back(float(GO_EDGEL_RATIO*imgSize.width));
  edges.push_back(float(GO_EDGEB_RATIO*imgSize.height));
  edges.push_back(float(GO_EDGER_RATIO*imgSize.width));
  edges.push_back(float(GO_EDGET_RATIO*imgSize.height));

  //float grid = imgSize.width-2*edge.x;
  //Clear previous coordinates
  c_coords.clear();
  //For each spot
  for(i=0;i<GO_SIZE;i++)
  {
    for(j=0;j<GO_SIZE;j++)
    {
      // Compute coordinates
      x=edges[0]+(imgSize.width-edges[0]-edges[2])*i/8;
      y=edges[3]+(imgSize.height-edges[1]-edges[3])*j/8;

      c_coords.push_back(Point((int)x,(int)y));
    }
  }
  //cout << GO_EDGEL_CM<< endl;
  //cout << GO_WIDTH_CM << endl;
  //cout << GO_EDGEL_RATIO << endl;
  //cout << edges[3] << endl;
  // Give coords to GUI
  c_ptrMainGUI->setCoords(c_coords);
}


bool processClass::isMoving(Mat imageOld, Mat imageNew)
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


//GET

Mat processClass::getStones()
{
  Mat copy;
  c_stones.copyTo(copy);
  return copy;
}

void processClass::getCoords(vector <Point>* ptrCoords)
{
  ptrCoords->assign(c_coords.begin(), c_coords.end()); 
}



//SET