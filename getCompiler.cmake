set(GeneratorName ${CMAKE_GENERATOR} CACHE INTERNAL "")

message(STATUS "Generator: ${GeneratorName}")

if(${GeneratorName} STREQUAL "MinGW Makefiles")
  set(BoostToolset "gcc" CACHE PATH "")
elseif(${GeneratorName} STREQUAL "Visual Studio 16 2019")
  set(BoostToolset "vc142" CACHE PATH "")
elseif(${GeneratorName} STREQUAL "Visual Studio 15 2017")
  set(BoostToolset "vc141" CACHE PATH "")
elseif(${GeneratorName} STREQUAL "Visual Studio 14 2015")
  set(BoostToolset "vc14" CACHE PATH "")
elseif(${GeneratorName} STREQUAL "Ninja")
  set(BoostToolset "msvc" CACHE PATH "")
endif()

message(STATUS "Boost Toolset set to: ${BoostToolset}")