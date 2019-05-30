#ifndef DEF_CONFIGCLASS
#define DEF_CONFIGCLASS

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <string>

    #if defined (__APPLE__)
    #define SCREEN_DELTA_Y	3
    #else
    #define SCREEN_DELTA_Y	0
    #endif


// Camera index 
#define CAMERA_INDEX    1

// Go board dimensions in cm. Change left, bottom, right, top edges and grid size
#define GO_EDGEL_CM     4.6
#define GO_EDGEB_CM     6.1
#define GO_EDGER_CM     5.6
#define GO_EDGET_CM     5.8

#define GO_GRID_CM      18.5

// Size of go-board
#define GO_SIZE         9

// Application framerate. Can be changed to adapt CPU load
#define FRAMERATE       30

// Configuration file
#define CONF_FILENAME   "config.txt"

#define GO_WIDTH_CM     (GO_EDGEL_CM+GO_GRID_CM+GO_EDGER_CM)
#define GO_HEIGHT_CM    (GO_EDGET_CM+GO_GRID_CM+GO_EDGEB_CM)

#define GO_EDGEL_RATIO  GO_EDGEL_CM/GO_WIDTH_CM
#define GO_EDGEB_RATIO  GO_EDGEB_CM/GO_HEIGHT_CM
#define GO_EDGER_RATIO  GO_EDGER_CM/GO_WIDTH_CM
#define GO_EDGET_RATIO  GO_EDGET_CM/GO_HEIGHT_CM


// Indexes for parameters frm Config file  
#define PARAM_CONTR		0
#define PARAM_BRIGH		1
#define PARAM_THRSB		2
#define PARAM_THRSW		3
//Number of parameters frm Config file
#define PARAM_N 		4

// Stones identification
#define STONE_NONE      0
#define STONE_BLACK     1
#define STONE_WHITE     2

// Application states
#define STATE_MAINMENU      1
#define STATE_CONFIGCOORDS  2
#define STATE_RUNNING	    3
#define STATE_SAVECONFIG    4
#define STATE_CONFIGPARAMS  5

class configClass
{
    public:
    void init();
    void saveConfig();
    void loadConfig();

    //get
    std::vector <cv::Point2f> getCorners();
    std::vector <float> getParams();

    //set
    void setCorners(std::vector <cv::Point2f>);
    void clearCorners();

    private:
    std::vector <cv::Point2f> c_corners;
    std::vector <float> c_params;

};

#endif









