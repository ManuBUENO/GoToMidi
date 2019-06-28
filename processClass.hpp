#ifndef DEF_PROCESSCLASS
#define DEF_PROCESSCLASS

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp" 
#include <string>

#include "configClass.hpp"
#include "guiClass.hpp"
#include "typesClass.hpp"


#define ROI_RAD		7
#define ROI_MAX		255*ROI_RAD*ROI_RAD


class processClass
{
    public:

    processClass(configClass *);
    void init();
    void start();
    void stop();
    void reset();
    cv::Mat preprocess(const cv::Mat&) const;
    void computeCoordinates(cv::Size);
    void scanBoard(const cv::Mat&);
    void checkBoardChanges();
    void computeChannelStates();
    bool isMoving(const cv::Mat&,const cv::Mat&) const;
    void channelsOff();
    
    //get

    private:

    //// Pointers to classes
    configClass * c_ptrMainConfig;

    //// Data
    // Iteration since valid spots state
    int c_iterSinceValid;

    // List of spots
    Spot* c_spots[GO_SIZE*GO_SIZE];

    // List of channels
    std::vector <Channel *> c_channels;

};

#endif





