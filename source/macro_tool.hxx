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
#include <sstream>

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

class KeyboardMacro {
protected:
    bool keyIsPressed(uint8_t vkid); // Uses GetAsyncKeyState() to determine if a key is currently being pressed.
    uint64_t getUnixTime(); // Gets the Unix timestamp in milliseconds.

    struct KeyboardInput {
        uint8_t  Vkid;            // The VKID of the pressed key.
        uint32_t Delay;           // The delay to wait until the key should be pressed.
        uint32_t ReleaseDelay;    // The delay to wait until the key should be released.
        
        KeyboardInput(uint8_t, uint32_t);
        KeyboardInput(uint8_t, uint32_t, uint32_t);
    };
    
    std::mutex macroInputsMutex; // Mutex to prevent data races when multiple threads append to macroInputs.
    std::vector<KeyboardInput> macroInputs; // Vector of recorded keyboard inputs.

    std::atomic<double> lastTimeStamp; // The last press down input recorded.
    std::atomic<bool> recordingActive; // A thread kill-signal to stop recording when false.
    
    std::atomic<uint32_t> runningThreads; // The number of running recorder threads.
    void recorderThreadWorker(const uint8_t vkid); // The function used to record and store inputs for one key.
    std::vector<std::thread> recorderThreads; // A vector of recorder threads, recording one key per thread.

public:
    const std::vector<KeyboardInput>& MacroInputs = macroInputs;

    void Record();          // Record into macroInputs.
    void StopRecording();   // Stop recording into macroInputs.
    void Play();            // Play the macro recorded in macroInputs.

    std::string DumpMacroString(bool hex_vkid=false);              // Dumps the recorded macro as a Json string.
    void LoadMacroString(std::string, bool hex_vkid=false);   // Loads a macro from a Json string.
    
    Json DumpMacroJson(bool hex_vkid=false);               // Return the recorded macro as a Json object.
    void LoadMacroJson(const Json&, bool hex_vkid);    // Load a Json object as a macro into macroInputs.
    
    void DumpMacroFile(const std::string& file_path, bool hex_vkid=false);   // Dumps the recorded macro into a file as Json.
    void LoadMacroFile(const std::string& file_path, bool hex_vkid=false);   // Loads a macro from a file as json into macroInputs.
    
    KeyboardMacro();
};

#endif // MACRO_TOOL_HH