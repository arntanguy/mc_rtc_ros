/*
 * Copyright 2021 CNRS-UM LIRMM, CNRS-AIST JRL
 */

#pragma once

#include "ClientWidget.h"
#include "utils.h"

#ifdef MC_RTC_ROS_IS_ROS2
#  include <visualization_msgs/msg/marker_array.hpp>
#else
#  include <visualization_msgs/MarkerArray.h>
#endif

namespace mc_rtc_rviz
{

#ifdef MC_RTC_ROS_IS_ROS2
using MarkerArry = visualization_msgs::msg::MarkerArray;
using Marker = visualization_msgs::msg::Marker;
#else
using MarkerArry = visualization_msgs::MarkerArray;
using Marker = visualization_msgs::Marker;
#endif

struct VisualWidget : public ClientWidget
{
  Q_OBJECT
public:
  VisualWidget(const ClientWidgetParam & params, MarkerArray & markers);

  ~VisualWidget() override;

  void update(const rbd::parsers::Visual & visual, const sva::PTransformd & pose);

private:
  MarkerArray & markers_;
  Marker marker_;
  bool visible_;
  bool was_visible_;
  QPushButton * button_;
private slots:
  void toggled(bool);
};

} // namespace mc_rtc_rviz
