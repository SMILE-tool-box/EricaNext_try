// Minimal stand-in for SMLib's SM_Log (only SM_LOGGER(level) << ... is used here).
#ifndef EricaSim_SM_Log_shim_h
#define EricaSim_SM_Log_shim_h 1
#include <iostream>
#define SM_LOGGER(level) (std::cerr << "[" #level "] ")
#endif
namespace smlib {}   // HAPositionManager.cxx does `using namespace smlib;`
