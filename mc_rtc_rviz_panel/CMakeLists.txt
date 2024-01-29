#
# Copyright 2016-2023 CNRS-UM LIRMM, CNRS-AIST JRL
#

cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME mc_rtc_rviz_panel)
set(PROJECT_DESCRIPTION "mc_rtc RVIz panel for prototyping controllers")
set(PROJECT_URL "")
set(CMAKE_CXX_STANDARD 17)

project(
  ${PROJECT_NAME}
  LANGUAGES CXX
  VERSION 1.4.0)

find_package(mc_rtc REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/CMakeModules)
find_package(Qwt)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(rviz2 REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rviz_common REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(visualization_msgs REQUIRED)
find_package(interactive_markers REQUIRED)
# catkin_package(CATKIN_DEPENDS roscpp tf rviz roslib)

# include_directories(SYSTEM ${catkin_INCLUDE_DIRS})
# link_directories(${catkin_LIBRARY_DIRS})

# install(FILES rviz_plugin.xml DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})
pluginlib_export_plugin_description_file(rviz_common rviz_plugin.xml)

add_subdirectory(src)

ament_package()
