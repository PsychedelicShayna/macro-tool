#include "macro_tool.hh"

double KeyboardMacro::getUnixTime() {
    using namespace std::chrono;
    
    const auto time_since_epoch = system_clock::now().time_since_epoch();
    milliseconds ms_since_epoch = duration_cast<milliseconds>(time_since_epoch);
    
    return static_cast<double>(ms_since_epoch.count());
}

bool KeyboardMacro::keyIsPressed(uint16_t vkid) {
    return (GetAsyncKeyState(vkid) * 0x8000) != 0;
}

KeyboardMacro::KeyboardInput::KeyboardInput(uint16_t vkid, double delay) {
    Vkid = vkid;
    Delay = delay;
    ReleaseDelay = 100;
}

KeyboardMacro::KeyboardInput::KeyboardInput(uint16_t vkid, double delay, double release_delay) {
    Vkid = vkid;
    Delay = delay;
    ReleaseDelay = release_delay;
}

void KeyboardMacro::recorderThreadWorker(const uint16_t target_vkid) {
    runningThreads += 1;

    for(;recordingActive;Sleep(1)) {
        if(keyIsPressed(target_vkid)) {       
            uint32_t last_input_index = 0;
            double pressed_timestamp = 0;
        
            {
                std::lock_guard<std::mutex> lock_guard(macroInputsMutex);
                pressed_timestamp = getUnixTime();
                
                macroInputs.push_back(KeyboardInput(target_vkid, pressed_timestamp - lastTimeStamp));                
                last_input_index = macroInputs.size()-1;
                lastTimeStamp = pressed_timestamp;
            }
                
            for(;keyIsPressed(target_vkid);) {
                if(!recordingActive) return;
            }
     
            {
                std::lock_guard<std::mutex> lock_guard(macroInputsMutex);
                macroInputs.at(last_input_index).ReleaseDelay = getUnixTime() - pressed_timestamp;
            }
        }
    }
    
    runningThreads -= 1;
}

void KeyboardMacro::Record() { 
    if(!recordingActive) {
        macroInputs.clear();
        recordingActive = true;
        lastTimeStamp = getUnixTime();
        
        std::cout << "Starting recorder threads.. " << std::endl;
        
        for(uint16_t vkid=0; vkid<0xFF; ++vkid) {
            recorderThreads.emplace_back(std::thread([this](uint16_t target_vkid) -> void { 
                recorderThreadWorker(target_vkid);
            }, vkid));            
        }
        
        std::cout << std::endl;
    }
}

void KeyboardMacro::StopRecording() {
    if(recordingActive) {
        recordingActive = false;
    
        for(auto& thread : recorderThreads) {
            thread.join();
        }
        
        recorderThreads.clear();
    }
}

void KeyboardMacro::Play() {
    if(macroInputs.size() && !recordingActive) {
        std::vector<std::thread> release_threads;

        for(const auto& input : macroInputs) {
            INPUT input_config;
            
            input_config.type = INPUT_KEYBOARD;
            input_config.ki.wScan = MapVirtualKeyA(input.Vkid, MAPVK_VK_TO_VSC);
            input_config.ki.time = NULL;
            input_config.ki.dwExtraInfo = NULL;
            input_config.ki.dwFlags = KEYEVENTF_SCANCODE;
            input_config.ki.wVk = 0;            
        
            Sleep(input.Delay);
            SendInput(1, &input_config, sizeof(input_config));
        
            release_threads.emplace_back([](uint16_t vkid, double delay) -> void {
                INPUT input_config;
                
                input_config.type = INPUT_KEYBOARD;
                input_config.ki.wScan = MapVirtualKeyA(vkid, MAPVK_VK_TO_VSC);
                input_config.ki.time = NULL;
                input_config.ki.dwExtraInfo = NULL;
                input_config.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
                input_config.ki.wVk = 0;
            
                Sleep(delay);
                SendInput(1, &input_config, sizeof(input_config));
            }, input.Vkid, input.ReleaseDelay);
        }
    
        for(auto& thread : release_threads) {
            thread.join();
        }
    }
}

Json KeyboardMacro::DumpMacroJson() {
    Json serialized_macro;
    
    for(const auto& macro_input : macroInputs) {
        std::tuple<uint16_t, double, double>  input_tuple;
        
        std::get<0>(input_tuple) = macro_input.Vkid;
        std::get<1>(input_tuple) = macro_input.Delay;
        std::get<2>(input_tuple) = macro_input.ReleaseDelay;
    
        serialized_macro.push_back(input_tuple);
    }
    
    return serialized_macro;    
}

void KeyboardMacro::LoadMacroJson(const Json& json_macro) {
    macroInputs.clear();

    for(const auto& input : json_macro) {
        macroInputs.push_back(KeyboardInput(
            input.at(0).get<uint16_t>(),
            input.at(1).get<double>(),
            input.at(2).get<double>()
        ));
    }
}

void KeyboardMacro::DumpMacroFile(const std::string& file_path) {
    std::ofstream ofile_stream(file_path, std::ios::binary);
    
    if(ofile_stream.good()) {
        std::string serialized_macro = DumpMacroJson().dump();
        ofile_stream.write(serialized_macro.data(), serialized_macro.size());
        ofile_stream.close();
    }
}

void KeyboardMacro::LoadMacroFile(const std::string& file_path) {
    std::ifstream ifile_stream(file_path, std::ios::binary);
    
    if(ifile_stream.good()) {
        std::vector<uint8_t> file_bytes((std::istreambuf_iterator<char>(ifile_stream)), (std::istreambuf_iterator<char>()));
        ifile_stream.close();
        
        std::string file_bytes_string(file_bytes.size()+1, 0x00);
        memcpy(file_bytes_string.data(), file_bytes.data(), file_bytes.size());
        
        Json macro_json = Json::parse(file_bytes_string);
        LoadMacroJson(macro_json);
    }
}


KeyboardMacro::KeyboardMacro() {
    runningThreads = 0;
    recordingActive = false;
}