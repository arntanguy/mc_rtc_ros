#
# Copyright 2016-2019 CNRS-UM LIRMM, CNRS-AIST JRL
#

set(MOC_HEADERS
    Panel.h
    ArrayInputWidget.h
    ButtonWidget.h
    CategoryWidget.h
    CheckboxWidget.h
    ComboInputWidget.h
    ConnectionDialog.h
    FormElement.h
    FormWidget.h
    GenericInputWidget.h
    NumberSliderWidget.h
    PlotWidget.h
    PlotTabWidget.h
    # SchemaArrayInput.h
    # SchemaWidget.h
    TableWidget.h
    # ROS WIDGETS
    InteractiveMarkerWidget.h
    ArrowInteractiveMarkerWidget.h
    # ForceInteractiveMarkerWidget.h
    Point3DInteractiveMarkerWidget.h # XXX Do we need it in MOC headers?
    TransformInteractiveMarkerWidget.h
    VisualWidget.h
    XYThetaInteractiveMarkerWidget.h
    DisplayTrajectoryWidget.h
    PolygonMarkerWidget.h
    # PolyhedronMarkerWidget.h
    plugin.h)

if(NOT DEFINED ENV{ROS_DISTRO})
  set(ROS_DISTRO "none")
else()
  set(ROS_DISTRO "$ENV{ROS_DISTRO}")
endif()

# Stricty speaking we'd want to check <= indigo but the goal is to support 14.04
# and strictly go forward from that
if(NOT DEFINED ENV{MC_RTC_RVIZ_PANEL_FORCE_QT5})
  set(FORCE_QT5 0)
else()
  set(FORCE_QT5 $ENV{MC_RTC_RVIZ_PANEL_FORCE_QT5})
endif()
if(NOT ${FORCE_QT5} AND "${ROS_DISTRO}" STREQUAL "indigo")
  find_package(
    Qt4
    COMPONENTS QtCore QtGui
    REQUIRED)
  qt4_wrap_cpp(MOC_FILES ${MOC_HEADERS})
  qt4_add_resources(QRC_FILES icons.qrc)
else()
  find_package(
    Qt5
    COMPONENTS Core Gui Widgets
    REQUIRED)
  qt5_wrap_cpp(MOC_FILES ${MOC_HEADERS})
  qt5_add_resources(QRC_FILES icons.qrc)
endif()

set(HEADER_FILES
    ArrayLabelWidget.h
    ClientWidget.h
    ConnectionConfiguration.h
    FormElementContainer.h
    LabelWidget.h
    Schema.h
    utils.h
    )

set(SOURCE_FILES
    ArrayLabelWidget.cpp
    ArrayInputWidget.cpp
    ButtonWidget.cpp
    CategoryWidget.cpp
    CheckboxWidget.cpp
    ClientWidget.cpp
    ComboInputWidget.cpp
    ConnectionConfiguration.cpp
    ConnectionDialog.cpp
    FormElement.cpp
    FormElementContainer.cpp
    FormWidget.cpp
    GenericInputWidget.cpp
    LabelWidget.cpp
    NumberSliderWidget.cpp
    PlotWidget.cpp
    PlotTabWidget.cpp
    # Schema.cpp
    # SchemaArrayInput.cpp
    # SchemaWidget.cpp
    TableWidget.cpp
    InteractiveMarkerWidget.cpp
    ArrowInteractiveMarkerWidget.cpp
    # ForceInteractiveMarkerWidget.cpp
    Point3DInteractiveMarkerWidget.cpp
    TransformInteractiveMarkerWidget.cpp
    VisualWidget.cpp
    XYThetaInteractiveMarkerWidget.cpp
    DisplayTrajectoryWidget.cpp
    PolygonMarkerWidget.cpp
    # PolyhedronMarkerWidget.cpp
    utils.cpp
    plugin.cpp
    ${MOC_FILES}
    ${QRC_FILES})

