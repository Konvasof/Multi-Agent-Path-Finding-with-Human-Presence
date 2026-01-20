#include "SharedData.h"

SharedData::SharedData() {
    is_new_info.store(false);
    is_end.store(false);
}

std::vector<LNSIterationInfo> SharedData::consume_lns_info() {
    // Zamkneme mutex, aby se data nepomíchala, kdyby běželo víc vláken
    std::lock_guard<std::mutex> lock(mutex);
    
    // Přesuneme data ven a vyčistíme frontu
    std::vector<LNSIterationInfo> result = std::move(lns_info_queue);
    lns_info_queue.clear(); 
    
    // Resetujeme flag
    is_new_info.store(false);
    
    return result;
}
