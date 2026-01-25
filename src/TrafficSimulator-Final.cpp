#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>

#include "Vehicle.h"
#include "Street.h"
#include "Intersection.h"
#include "Graphics.h"


// Paris
void createTrafficObjects_Paris(std::vector<std::shared_ptr<Street>> &streets, std::vector<std::shared_ptr<Intersection>> &intersections, std::vector<std::shared_ptr<Vehicle>> &vehicles, std::string &filename, int nVehicles)
{
    // assign filename of corresponding city map
    filename = "../data/paris.jpg";

    // init traffic objects
    int nIntersections = 9;
    for (size_t ni = 0; ni < nIntersections; ni++)
    {
        intersections.push_back(std::make_shared<Intersection>());
    }

    // position intersections in pixel coordinates (counter-clockwise)
    intersections.at(0)->setPosition(385, 270);
    intersections.at(1)->setPosition(1240, 80);
    intersections.at(2)->setPosition(1625, 75);
    intersections.at(3)->setPosition(2110, 75);
    intersections.at(4)->setPosition(2840, 175);
    intersections.at(5)->setPosition(3070, 680);
    intersections.at(6)->setPosition(2800, 1400);
    intersections.at(7)->setPosition(400, 1100);
    intersections.at(8)->setPosition(1700, 900); // central plaza

    // create streets and connect traffic objects
    int nStreets = 8;
    for (size_t ns = 0; ns < nStreets; ns++)
    {
        streets.push_back(std::make_shared<Street>());
        streets.at(ns)->setInIntersection(intersections.at(ns));
        streets.at(ns)->setOutIntersection(intersections.at(8));
    }

    // add vehicles to streets
    for (size_t nv = 0; nv < nVehicles; nv++)
    {
        vehicles.push_back(std::make_shared<Vehicle>());
        vehicles.at(nv)->setCurrentStreet(streets.at(nv));
        vehicles.at(nv)->setCurrentDestination(intersections.at(8));
    }
}

// NYC
void createTrafficObjects_NYC(std::vector<std::shared_ptr<Street>> &streets, std::vector<std::shared_ptr<Intersection>> &intersections, std::vector<std::shared_ptr<Vehicle>> &vehicles, std::string &filename, int nVehicles)
{
    // assign filename of corresponding city map
    filename = "../data/nyc.jpg";

    // init traffic objects
    int nIntersections = 6;
    for (size_t ni = 0; ni < nIntersections; ni++)
    {
        intersections.push_back(std::make_shared<Intersection>());
    }

    // position intersections in pixel coordinates
    intersections.at(0)->setPosition(1430, 625);
    intersections.at(1)->setPosition(2575, 1260);
    intersections.at(2)->setPosition(2200, 1950);
    intersections.at(3)->setPosition(1000, 1350);
    intersections.at(4)->setPosition(400, 1000);
    intersections.at(5)->setPosition(750, 250);

    // create streets and connect traffic objects
    int nStreets = 7;
    for (size_t ns = 0; ns < nStreets; ns++)
    {
        streets.push_back(std::make_shared<Street>());
    }

    streets.at(0)->setInIntersection(intersections.at(0));
    streets.at(0)->setOutIntersection(intersections.at(1));

    streets.at(1)->setInIntersection(intersections.at(1));
    streets.at(1)->setOutIntersection(intersections.at(2));

    streets.at(2)->setInIntersection(intersections.at(2));
    streets.at(2)->setOutIntersection(intersections.at(3));

    streets.at(3)->setInIntersection(intersections.at(3));
    streets.at(3)->setOutIntersection(intersections.at(4));

    streets.at(4)->setInIntersection(intersections.at(4));
    streets.at(4)->setOutIntersection(intersections.at(5));

    streets.at(5)->setInIntersection(intersections.at(5));
    streets.at(5)->setOutIntersection(intersections.at(0));

    streets.at(6)->setInIntersection(intersections.at(0));
    streets.at(6)->setOutIntersection(intersections.at(3));

    // add vehicles to streets
    for (size_t nv = 0; nv < nVehicles; nv++)
    {
        vehicles.push_back(std::make_shared<Vehicle>());
        vehicles.at(nv)->setCurrentStreet(streets.at(nv));
        vehicles.at(nv)->setCurrentDestination(intersections.at(nv));
    }
}

