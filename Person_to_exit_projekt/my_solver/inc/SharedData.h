#pragma once
#include <vector>
#include <mutex>
#include <atomic>
#include "IterInfo.h"

// Použijeme struct, aby bylo vše automaticky public
struct SharedData {
    std::atomic<bool> is_new_info;
    std::atomic<bool> is_end;
    
    // Fronta pro data, kterou chceš plnit z Main.cpp
    std::vector<LNSIterationInfo> lns_info_queue;
    std::mutex mutex;

    SharedData();
    
    // Metoda, kterou používá Visualizer pro čtení
    std::vector<LNSIterationInfo> consume_lns_info();
};
