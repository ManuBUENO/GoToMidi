#ifndef DEF_PROCESSCLASS
#define DEF_PROCESSCLASS

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp" 
#include <string>

#include "configClass.hpp"
#include "guiClass.hpp"


#define ROI_RAD		5
#define ROI_MAX		255*ROI_RAD*ROI_RAD


class processClass
{
    public:

    void init(guiClass*, configClass *);
    cv::Mat preprocess(cv::Mat);
    void scanBoard(cv::Mat);
    std::vector <cv::Vec3i> getBoardChanges();
    void computeCoordinates(cv::Size);
    bool isMoving(cv::Mat,cv::Mat);

    //get
    cv::Mat getStones();
    void getCoords(std::vector <cv::Point>*);

    private:

    //pointers
    guiClass * c_ptrMainGUI;
    configClass * c_ptrMainConfig;

    //data
    std::vector <cv::Point> c_coords;
    cv::Mat c_stones;
    cv::Mat c_lastValidStones;
    int c_iterSinceValid;

};

#endif

/* 
---------------------ProcessClass.hpp
cv::Mat2i c_stones;
cv::Mat2i c_lastValidStones;
cv::Mat2i getStones();


--------------------guiClass.hpp
void setStones(cv::Mat2i); L80

--------------------guiClass.cpp
c_stones =cv::Mat2i::zeros(GO_SIZE,GO_SIZE);  L58
if(c_stones[i][j][0]==STONE_NONE){ L121
else if(c_stones[i][j][0]==STONE_BLACK){ L123    

forgot L136

void guiClass::setStones(Mat stones) L298

*/