/* Main function */
int main(int argc, char* argv[])
{
    /* PART 1 : Set up traffic objects */

    // create and connect intersections and streets
    std::vector<std::shared_ptr<Street>> streets;
    std::vector<std::shared_ptr<Intersection>> intersections;
    std::vector<std::shared_ptr<Vehicle>> vehicles;
    std::string backgroundImg;
    int nVehicles = 6;
    
    // Default to Paris map
    std::string cityMap = "paris";
    
    // Parse command line arguments
    std::string outputVideo = "../data/traffic_simulation.mp4";
    int simulationDuration = 20; // seconds
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--city" && i + 1 < argc) {
            cityMap = argv[++i];
        } else if (arg == "--output" && i + 1 < argc) {
            outputVideo = argv[++i];
        } else if (arg == "--duration" && i + 1 < argc) {
            simulationDuration = std::stoi(argv[++i]);
        } else if (arg == "--vehicles" && i + 1 < argc) {
            nVehicles = std::stoi(argv[++i]);
        } else if (arg == "--help") {
            std::cout << "Traffic Simulation" << std::endl;
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --city <name>      City map to use (paris or nyc, default: paris)" << std::endl;
            std::cout << "  --output <file>    Output video file (default: ../data/traffic_simulation.mp4)" << std::endl;
            std::cout << "  --duration <sec>   Simulation duration in seconds (default: 15)" << std::endl;
            std::cout << "  --vehicles <num>   Number of vehicles (default: 6)" << std::endl;
            std::cout << "  --help             Show this help message" << std::endl;
            return 0;
        }
    }
    
    // Create traffic objects based on selected city
    if (cityMap == "nyc") {
        createTrafficObjects_NYC(streets, intersections, vehicles, backgroundImg, nVehicles);
    } else {
    createTrafficObjects_Paris(streets, intersections, vehicles, backgroundImg, nVehicles);
    }

    /* PART 2 : simulate traffic objects */

    // simulate intersection
    std::for_each(intersections.begin(), intersections.end(), [](std::shared_ptr<Intersection> &i) {
        i->simulate();
    });

    // simulate vehicles
    std::for_each(vehicles.begin(), vehicles.end(), [](std::shared_ptr<Vehicle> &v) {
        v->simulate();
    });

    /* PART 3 : Launch visualization */

    // add all objects into common vector
    std::vector<std::shared_ptr<TrafficObject>> trafficObjects;
    std::for_each(intersections.begin(), intersections.end(), [&trafficObjects](std::shared_ptr<Intersection> &intersection) {
        std::shared_ptr<TrafficObject> trafficObject = std::dynamic_pointer_cast<TrafficObject>(intersection);
        trafficObjects.push_back(trafficObject);
    });

    std::for_each(vehicles.begin(), vehicles.end(), [&trafficObjects](std::shared_ptr<Vehicle> &vehicles) {
        std::shared_ptr<TrafficObject> trafficObject = std::dynamic_pointer_cast<TrafficObject>(vehicles);
        trafficObjects.push_back(trafficObject);
    });

    // draw all objects in vector
    Graphics *graphics = new Graphics();
    graphics->setBgFilename(backgroundImg);
    graphics->setTrafficObjects(trafficObjects);
    graphics->setVideoFilename(outputVideo);
    graphics->setSimulationDuration(simulationDuration);
    
    std::cout << "Starting traffic simulation..." << std::endl;
    std::cout << "City: " << cityMap << std::endl;
    std::cout << "Duration: " << simulationDuration << " seconds" << std::endl;
    std::cout << "Output: " << outputVideo << std::endl;
    
    graphics->simulate();
    
    // Request all vehicle threads to stop before proceeding
    for (auto &vehicle : vehicles)
    {
        vehicle->stop();
    }
    
    std::cout << "Simulation complete!" << std::endl;
    std::cout << "Waiting for all threads to complete..." << std::endl;
    
    // Wait for all vehicle threads to complete
    for (auto& vehicle : vehicles) {
        std::cout << "Waiting for vehicle #" << vehicle->getID() << " threads..." << std::endl;
    }
    
    // Wait for all intersection threads to complete  
    for (auto& intersection : intersections) {
        std::cout << "Waiting for intersection #" << intersection->getID() << " threads..." << std::endl;
    }
    
    std::cout << "All threads completed. Cleaning up..." << std::endl;
    delete graphics;
    std::cout << "Graphics cleaned up. Exiting..." << std::endl;
    return 0;
}
