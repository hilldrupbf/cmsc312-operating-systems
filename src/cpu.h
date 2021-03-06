#pragma once
#include <deque>
#include "pcb.h"

// Semaphore
struct Semaphore {
    int count = 1;
    std::deque<PCB> blockedProcesses = std::deque<PCB>();
};

// CPU class to handle execution of process instructions
class CPU {
    public:
        // Default constructor
        CPU();

        // void clockTick();
        void execute();

        // Getters
        double getClock();
        PCB& getPcb();

        // Setters
        void setClock(int clock);
        void setPcb(PCB pcb);

        const int totalCache = 5; // Kilobytes
        int remainingCache;

    private:
        PCB pcb;        // Current pcb on CPU
        int cycleTime;  // Time in milliseconds per clock cycle
        int timeSlice;  // Number of cycles before pre-emption
        int startTime;  // Start of execution


        // Creates an interrupt object that corresponds to the interrupted process
        struct Interrupt {
            int pid;        // Interrupted processes id
            ioType device;  // Which device caused interrupt

            // Constructor for quantum expiration interrupt
            Interrupt(int id) 
                : pid(id), device(NONE) {}
            // Constructor for IO interrupt
            Interrupt(int id, ioType io)
                : pid(id), device(io) {}
        };

        // Queue to hold interrupted processes
        std::deque<Interrupt> interrupts;

        // Semaphore methods 
        void wait(Semaphore S);
        void signal(Semaphore S);
};
