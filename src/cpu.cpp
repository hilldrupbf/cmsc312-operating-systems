#include <ctime>
#include <thread>
#include <iostream>
#include "cpu.h"
#include "scheduler.h"
#include "dispatcher.h"
#include "os.h"

extern Dispatcher dispatcher;   // Defined in os.cpp
extern Scheduler scheduler;     // Defined in dispatcher.cpp
extern int totalProcesses;      // Defined in process.cpp
extern OS os;

CPU::CPU() {
    cycleTime = 10; // 10ms    
    timeSlice = 15; // 15 cycles before pre-emption
    // Saves starting time and prints OS start
    startTime = std::clock();
    this->interrupts = std::deque<Interrupt>();
}

// Execute process until interruption
void CPU::execute() {

    // Move interrupted processes back to ready queue
    // Processes only added to interrupt queue after finishing IO cycles on separate thread
    // or when pre-empted by expired time slice
    while(!interrupts.empty()) {
        // For each interrupt, remove it from the list of interrupts
        Interrupt interrupt = interrupts.front();
        interrupts.pop_front();

        // Moves the interrupted process from waiting to ready queue
        // Process ID's start at 1, pid=0 signifies an expired time slice interrupt
        if(interrupt.pid > 0) {
            PCB p = scheduler.getWaitingQueue().at(interrupt.pid);
            scheduler.getWaitingQueue().erase(interrupt.pid);
            dispatcher.addProcessToReadyQueue(p);
            std::cout << "Process " << interrupt.pid << " moved to ready queue" << std::endl;
        }
    }

    // If the ready queue has processes
    if(!(scheduler.getReadyQueue().empty())) {
        this->pcb = dispatcher.getPcbFromReady();
        std::cout << "Process " << this->pcb.getPid() << " on CPU" << std::endl;
        this->pcb.setCurrentState(RUNNING);

        // Retrieve instruction to be executed
        instruction currentInstruction = this->pcb.getNextInstruction();

        switch(currentInstruction.instrType) {
            // Calculate Instruction
            case 0:
            case 3:
            {
                std::cout << "Executing process " << this->pcb.getPid() << std::endl;
                // Number of cycles executed this time slice
                int cycleCount = 0;

                // Continue executing while there are cycles left in the instruction
                // and the instruction's time slice hasn't expired
                while(currentInstruction.remainingCycles > 0 && interrupts.empty()) {
                    cycleCount++;

                    // Sleep the thread for a number of milliseconds to simulate
                    // actual time needed to execute a clock cycle
                    std::this_thread::sleep_for(std::chrono::milliseconds(cycleTime));
                    currentInstruction.remainingCycles--;

                    // Checks to see if time slice has expired
                    if(cycleCount == timeSlice) {
                        // -1 chosen as its an impossible PID
                        interrupts.push_back(Interrupt(-1));
                        std::cout << "Time slice for process " << this->pcb.getPid() << " Expired" << std::endl;
                    }
                }

                // If instruction finished
                if (currentInstruction.remainingCycles == 0) {
                    std::cout << "Process " << this->pcb.getPid() << " CALCULATE instruction has finished" << std::endl;
                    // Increments the program counter if instruction completed
                    this->pcb.incrementInstrNum();
                }
                else {
                    // Otherwise put at front of instruction queue
                    this->pcb.pushInstructionBack(currentInstruction);
                }

                break;
            }
            // IO Instruction
            case 1:
            {
                std::cout << "Process " << this->pcb.getPid() << " beginning IO" << std::endl;
                // Execute IO on separate thread for concurrency
                // Meant to simulate how IO would not tie up the CPU
                int pid = this->pcb.getPid();
                // Process sleeps until IO instructions have completed
                std::thread ioThread([this, currentInstruction, pid]() {
                    std::this_thread::sleep_for(std::chrono::milliseconds(currentInstruction.remainingCycles * cycleTime));
                    std::cout << "Process " << pid << " finished IO" << std::endl;
                    // Once IO has finished, signal interrupt
                    interrupts.push_back(pid);
                });
                // Exception thrown if thread not detached or joined. Join would cause parent thread to wait
                ioThread.detach();

                // Increments the program counter if instruction completed
                this->pcb.incrementInstrNum();
                // Add process to waiting queue until its moved back to ready queue
                dispatcher.addProcessToWaitingQueue(this->pcb);

                break;
            }
            case 4:
            {
                os.fork(this->getPcb().getPid());
            }
            default:
            {
                break;
            }
        }

        // Checks to see that there is only the exit instruction left and process is not waiting
        if (this->pcb.getInstructionsRemaining().size() <= 1 && this->pcb.getCurrentState() != WAITING) {
            dispatcher.addProcessToTerminatedQueue(this->pcb);
            std::cout << "Process " << this->pcb.getPid() << " terminated" << std::endl;
        }

        // If the processes didnt hit IO instruction or terminate
        if (this->pcb.getCurrentState() == RUNNING) {
            // Dispatch program from cpu back to ready queue
            dispatcher.addProcessToReadyQueue(this->pcb);
        }
    }
    // Ready queue has no processes
    else {
        std::cout << "CPU Idle: No processes ready to be executed" << std::endl;
        // Thread sleeps so there is less output while idle
        std::this_thread::sleep_for(std::chrono::milliseconds(80)); 
    }
}



// Getters
double CPU::getClock() {
    double duration = (std::clock() - startTime) / (double) CLOCKS_PER_SEC;
    return duration;
}

PCB CPU::getPcb() {
    return this->pcb;
}

// Setters
void CPU::setPcb(PCB pcb) {
    this->pcb = pcb;
}



