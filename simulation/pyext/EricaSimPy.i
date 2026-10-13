%module EricaSimPy
%{
#include <anlnext/BasicModule.hh>
#include "SMILEPhysicsList.hh"
#include "EdepPickUp.hh"
#include "SMILE2pGeometry.hh"
#include "StepLimit.hh"
#include "ParmaCRPrimaryGen.hh"
%}

%import(module="anlnext.anlnextpy") "anlnext/python/anlnextpy.i"

namespace ericasim {
class SMILEPhysicsList : public anlnext::BasicModule {};
class EdepPickUp       : public anlnext::BasicModule {};
class SMILE2pGeometry  : public anlnext::BasicModule {};
class StepLimit        : public anlnext::BasicModule {};
class ParmaCRPrimaryGen : public anlnext::BasicModule {};
}
