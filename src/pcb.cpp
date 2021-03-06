#include <string>
#include <fstream>
#include <iostream>
#include "pcb.h"

// PCB constructor
PCB::PCB (int PID, state CurrentState, std::string templateFile){
    pid = PID;
    currentState = CurrentState;
    reqMem = (rand() % 5 + 1); // Random memory assignment between 1 and 5 Kilobytes
    instructionsList = ParseTemplate(templateFile);
    instructionsRemaining = instructionsList;
    progCount = 0;
    burst = calculateBurst();
    io = -1;
    childProcesses = std::vector<PCB*>();
}

int PCB::calculateBurst() {
    double burst = 0; 
    for (size_t i = 0; i < this->instructionsList.size(); i++) {
        int instrSize = this->instructionsList[i].remainingCycles;
        burst += instrSize;
    }

    return (int)burst;
}


void PCB::incrementInstrNum() {
    this->progCount++;
}

std::deque<instruction> PCB::ParseTemplate(const std::string tp) {
    std::deque<instruction> instructions;
    // Opens passed file name or prints error
    std::ifstream in(tp);
    std::string str;
    if(!in) {
        std::cout << ("ERROR: File not opened") << std::endl;
        exit(1);
    }

    // Loops until all lines have been read
    while (std::getline(in, str)) {
        // Ensures that the line is not blank
        if(str.size() > 0) {
            // Randomizes the number of instruction cycles for the given line
            instruction randInstr = Randomize(str);

            // Adds the randomized instruction to the deque of instructions
            instructions.push_back(randInstr); 
        }
    }

    // Add exit instruction to the end of the queue
    instruction exitInstr = {EXIT, NONE, 0};
    instructions.push_back(exitInstr);

    // Close the file
    in.close();

    return instructions;
}

// Replaces seeds with appropriate instruction cycles
instruction PCB::Randomize(std::string& str) {
    int firstSpace = (int)str.find(' '); // Index before min
    int lastSpace = (int)str.rfind(' '); // Index before max
    std::string::size_type sz;           // Size of string

    // Minimum and maximum values for randomization of cycles
    int min = std::stoi(str.substr(firstSpace, lastSpace), &sz);
    int max = std::stoi(str.substr(lastSpace), &sz);

    // Randomly generates a number of cycles in the given range
    int randCycles = (rand() % (max - min + 1)) + min;
    // Finds instruction type and IO device if instruction is IO
    instructionType type = getInstructionType(str.substr(0, firstSpace));
    ioType iotype = getIOType(str.substr(3, firstSpace - 3));

    // Creates a new string with randomized cycles to be returned
    instruction randomizedInstr = {type, iotype, randCycles};

    return randomizedInstr;
}

// Getters
int PCB::getPid() {
    return this->pid;
}

state PCB::getCurrentState() {
    return this->currentState;
}

int& PCB::getProgCount() {
    return this->progCount;
}

int PCB::getReqMem() {
    return this->reqMem;
}

int PCB::getCycle() {
    return this->cycle;
}

std::deque<int> PCB::getCpuRegisters() {
    return this->cpuRegisters;
}

std::deque<instruction>& PCB::getInstructionsList() {
    return this->instructionsList;
}

std::deque<instruction>& PCB::getInstructionsRemaining() {
    return this->instructionsRemaining;
}

instruction PCB::getNextInstruction() {
    instruction nextInstr = this->instructionsRemaining.front();
    this->instructionsRemaining.pop_front();
    return nextInstr;
}

void PCB::pushInstructionBack(instruction instr) {
    this->instructionsRemaining.push_front(instr);
}

int PCB::getBurst() {
    return this->burst;
}

int PCB::getIO() {
    return this->io;
}

std::vector<PCB*>& PCB::getChildProcesses() {
    return this->childProcesses;
}

instructionType PCB::getInstructionType(std::string line) {
    if(line.compare("CALCULATE") == 0) {
        return CALCULATE;
    }
    else if(line.compare(0, 2, "IO") == 0) {
        return IO;
    }
    else if(line.compare("CRITICAL") == 0) {
        return CRITICAL;
    }
    else if(line.compare("FORK") == 0) {
        return FORK;
    }
    return ERROR;
}

ioType PCB::getIOType(std::string line) {
    if(line.compare("Keyboard") == 0) {
        return KEYBOARD;
    }
    else if(line.compare("Monitor") == 0) {
        return MONITOR;
    }
    else if(line.compare("Disk") == 0) {
        return DISK;
    }
    return NONE;
}


// Setters
void PCB::setPid(int pid) {
    this->pid = pid;
}

void PCB::setCurrentState(state currentState) {
    this->currentState = currentState;
}

void PCB::setProgCount(int progCount) {
    this->progCount = progCount;
}

void PCB::setCycle(int cycle) {
    this->cycle = cycle;
}

void PCB::setCpuRegisters(std::deque<int> cpuRegisters) {
    this->cpuRegisters = cpuRegisters;
}

void PCB::setInstructions(std::deque<instruction> instructions) {
    this->instructionsList = instructions;
}

void PCB::setBurst(int burst) {
    this->burst = burst;
}

void PCB::setIO(int io) {
    this->io = io;
}