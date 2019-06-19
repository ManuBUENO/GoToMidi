#ifndef DEF_TYPESCLASS
#define DEF_TYPESCLASS

#include "opencv2/core/core.hpp"
#include <string>

// Stones identification
#define STONE_NONE      0
#define STONE_BLACK     1
#define STONE_WHITE     2


class Channel;

class Spot
{
    public:

    Spot(unsigned int);
    void reset();
    void updateState(unsigned int);
    void revertState();
    bool isChanged() const;
    void addChannel(Channel*);

    //get
    unsigned int getId() const;
    unsigned int getState() const;
    cv::Point getCoord() const;
    cv::Point2i getPos() const;
    std::vector<Channel*> getChannels() const;

    //set
    void setCoord(cv::Point2f);
    void setPos(cv::Point2i);

    private:
    
    // Spot ID 
    unsigned int c_id;
    // Coordinates (x,y)
    cv::Point c_coord;
    // position (row, col)
    cv::Point2i c_position;  
    // Valid state (empty, black, white)
    unsigned int c_state;
    // Previous state
    unsigned int c_state_old;
    // Changed
    bool c_changed;
    // Stability
    int c_stability;
    // Associated channels 
    std::vector<Channel*> c_channels ;
};

class Channel
{
    public:

    Channel(unsigned int);
    void reset();
    void updateState(unsigned int);
    bool isChanged() const;
    void addSpot(Spot *);
    

    // Get
    unsigned int getId() const;
    void getMode(char *) const;
    unsigned int getState() const;
    std::vector <unsigned char> getMsg();
    std::vector<Spot*> getSpots() const;

    // Set
    void setMode(char *);
    void setMsgOn(unsigned char *);
    void setMsgOff(unsigned char *);

    private:
    // mode
    std::string c_mode;
    // channel ID 
    unsigned int c_id;
    // Message ON
    unsigned char c_msgOn[3];
    // Message OFF
    unsigned char c_msgOff[3];
    // Spots
    std::vector<Spot*> c_spots;
    // State
    unsigned int c_state;
    // Changed
    bool c_changed;
};


#endif
