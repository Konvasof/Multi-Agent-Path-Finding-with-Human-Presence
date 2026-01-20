#pragma once
#include "Instance.h"
#include "SharedData.h"
#include <atomic>
#include <thread>

// Zjednodušená třída Computation jen pro potřeby Vizualizéru
class Computation {
public:
    // Jednoduchý konstruktor bez LNS_settings
    Computation(const Instance& instance, SharedData& shared_data);
    ~Computation();

    void start();
    void stop();
    void join();
    void run(); // Hlavní smyčka (zde bude prázdná)

    std::atomic<bool> running;
    std::thread t;
    const Instance& instance;
    SharedData& shared_data;
};
