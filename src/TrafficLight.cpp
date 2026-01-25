#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include "TrafficLight.h"


// ============================================================================
// TrafficLight Constructor
// ============================================================================
// Initialize traffic light to red for safety
// A newly installed traffic light should be red by default
// ============================================================================
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
    _type = ObjectType::objectIntersection;  // Reuse intersection type for rendering
}

// ============================================================================
// getCurrentPhase
// ============================================================================
// Simple getter for the current phase
// Note: No locking needed for reading an enum (atomic on most architectures)
// ============================================================================
TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}


// ============================================================================
// waitForGreen
// ============================================================================
// Blocks until the traffic light turns green.
//
// How it works:
// 1. Infinite loop calls receive() on the message queue
// 2. receive() blocks until a new phase is available
// 3. When green phase is received, return immediately
// 4. If red phase is received, continue waiting
//
// Why an infinite loop?
// - The traffic light may toggle red→green→red→green...
// - We need to wait until we specifically get a green
// - receive() handles the efficient waiting (no busy-waiting)
// ============================================================================
void TrafficLight::waitForGreen()
{
    while (true)
    {
        // Block until next phase change is received
        // receive() uses condition_variable - no CPU wasted
        TrafficLightPhase phase = _messageQueue.receive();
        
        // Check if the received phase is green
        if (phase == TrafficLightPhase::green)
        {
            return;  // Green light - allow the vehicle to proceed
        }
        
        // Red light received - continue waiting for green
        // Loop back and call receive() again
    }
}

// ============================================================================
// simulate
// ============================================================================
// Starts the traffic light's main loop in a new thread.
//
// Key Points:
// 1. We use emplace_back to construct the thread in-place
// 2. The thread is stored in the 'threads' vector (inherited from TrafficObject)
// 3. TrafficObject's destructor will join this thread automatically
// 4. &TrafficLight::cycleThroughPhases is a pointer to member function
// 5. 'this' is required because member functions need an object to operate on
// ============================================================================
void TrafficLight::simulate()
{
    // Start the cycleThroughPhases method in a new thread
    // Store the thread in the base class's threads vector for lifecycle management
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// ============================================================================
// cycleThroughPhases
// ============================================================================
// Main simulation loop that toggles the traffic light between red and green.
//
// Algorithm:
// 1. Initialize random cycle duration between 4-6 seconds
// 2. Start infinite loop
// 3. Sleep 1ms per iteration (reduce CPU usage)
// 4. Measure time since last phase change
// 5. When duration exceeded:
//    a. Toggle the phase (red↔green)
//    b. Send new phase to message queue
//    c. Reset timer and get new random duration
//
// Why random duration?
// - Simulates real traffic light timing variability
// - Makes the simulation more interesting/realistic
// - Tests that our synchronization works with varying timings
// ============================================================================
void TrafficLight::cycleThroughPhases()
{
    // ========================================================================
    // Step 1: Initialize Random Number Generator
    // ========================================================================
    // std::random_device provides a seed from hardware entropy
    // std::mt19937 is the Mersenne Twister algorithm - high quality randomness
    // uniform_int_distribution gives us values uniformly between 4000 and 6000 ms
    // ========================================================================
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<int> distribution(4000, 6000);  // 4-6 seconds in ms
    
    // Get initial random cycle duration
    int cycleDuration = distribution(eng);
    
    // ========================================================================
    // Step 2: Initialize Stopwatch
    // ========================================================================
    // We use system_clock for measuring elapsed time
    // time_point represents a moment in time
    // ========================================================================
    std::chrono::time_point<std::chrono::system_clock> lastUpdate;
    lastUpdate = std::chrono::system_clock::now();
    
    // ========================================================================
    // Step 3: Main Simulation Loop
    // ========================================================================
    while (true)
    {
        // --------------------------------------------------------------------
        // Sleep to reduce CPU usage
        // --------------------------------------------------------------------
        // Without this sleep, the while loop would consume 100% of one CPU core
        // 1ms is short enough to be responsive, long enough to be efficient
        // --------------------------------------------------------------------
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        // --------------------------------------------------------------------
        // Calculate time since last phase change
        // --------------------------------------------------------------------
        // now() - lastUpdate gives us a duration object
        // duration_cast converts to milliseconds
        // .count() extracts the raw number
        // --------------------------------------------------------------------
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - lastUpdate
        ).count();
        
        // --------------------------------------------------------------------
        // Check if it's time to toggle the phase
        // --------------------------------------------------------------------
        if (timeSinceLastUpdate >= cycleDuration)
        {
            // ----------------------------------------------------------------
            // Toggle the phase
            // ----------------------------------------------------------------
            // If currently red → switch to green
            // If currently green → switch to red
            // ----------------------------------------------------------------
            if (_currentPhase == TrafficLightPhase::red)
            {
                _currentPhase = TrafficLightPhase::green;
            }
            else
            {
                _currentPhase = TrafficLightPhase::red;
            }
            
            // ----------------------------------------------------------------
            // Send the new phase to the message queue
            // ----------------------------------------------------------------
            // std::move converts _currentPhase to an rvalue reference
            // This allows the send() method to use move semantics
            // For an enum, this doesn't save much, but it's the correct pattern
            // ----------------------------------------------------------------
            _messageQueue.send(std::move(_currentPhase));
            
            // ----------------------------------------------------------------
            // Reset for next cycle
            // ----------------------------------------------------------------
            // Update the stopwatch to current time
            lastUpdate = std::chrono::system_clock::now();
            
            // Get a new random duration for the next cycle
            cycleDuration = distribution(eng);
        }
    }
}