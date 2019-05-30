#ifndef DEF_GUICLASS
#define DEF_GUICLASS

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp" 
#include <string>

#include "configClass.hpp"

#define WNDW_W			1100
#define WNDW_H			650

#define TOP_IMG_X		0
#define TOP_IMG_Y		0
#define TOP_IMG_W		WNDW_W*2/3
#define TOP_IMG_H		WNDW_H*1/2

#define DOWN_IMG_X		WNDW_W*2/3-(TOP_IMG_W*3/4)
#define DOWN_IMG_Y		WNDW_H*1/2
#define DOWN_IMG_W		WNDW_H*1/2
#define DOWN_IMG_H		WNDW_H*1/2

#define TEXT_IMG_X		WNDW_W*2/3+10
#define TEXT_IMG_Y		200
#define TEXT_IMG_W		WNDW_W*1/3-20
#define TEXT_IMG_H		WNDW_H-TEXT_IMG_Y-10

#define TEXT_START_X	5
#define TEXT_START_Y	10
#define TEXT_GAP_X		7
#define TEXT_GAP_Y		10

#define BTN1_IMG_X		WNDW_W*2/3+10
#define BTN1_IMG_Y		20
#define BTN1_IMG_W		WNDW_W*1/3-20
#define BTN1_IMG_H		30

#define BTN2_IMG_X		WNDW_W*2/3+10
#define BTN2_IMG_Y		70
#define BTN2_IMG_W		WNDW_W*1/3-20
#define BTN2_IMG_H		30

#define BTN3_IMG_X      WNDW_W*2/3+10
#define BTN3_IMG_Y      120
#define BTN3_IMG_W      WNDW_W*1/3-20
#define BTN3_IMG_H      30

#define BTN1_ID         1
#define BTN2_ID         2
#define BTN3_ID         3


class guiClass
{
    public:

    //guiClass();
    void init(std::string,configClass*);
    void updateTopImg(cv::Mat);
    void updateDownImg(cv::Mat);
    void clearImgs();
    void writeTxt(std::string, bool);
    void clearTxt();
    void toogleButton(int);
    bool checkWindow();

    //get
    std::vector <cv::Point2f> getCorners();
    int getState();

    //set
    void setCoords(std::vector <cv::Point>);
    void setCorners(std::vector <cv::Point2f>);
    void setState(int);
    void setStones(cv::Mat);

    private:

    void updateScreen();

    //names
    std::string c_windowName;
    
    //ptr config
    configClass * c_ptrConfig;

    // Images
    cv::Mat c_imgFull;
    cv::Mat c_imgTop;
    cv::Mat c_imgDown;

    // text zone
    cv::Mat c_imgDispText;
    std::string c_dispText;
    cv::Point c_textPoint;

    //buttons
    //cv::Rect c_recyBtn_1;
    cv::Mat c_imgBtn1;
    bool c_stateBtn1;
    
    //cv::Rect c_recyBtn_2;
    cv::Mat c_imgBtn2;
    bool c_stateBtn2;

    //cv::Rect c_recyBtn_3;
    cv::Mat c_imgBtn3;
    bool c_stateBtn3;

    //State
    int c_state;

    // data
   // std::vector <cv::Point2f> c_corners;
    std::vector <cv::Point> c_coords;
    cv::Mat c_stones;
 

};

#endif

