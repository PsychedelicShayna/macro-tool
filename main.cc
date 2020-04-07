#include <iostream>

#include "macro_tool.hh"

int main() {
    std::cout << "Program started." << std::endl;

    KeyboardMacro macro;

    std::atomic<bool> thread_kill;
    std::thread macro_looper;

    for(;;Sleep(10)) {
        if((GetAsyncKeyState(0x6A) & 0x8000) != 0) {
            for(;(GetAsyncKeyState(0x6A) & 0x8000) != 0;Sleep(500))
            std::cout << "Started recording.." << std::endl;
            macro.Record();
        }
        
        if((GetAsyncKeyState(0x6D) & 0x8000) != 0) {
            for(;(GetAsyncKeyState(0xBD) & 0x8000) != 0;Sleep(500));
            std::cout << "Stopped recording" << std::endl;
            macro.StopRecording();           
        }
        
        if((GetAsyncKeyState(0x6B) & 0x8000) != 0) {
            for(;(GetAsyncKeyState(0x6B) & 0x8000) != 0;Sleep(500));
            
            std::cout << "Playing macro" << std::endl;
            macro.Play();
            std::cout << "Stoped playing" << std::endl;
        }
        
        if((GetAsyncKeyState(0x6F) & 0x8000) != 0) {
            for(;(GetAsyncKeyState(0x6F) & 0x8000) != 0;Sleep(500));
            std::cout << "Saved macro" << std::endl;
            macro.DumpMacroFile("./new.macro");
        }
    }
    
    return 0;
}