if(NOT DEFINED MC_RTC_DOCDIR)
  set(MC_RTC_DOCDIR "${mc_rtc_DIR}/../../../share/doc/mc_rtc")
endif()
get_filename_component(MC_RTC_DOCDIR "${MC_RTC_DOCDIR}" ABSOLUTE)
if(NOT EXISTS "${MC_RTC_DOCDIR}")
  message(
    SEND_ERROR
      "Provided mc_rtc documentation directory (${MC_RTC_DOCDIR}) does not exist"
  )
endif()

add_library(_gui_objects OBJECT ${SOURCE_FILES})
target_compile_definitions(_gui_objects
                           PRIVATE -DMC_RTC_DOCDIR="${MC_RTC_DOCDIR}")

macro(panel_link_libraries target)
  target_include_directories(
    _gui_objects
    PUBLIC $<TARGET_PROPERTY:${target},INTERFACE_INCLUDE_DIRECTORIES>)
  target_compile_definitions(
    _gui_objects
    PUBLIC $<TARGET_PROPERTY:${target},INTERFACE_COMPILE_DEFINITIONS>)
  target_compile_features(
    _gui_objects
    PUBLIC $<TARGET_PROPERTY:${target},INTERFACE_COMPILE_FEATURES>)
endmacro()

if(NOT ${FORCE_QT5} AND "${ROS_DISTRO}" STREQUAL "indigo")
  panel_link_libraries(Qt4::QtCore)
  panel_link_libraries(Qt4::QtGui)
else()
  panel_link_libraries(Qt5::Core)
  panel_link_libraries(Qt5::Gui)
  panel_link_libraries(Qt5::Widgets)
endif()
target_compile_options(_gui_objects PRIVATE -fPIC)
target_include_directories(
  _gui_objects PUBLIC $<TARGET_PROPERTY:Qwt::Qwt,INTERFACE_INCLUDE_DIRECTORIES>)

panel_link_libraries(mc_rtc::mc_rtc_ros)
panel_link_libraries(mc_rtc::mc_control_client)
panel_link_libraries(rviz_common::rviz_common)
panel_link_libraries(pluginlib::pluginlib)
panel_link_libraries(visualization_msgs::visualization_msgs__rosidl_typesupport_cpp)
panel_link_libraries(interactive_markers::interactive_markers)
panel_link_libraries(tf2_ros::tf2_ros)

add_library(${PROJECT_NAME} Panel.cpp ${HEADER_FILES} ${MOC_HEADERS}
                            $<TARGET_OBJECTS:_gui_objects>)
target_link_libraries(${PROJECT_NAME} PUBLIC mc_rtc::mc_rtc_ros mc_rtc::mc_control_client Qwt::Qwt)
target_link_libraries(${PROJECT_NAME} PUBLIC mc_rtc::mc_rtc_ros)
target_link_libraries(${PROJECT_NAME} PUBLIC mc_rtc::mc_control_client)
target_link_libraries(${PROJECT_NAME} PUBLIC rviz_common::rviz_common)
target_link_libraries(${PROJECT_NAME} PUBLIC pluginlib::pluginlib)
target_link_libraries(${PROJECT_NAME} PUBLIC visualization_msgs::visualization_msgs__rosidl_typesupport_cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC interactive_markers::interactive_markers)
target_link_libraries(${PROJECT_NAME} PUBLIC tf2_ros::tf2_ros)


target_link_libraries(${PROJECT_NAME} PUBLIC)
if(NOT ${FORCE_QT5} AND "${ROS_DISTRO}" STREQUAL "indigo")
  target_link_libraries(${PROJECT_NAME} PUBLIC Qt4::QtCore Qt4::QtGui)
else()
  target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::Core Qt5::Gui Qt5::Widgets)
endif()

add_executable(mc_rtc_gui main.cpp)
target_link_libraries(mc_rtc_gui PUBLIC ${PROJECT_NAME})
target_compile_options(mc_rtc_gui PRIVATE -fPIC)

install(
  TARGETS ${PROJECT_NAME} mc_rtc_gui
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})
