#include <iostream>
#include "Vehicle.h"
#include "Intersection.h"
#include "Street.h"


Street::Street()
{
    _type = ObjectType::objectStreet;
    _length = 1000.0; // in m
}

void Street::setInIntersection(std::shared_ptr<Intersection> in)
{
    _interIn = in;
    try
    {
        std::shared_ptr<Street> self = get_shared_this();
        std::cout << "[LOG] Street #" << _id << " setInIntersection: use_count=" << self.use_count() << std::endl;
        in->addStreet(self); // add this street to list of streets connected to the intersection
    }
    catch (const std::bad_weak_ptr &e)
    {
        std::cerr << "[ERROR] bad_weak_ptr in Street::setInIntersection for Street #" << _id << ": " << e.what() << std::endl;
        throw; // rethrow to preserve original behaviour
    }
}

void Street::setOutIntersection(std::shared_ptr<Intersection> out)
{
    _interOut = out;
    try
    {
        std::shared_ptr<Street> self = get_shared_this();
        std::cout << "[LOG] Street #" << _id << " setOutIntersection: use_count=" << self.use_count() << std::endl;
        out->addStreet(self); // add this street to list of streets connected to the intersection
    }
    catch (const std::bad_weak_ptr &e)
    {
        std::cerr << "[ERROR] bad_weak_ptr in Street::setOutIntersection for Street #" << _id << ": " << e.what() << std::endl;
        throw;
    }
}
