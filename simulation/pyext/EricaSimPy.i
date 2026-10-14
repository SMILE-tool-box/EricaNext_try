%module EricaSimPy
%{
#include <anlnext/BasicModule.hh>
#include "SMILEPhysicsList.hh"
#include "EdepPickUp.hh"
#include "SMILE2pGeometry.hh"
#include "StepLimit.hh"
#include "ParmaCRPrimaryGen.hh"
#include "SimTrigger.hh"
#include "TPCEmulator.hh"
#include "HAEmulator.hh"
#include "RawWriter2p.hh"
%}

%import(module="anlnext.anlnextpy") "anlnext/python/anlnextpy.i"

namespace ericasim {
class SMILEPhysicsList : public anlnext::BasicModule {};
class EdepPickUp       : public anlnext::BasicModule {};
class SMILE2pGeometry  : public anlnext::BasicModule {};
class StepLimit        : public anlnext::BasicModule {};
class ParmaCRPrimaryGen : public anlnext::BasicModule {};
class SimTrigger       : public anlnext::BasicModule {};
class TPCEmulator      : public anlnext::BasicModule {};
class HAEmulator       : public anlnext::BasicModule {};
class RawWriter2p      : public anlnext::BasicModule {};
}
