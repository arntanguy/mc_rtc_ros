#
# Copyright 2016-2024 CNRS-UM LIRMM, CNRS-AIST JRL
#

cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 17)
set(PROJECT_NAME mc_rtc_ticker)
project(
  ${PROJECT_NAME}
  LANGUAGES CXX
  VERSION 1.4.0)

find_package(ament_cmake REQUIRED)

install(DIRECTORY launch DESTINATION share/${PROJECT_NAME}/)

ament_package()
