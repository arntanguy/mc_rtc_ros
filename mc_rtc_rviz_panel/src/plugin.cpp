/*
 * Copyright 2016-2019 CNRS-UM LIRMM, CNRS-AIST JRL
 */

#include "plugin.h"
#include <rviz_common/panel.hpp>

namespace mc_rtc_rviz
{

MyPanel::MyPanel(QWidget * parent) : rviz_common::Panel(parent)
{
  auto layout = new QVBoxLayout();
  panel = new mc_rtc_rviz::Panel(parent);
  layout->QLayout::addWidget(panel);
  setLayout(layout);
}

MyPanel::~MyPanel() {}

} // namespace mc_rtc_rviz

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(mc_rtc_rviz::MyPanel, rviz_common::Panel)
