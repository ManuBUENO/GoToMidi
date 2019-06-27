#ifndef DEF_GUICLASS
#define DEF_GUICLASS

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp" 
#include <string>
#include <thread>
#include <mutex>

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

    guiClass(configClass*);
    void init(const std::string&);//,configClass*);
    void updateTopImg(const cv::Mat&);
    void updateDownImg(const cv::Mat&);
    void clearImgs();
    void writeTxt(std::string, bool);
    void clearTxt();
    void toogleButton(int);

    //get
    const std::vector <cv::Point2f> getCorners() const;
    int getState() const;

    //set
    void setCorners(std::vector <cv::Point2f>);
    void setState(int);

    private:

    void updateScreen() const;
    void guiThread();

    //names
    std::string c_windowName;
    
    //ptr config
    configClass *c_ptrMainConfig;

    // GUI Thread
    std::thread c_thread;

    // Images
    cv::Mat c_imgFull;
    cv::Mat c_imgTop;
    cv::Mat c_imgDown;

    // text zone
    cv::Mat c_imgDispText;
    std::string c_dispText;
    cv::Point c_textPoint;

    //buttons
    cv::Mat c_imgBtn1;
    bool c_stateBtn1;
    
    cv::Mat c_imgBtn2;
    bool c_stateBtn2;

    cv::Mat c_imgBtn3;
    bool c_stateBtn3;

    // Main program state
    int c_state;
    std::mutex c_stateMut;

    // Spots
    Spot* c_spots[GO_SIZE*GO_SIZE];
};



#endif

