#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <string>
#include <vector>
#include <memory>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include "TrafficObject.h"

class Graphics
{
public:
    // constructor / desctructor
    Graphics();
    ~Graphics();

    // getters / setters
    void setBgFilename(std::string filename) { _bgFilename = filename; }
    void setTrafficObjects(std::vector<std::shared_ptr<TrafficObject>> &trafficObjects) { _trafficObjects = trafficObjects; };
    void setVideoFilename(std::string filename) { _videoFilename = filename; }
    void setSimulationDuration(int seconds) { _simulationDuration = seconds; }

    // typical behaviour methods
    void simulate();

private:
    // typical behaviour methods
    void loadBackgroundImg();
    void drawTrafficObjects();
    void initializeVideoWriter();

    // member variables
    std::vector<std::shared_ptr<TrafficObject>> _trafficObjects;
    std::string _bgFilename;
    std::string _windowName;
    std::string _videoFilename;
    std::vector<cv::Mat> _images;
    cv::VideoWriter _videoWriter;
    bool _headlessMode;
    int _simulationDuration; // in seconds
    int _frameCount;
    int _fps;
};

#endif