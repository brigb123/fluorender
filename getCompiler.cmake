# /*
# For more information, please see: http://software.sci.utah.edu
# 
# The MIT License
# 
# Copyright (c) 2018 Scientific Computing and Imaging Institute,
# University of Utah.
# 
# 
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
# */

# Finds the Generator being used and caches the name internally, this is used for Boost
set(GeneratorName ${CMAKE_GENERATOR} CACHE INTERNAL "")
OPTION(MSVC_FOUND OFF)

message(STATUS "Generator: ${GeneratorName}")

if(${GeneratorName} STREQUAL "Visual Studio 16 2019")
  set(BoostToolset "vc142" CACHE PATH "")
	set(MSVC_FOUND ON)
elseif(${GeneratorName} STREQUAL "Visual Studio 15 2017")
  set(BoostToolset "vc141" CACHE PATH "")
	set(MSVC_FOUND ON)
elseif(${GeneratorName} STREQUAL "Visual Studio 14 2015")
  set(BoostToolset "vc14" CACHE PATH "")
	set(MSVC_FOUND ON)
elseif(${GeneratorName} STREQUAL "Ninja")
  set(BoostToolset "msvc" CACHE PATH "")
elseif(${GeneratorName} STREQUAL "Unix Makefiles")
  set(UNIX_FOUND ON)
elseif(${GeneratorName} STREQUAL "Xcode")
  set(APPLE_FOUND ON)
  set(Xcode_FOUND ON)
endif()

if(${BoostToolSet})
  message(STATUS "Boost Toolset set to: ${BoostToolset}")
endif()
