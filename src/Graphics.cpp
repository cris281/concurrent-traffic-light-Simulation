#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include "Graphics.h"
#include "Intersection.h"

Graphics::Graphics() : _headlessMode(true), _simulationDuration(60), _frameCount(0), _fps(10), _videoFilename("../data/traffic_simulation.mp4")
{
}

Graphics::~Graphics()
{
    // Release video writer resources
    if (_videoWriter.isOpened())
    {
        _videoWriter.release();
        std::cout << "Video saved to " << _videoFilename << std::endl;
    }
}

void Graphics::simulate()
{
    this->loadBackgroundImg();
    this->initializeVideoWriter();
    
    // Calculate total frames based on duration and fps
    int totalFrames = _simulationDuration * _fps;
    
    while (_frameCount < totalFrames)
    {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // update graphics
        this->drawTrafficObjects();
        
        // Increment frame counter
        _frameCount++;
        
        // Print progress every second
        if (_frameCount % _fps == 0)
        {
            std::cout << "Simulation progress: " << _frameCount / _fps << "/" << _simulationDuration << " seconds" << std::endl;
        }
    }
}

void Graphics::initializeVideoWriter()
{
    // Get image size from background
    cv::Size frameSize = _images.at(0).size();
    
    // Initialize video writer
    _videoWriter.open(_videoFilename, cv::VideoWriter::fourcc('m', 'p', '4', 'v'), _fps, frameSize);
    
    if (!_videoWriter.isOpened())
    {
        std::cerr << "Could not open video file for writing: " << _videoFilename << std::endl;
        exit(1);
    }
    
    std::cout << "Video writer initialized. Writing to: " << _videoFilename << std::endl;
}

void Graphics::loadBackgroundImg()
{
    // load image and create copy to be used for semi-transparent overlay
    cv::Mat background = cv::imread(_bgFilename);
    
    if (background.empty())
    {
        std::cerr << "Could not load background image: " << _bgFilename << std::endl;
        exit(1);
    }
    
    _images.push_back(background);         // first element is the original background
    _images.push_back(background.clone()); // second element will be the transparent overlay
    _images.push_back(background.clone()); // third element will be the result image for display
    
    // Only create window if not in headless mode
    if (!_headlessMode)
    {
        _windowName = "Concurrency Traffic Simulation";
        cv::namedWindow(_windowName, cv::WINDOW_NORMAL);
    }
}

void Graphics::drawTrafficObjects()
{
    // reset images
    _images.at(1) = _images.at(0).clone();
    _images.at(2) = _images.at(0).clone();

    // create overlay from all traffic objects
    for (auto it : _trafficObjects)
    {
        double posx, posy;
        it->getPosition(posx, posy);

        if (it->getType() == ObjectType::objectIntersection)
        {
            // cast object type from TrafficObject to Intersection
            std::shared_ptr<Intersection> intersection = std::dynamic_pointer_cast<Intersection>(it);

            // set color according to traffic light and draw the intersection as a circle
            cv::Scalar trafficLightColor = intersection->trafficLightIsGreen() == true ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255);
            cv::circle(_images.at(1), cv::Point2d(posx, posy), 25, trafficLightColor, -1);
        }
        else if (it->getType() == ObjectType::objectVehicle)
        {
            cv::RNG rng(it->getID());
            int b = rng.uniform(0, 255);
            int g = rng.uniform(0, 255);
            int r = sqrt(255*255 - g*g - b*b); // ensure that length of color vector is always 255
            cv::Scalar vehicleColor = cv::Scalar(b,g,r);
            cv::circle(_images.at(1), cv::Point2d(posx, posy), 50, vehicleColor, -1);
        }
    }

    float opacity = 0.85;
    cv::addWeighted(_images.at(1), opacity, _images.at(0), 1.0 - opacity, 0, _images.at(2));

    // Write frame to video
    _videoWriter.write(_images.at(2));
    
    // Only display if not in headless mode
    if (!_headlessMode)
    {
    cv::imshow(_windowName, _images.at(2));
    cv::waitKey(33);
    }
}
