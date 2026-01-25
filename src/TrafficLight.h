#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include "TrafficObject.h"

// Forward declaration to avoid circular dependency
class Vehicle;

// ============================================================================
// TrafficLightPhase Enum
// ============================================================================
// Represents the two possible states of a traffic light.
// Using enum class for type safety (prevents implicit conversions to int).
// ============================================================================
enum class TrafficLightPhase
{
    red,
    green
};

// ============================================================================
// MessageQueue Class Template
// ============================================================================
// A thread-safe FIFO queue for communication between threads.
// 
// Design Pattern: Producer-Consumer
// - Producer (cycleThroughPhases): calls send() to add phases to queue
// - Consumer (waitForGreen): calls receive() to get phases from queue
//
// Thread Safety:
// - All operations protected by mutex
// - condition_variable for efficient waiting (no busy-waiting/polling)
// ============================================================================
template <typename T>
class MessageQueue
{
public:
    // ========================================================================
    // send - Add a message to the queue (Producer side)
    // ========================================================================
    // Parameters:
    //   msg: rvalue reference - takes ownership via move semantics
    //        This is efficient because we don't copy the message
    // 
    // Thread Safety:
    //   Uses lock_guard for RAII-style mutex management
    //   Notifies one waiting consumer after adding message
    // ========================================================================
    void send(T &&msg)
    {
        // RAII lock - automatically releases when going out of scope
        // Even if an exception is thrown, the lock will be released
        std::lock_guard<std::mutex> lock(_mutex);
        
        // Add message to the back of the queue using move semantics
        // std::move transfers ownership, avoiding expensive copies
        _queue.push_back(std::move(msg));
        
        // Wake up ONE waiting thread (if any)
        // We use notify_one() because only one consumer should process each message
        _condition.notify_one();
    }
    
    // ========================================================================
    // receive - Wait for and retrieve a message from the queue (Consumer side)
    // ========================================================================
    // Returns:
    //   The oldest message in the queue (FIFO order)
    // 
    // Behavior:
    //   BLOCKS if queue is empty until a message is available
    //   This is efficient - no CPU cycles wasted on polling
    //
    // Thread Safety:
    //   Uses unique_lock (required by condition_variable::wait)
    //   The wait() automatically unlocks during sleep, relocks on wake
    // ========================================================================
    T receive()
    {
        // unique_lock is required for condition_variable::wait()
        // Unlike lock_guard, unique_lock can be unlocked/relocked manually
        // wait() needs this ability to release the lock while waiting
        std::unique_lock<std::mutex> lock(_mutex);
        
        // Wait until the queue has at least one element
        // The lambda predicate prevents spurious wakeups:
        // - Thread might wake up without notify being called
        // - Lambda ensures we only proceed when condition is truly met
        _condition.wait(lock, [this]() { 
            return !_queue.empty(); 
        });
        
        // Move the front element out of the queue
        // std::move transfers ownership, front() returns reference
        T msg = std::move(_queue.front());
        
        // Remove the (now empty) front element
        _queue.pop_front();
        
        return msg;  // Return by value - move semantics apply
    }

private:
    // The underlying container - deque provides O(1) push_back and pop_front
    std::deque<T> _queue;
    
    // Protects all access to _queue
    std::mutex _mutex;
    
    // Allows efficient blocking until messages are available
    // Better than busy-waiting (polling in a loop)
    std::condition_variable _condition;
};


// FP.1 : Define a class „TrafficLight“ which is a child class of TrafficObject.
// The class shall have the public methods „void waitForGreen()“ and „void simulate()“ 
// as well as „TrafficLightPhase getCurrentPhase()“, where TrafficLightPhase is an enum that 
// can be either „red“ or „green“. Also, add the private method „void cycleThroughPhases()“. 
// Furthermore, there shall be the private member _currentPhase which can take „red“ or „green“ as its value. 

// ============================================================================
// TrafficLight Class
// ============================================================================
// Represents a traffic light at an intersection.
// 
// Inheritance:
//   Inherits from TrafficObject to use the common thread management
//   (threads vector, automatic joining in destructor)
//
// Threading Model:
//   - cycleThroughPhases() runs in its own thread (started by simulate())
//   - Continuously toggles between red and green
//   - Sends new phases to _messageQueue
//   - Other threads call waitForGreen() to block until green
// ============================================================================
class TrafficLight : public TrafficObject
{
public:
    // ========================================================================
    // Constructor
    // ========================================================================
    // Initializes the traffic light to red (safe default)
    // ========================================================================
    TrafficLight();
    
    // ========================================================================
    // Getters
    // ========================================================================
    
    // Returns the current phase of the traffic light
    // Used by Intersection to check if entry is allowed
    TrafficLightPhase getCurrentPhase();
    
    // ========================================================================
    // Simulation Control
    // ========================================================================
    
    // Starts the traffic light simulation in a new thread
    // Override of virtual method from TrafficObject
    void simulate() override;
    
    // ========================================================================
    // Synchronization Methods
    // ========================================================================
    
    // Blocks the calling thread until the traffic light turns green
    // This is the main interface for vehicles waiting at the light
    void waitForGreen();

private:
    // ========================================================================
    // Private Methods
    // ========================================================================
    
    // The main simulation loop (runs in its own thread)
    // Toggles between red and green phases at random intervals
    void cycleThroughPhases();
    
    // ========================================================================
    // Private Members
    // ========================================================================
    
    // Current state of the traffic light
    TrafficLightPhase _currentPhase;
    
    // Message queue for communicating phase changes
    // Producer: cycleThroughPhases() calls send()
    // Consumer: waitForGreen() calls receive()
    MessageQueue<TrafficLightPhase> _messageQueue;
    
    // Note: _condition and _mutex from original template are not needed
    // because MessageQueue handles its own synchronization
};

#endif  // TRAFFICLIGHT_H