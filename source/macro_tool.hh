#ifndef MACRO_TOOL_HH 
#define MACRO_TOOL_HH

// Windows library
#include <Windows.h>
#define WINVER 0x5000

// Function objects
#include <functional>

// Stream objects
#include <iostream>
#include <fstream>

// Int conventions
#include <cstdint>

// Containers
#include <string>
#include <vector>
#include <tuple>
#include <map>

// Multithreading
#include <thread>
#include <atomic>
#include <mutex>

// Chronometer
#include <chrono>
#include <ctime>

#include "json.hpp"
using Json = nlohmann::json;

double GetUnixTime();

class KeyboardMacro {
protected:
    bool keyIsPressed(uint16_t vkid); // Uses GetAsyncKeyState() to determine if a key is currently being pressed.
    double getUnixTime(); // Gets the Unix timestamp in milliseconds.

    struct KeyboardInput {
        uint16_t Vkid;          // The VKID of the pressed key.
        double Delay;           // The delay to wait until the key should be pressed.
        double ReleaseDelay;    // The delay to wait until the key should be released.
        
        KeyboardInput(uint16_t, double);
        KeyboardInput(uint16_t, double, double);
    };
    
    std::mutex macroInputsMutex; // Mutex to prevent data races when multiple threads append to macroInputs.
    std::vector<KeyboardInput> macroInputs; // Vector of recorded keyboard inputs.

    std::atomic<double> lastTimeStamp; // The last press down input recorded.
    std::atomic<bool> recordingActive; // A thread kill-signal to stop recording when false.
    
    std::atomic<uint32_t> runningThreads; // The number of running recorder threads.
    void recorderThreadWorker(const uint16_t vkid); // The function used to record and store inputs for one key.
    std::vector<std::thread> recorderThreads; // A vector of recorder threads, recording one key per thread.

public:
    const std::vector<KeyboardInput>& MacroInputs = macroInputs;

    void Record();          // Record into macroInputs.
    void StopRecording();   // Stop recording into macroInputs.
    void Play();            // Play the macro recorded in macroInputs.
    
    Json DumpMacroJson();               // Return the recorded macro as a Json object.
    void LoadMacroJson(const Json&);    // Load a Json object as a macro into macroInputs.
    
    void DumpMacroFile(const std::string& file_path);   // Dumps the recorded macro into a file as Json.
    void LoadMacroFile(const std::string& file_path);   // Loads a macro from a file as json into macroInputs.

    void DumpMacroStruct(const std::string& file_path);
    
    KeyboardMacro();
};

#endif // MACRO_TOOL_HH