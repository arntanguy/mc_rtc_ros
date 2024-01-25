/*
 * Copyright 2016-2019 CNRS-UM LIRMM, CNRS-AIST JRL
 */

#include <rviz_common/panel.hpp>

#include "Panel.h"

namespace mc_rtc_rviz
{

class MyPanel : public rviz_common::Panel
{
  Q_OBJECT
public:
  MyPanel(QWidget * parent = 0);

  virtual ~MyPanel();

  mc_rtc_rviz::Panel * panel;
};

} // namespace mc_rtc_rviz
