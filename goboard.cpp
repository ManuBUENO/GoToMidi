#include <iostream>
#include <string>

//#include <videoio.hpp>

#include "RtMidi.h"
#include "guiClass.hpp"
#include "configClass.hpp"
#include "processClass.hpp"
#include "midiClass.hpp"


using namespace cv;
using namespace std;

/* NOTES
- Board edges thinckness defines in "configClass.hpp"
- NOTEZERO from mapping.txt
*/

void onMouse(int event, int x, int y, int flag, void* ptrGUI)
{
    /* Debug: print coordinates
    if( event == EVENT_MOUSEMOVE )
        cout<< "x= " << x <<" y= " << y << endl;
    */

    // If event is not a left button click, leave function  
    if( event != CV_EVENT_LBUTTONDOWN )
            return;

    // Add delta for OS X windows
    y+=SCREEN_DELTA_Y;
    if(y>WNDW_H)
	    return;

    // Cast gui pointer
    guiClass* ptrMainGUI = (guiClass *) ptrGUI;

    // Actions depends on application state
    switch(ptrMainGUI->getState())
    {
        case STATE_MAINMENU:
            {  
            // define buttons rectangles
            Rect rectBtn1(BTN1_IMG_X,BTN1_IMG_Y,BTN1_IMG_W,BTN1_IMG_H);
            Rect rectBtn2(BTN2_IMG_X,BTN2_IMG_Y,BTN2_IMG_W,BTN2_IMG_H);
            Rect rectBtn3(BTN3_IMG_X,BTN3_IMG_Y,BTN3_IMG_W,BTN3_IMG_H);
            
            //Check if the click is on one button
            if (rectBtn1.contains(Point(x,y))){
                ptrMainGUI->toogleButton(BTN1_ID);
            }
            else if (rectBtn2.contains(Point(x,y))){
                ptrMainGUI->toogleButton(BTN2_ID);
            }
            else if (rectBtn3.contains(Point(x,y))){
                ptrMainGUI->toogleButton(BTN3_ID);
            }
            break;
        }

        case STATE_CONFIGCOORDS:
        {
            float xf = float(x)/float(TOP_IMG_W);
            float yf = float(y)/float(TOP_IMG_H+DOWN_IMG_W);

            // retreive corners
            // nota: Unable to retreive corners directly from configClass.
            //       So we get them from configClass THROUGH guiClass
            vector <Point2f> corners = ptrMainGUI->getCorners();
            // add new point
            corners.push_back(Point2f(xf,yf));
            // update corners
            ptrMainGUI->setCorners(corners);
            // print point
            ostringstream ss;
            ss << "Point "<< corners.size() << " set";

            // write Text on GUI
            ptrMainGUI->writeTxt(ss.str(),true);

            // when all corners are defined, leave get coordinates state
            // by virtually pushing button 1 
            if (corners.size()>=4)
            {
                ptrMainGUI->toogleButton(BTN1_ID);
            }
            break;
        }
        case STATE_RUNNING:
        {
            //Check if the click is on button 3
            Rect rectBtn3(BTN3_IMG_X,BTN3_IMG_Y,BTN3_IMG_W,BTN3_IMG_H);
            if (rectBtn3.contains(Point(x,y))){
                ptrMainGUI->toogleButton(BTN3_ID);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

int main(int argc, char** argv)
{
	//Get camera index
    int cameraIndex = argc >= 2 ? stoi(argv[1]) : 1 ;

    // Init classes
    configClass mainConfig;
    guiClass mainGUI;
    processClass mainProcess;
    midiClass mainMidi;

    // Init Imagesc
    Mat imageSrc;
    Mat imageOld;
    Mat imageDst;

    // Init Camera
    // Open camera 1
    VideoCapture cam(cameraIndex);
    if(!cam.isOpened())
    {
        // check if we succeeded
        cout << "Unable to access camera named \"video"<<cameraIndex<<"\""<<endl;
        cout << "Camera index can be specified as argument. Try \"./go_board_reader 0\"" <<endl;
         
        return -1;   
    }  

    //// Init config
    mainConfig.init();

    //// Init Midi
    if(!mainMidi.init())  // check if we succeeded
        return -1;

    //// Get first image
    cam>>imageSrc;

    //// Init GUI
    mainGUI.init("Go-board reader V1.0",&mainConfig);
    
    //// Init process 
    mainProcess.init(&mainGUI,&mainConfig);

    //// First preprocess;
    // treat image
    imageDst = mainProcess.preprocess(imageSrc);
    // compute stones coordinate and share data to GUI
    mainProcess.computeCoordinates(imageDst.size());

    //// Init mouse callback
    setMouseCallback("Go-board reader V1.0", onMouse, (void*) &mainGUI);


    while(waitKey(1000/FRAMERATE)<0)
    {
        // store previous image in imageOld
        imageDst.copyTo(imageOld);
        // get new image from camera
        cam>>imageSrc;

        // State manager
        switch(mainGUI.getState())
        {
            case STATE_MAINMENU:
            {
                //// Update top image with coordinates
                mainGUI.updateTopImg(imageSrc);
                //// Treat image
                imageDst = mainProcess.preprocess(imageSrc);
                //// search stones, send data to GUI
                mainProcess.scanBoard(imageDst);
                //// Update bottom image with stones coordinates
                mainGUI.updateDownImg(imageDst);
                break;
            }
            case STATE_CONFIGCOORDS:
            {
                //// Update top image
                mainGUI.updateTopImg(imageSrc);
                break;
            }
            case STATE_CONFIGPARAMS:
            {
                // Load parameters
                mainConfig.loadConfig();
                mainGUI.writeTxt("Config loaded",true);
                // virtually push button 2 to leave config
                mainGUI.toogleButton(BTN2_ID);
                break;
            }
            case STATE_SAVECONFIG:
            {
                // Save config in CONF_FILENAME
                mainConfig.saveConfig();
                // Compute spots coordinates after updating coordinates
                mainProcess.computeCoordinates(imageDst.size());
                // Set application state to main menu 
                mainGUI.setState(STATE_MAINMENU);
                // Write something  
                break;
            }
            case STATE_RUNNING:
            {
                //// Treat image
                imageDst = mainProcess.preprocess(imageSrc);                
                //// if motion is detected, do nothing
                if(!mainProcess.isMoving(imageOld,imageDst))
                {
                    // Show top image
                    mainGUI.updateTopImg(imageSrc);
                    // Search stones, send data to GUI
                    mainProcess.scanBoard(imageDst);
                    // Get changes on board
                    mainProcess.getBoardChanges();
                    // Compute channel states
                    mainProcess.computeChannelStates();
                    // Get notes cahnged depending on mapping
                    vector <Vec2i> Notes = mainProcess.getChannelChanges();
                    // Send messages to midi
                    if(!mainMidi.updateChannels(Notes))
                        return -1;
                    //// Update bottom image with stones coordinates
                    mainGUI.updateDownImg(imageDst);
                }
                break;
            }
            case STATE_STOPPING:
            {
                // Channels Off, update midi
                mainProcess.channelsOff();
                vector <Vec2i> Notes = mainProcess.getChannelChanges();
                if(!mainMidi.updateChannels(Notes))
                        return -1;

                // Reset process
                mainProcess.reset();
                // Go back to main menu    
                mainGUI.setState(STATE_MAINMENU);
            }
            default:
                break;
        }
    }
    //Both channelsOff() needs to be called 
    mainProcess.channelsOff();
    vector <Vec2i> Notes = mainProcess.getChannelChanges();
    if(!mainMidi.updateChannels(Notes))
            return -1;
}
