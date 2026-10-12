%module EricaSimPy
%{
#include <anlnext/BasicModule.hh>
#include "SMILEPhysicsList.hh"
#include "EdepPickUp.hh"
%}

%import(module="anlnext.anlnextpy") "anlnext/python/anlnextpy.i"

namespace ericasim {
class SMILEPhysicsList : public anlnext::BasicModule {};
class EdepPickUp       : public anlnext::BasicModule {};
}
