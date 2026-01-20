#include "Computation.h"

// Implementace konstruktoru
Computation::Computation(const Instance& inst, SharedData& sd) 
    : instance(inst), shared_data(sd), running(false) 
{
}

Computation::~Computation() {
    stop();
    join();
}

void Computation::start() {
    // V naší verzi nic nespouštíme ve vlákně, počítáme v Mainu
}

void Computation::stop() {
    running = false;
}

void Computation::join() {
    if(t.joinable()) {
        t.join();
    }
}

void Computation::run() {
    // Prázdná metoda
}
