#ifndef DEF_CONFIGCLASS
#define DEF_CONFIGCLASS

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <string>
#include <mutex>

#include "typesClass.hpp"


// GUI framerate
#define FRAMERATE       30
// Process rate
#define PROCESSRATE     10
// Midi rate
#define MIDIRATE        10

// Go board dimensions in cm. Change left, bottom, right, top edges and grid size
#define GO_EDGEL_CM     4.6
#define GO_EDGEB_CM     6.1
#define GO_EDGER_CM     5.6
#define GO_EDGET_CM     5.8
#define GO_GRID_CM      18.5

// Size of go-board /!\ to update
#define GO_SIZE         9
#define GO_N_SPOTS      GO_SIZE*GO_SIZE

// Define metronome characteristcs 
#define METRO_TIC    76
#define METRO_TOC    77
#define METRO_CHAN   9

// Window delta observed on OS X targets
#define TARGET_OSX      0
#define TARGET_OTHER    1

#if defined (__APPLE__)
#define SOFT_TARGET     TARGET_OSX 
#define SCREEN_DELTA_Y  3
#else
#define SOFT_TARGET     TARGET_OTHER 
#define SCREEN_DELTA_Y  0
#endif


// Configuration file
#define CONF_FILENAME   "config.txt"

// Mapping file
#define MAPP_FILENAME   "mapping.txt"

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

// Mapping modes
#define MAPP_MODE_DEFAULT "default"
#define MAPP_MODE_RAND  "random"
#define MAPP_MODE_SEQU  "sequencer"


// Application states
#define STATE_MAINMENU      1
#define STATE_CONFIGCOORDS  2
#define STATE_RUNNING	    3
#define STATE_SAVECONFIG    4
#define STATE_CONFIGPARAMS  5
#define STATE_STOPPING      6
#define STATE_STARTING      7
#define STATE_KILL          8
#define STATE_KILLED        9



class configClass
{
    public:

    configClass();
    ~configClass();
    void init();
    void saveConfig() const;
    void loadConfig();
    void loadMapping();


    //get
    std::vector <cv::Point2f> getCorners() const;
    std::vector <float> getParams() const;
    void getSpots(Spot**) const;
    std::vector <Channel*> getChannels() const;

    //set
    void setCorners(std::vector <cv::Point2f>);
    void clearCorners();

    private:

    std::vector<std::vector<unsigned char>> scanMsg(FILE *);

    // Go_board corners
    std::vector <cv::Point2f> c_corners;
    // Image Parameters
    std::vector <float> c_params;

    // Reference to spots
    Spot* c_spots[GO_N_SPOTS];

    // Reference to channels
    std::vector <Channel*> c_channels;

};

#endif









