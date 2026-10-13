cmake_minimum_required(VERSION 3.20)
# EricaNext simulation (Geant4 via anlgeant4). Build in the simulation environment:
#   source ~/opt/setup_sim.sh
#   cmake -S simulation -B simulation/build -DCMAKE_INSTALL_PREFIX=simulation/install && cmake --build simulation/build --target install
project(EricaSim CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release CACHE STRING "build type")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
# RPATH for both libEricaSim and the Python extension (set before the targets are created)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

set(ANLNEXT_INSTALL $ENV{ANLNEXT_INSTALL} CACHE PATH "ANLNext built with ANLNEXT_USE_G4VECTOR")
set(COMPTONSOFT_INSTALL $ENV{COMPTONSOFT_INSTALL} CACHE PATH "ComptonSoft (anlgeant4)")

find_package(Geant4 REQUIRED gdml)
find_package(ROOT REQUIRED)
find_package(Boost 1.80.0 CONFIG)

# ANLNext must be used with the same vector option it was built with
add_definitions(-DANLNEXT_USE_HEPVECTOR)

set(SIM_INC_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/ggg
  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/haboc_g4
  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/ha_common
  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/smile2p_geo
  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/gd_capture
  ${ANLNEXT_INSTALL}/include
  ${COMPTONSOFT_INSTALL}/include/anlgeant4
  ${Geant4_INCLUDE_DIRS}
  ${ROOT_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS})
include_directories(${SIM_INC_DIRS})
link_directories(${ANLNEXT_INSTALL}/lib ${COMPTONSOFT_INSTALL}/lib)
list(APPEND CMAKE_INSTALL_RPATH ${ANLNEXT_INSTALL}/lib ${COMPTONSOFT_INSTALL}/lib ${ROOT_LIBRARY_DIR})

file(GLOB SIM_SOURCES src/*.cc vendor/*/*.cxx)
add_library(EricaSim SHARED ${SIM_SOURCES})
target_link_libraries(EricaSim ANLNext ANLGeant4 ${Geant4_LIBRARIES} ${ROOT_LIBRARIES} ROOT::Graf)
install(TARGETS EricaSim LIBRARY DESTINATION lib)

# Python extension EricaSimPy (same helper as EricaPy / anlgeant4)
set(TARGET_EXT_LIBRARY EricaSimPy)
set(SWIG_IF_FILE ${CMAKE_CURRENT_SOURCE_DIR}/pyext/EricaSimPy.i)
set(PYTHON_EXT_INCLUDE_DIRS ${SIM_INC_DIRS})
set(PYTHON_EXT_LIBRARY_DIRS ${ANLNEXT_INSTALL}/lib ${COMPTONSOFT_INSTALL}/lib)
set(PYTHON_EXT_LIBRARIES ANLNext ANLGeant4 EricaSim)
set(SWIG_FLAGS -DANLNEXT_USE_HEPVECTOR)
list(APPEND CMAKE_MODULE_PATH ${ANLNEXT_INSTALL}/lib/anlnext)
include(CreateSwigPython)
