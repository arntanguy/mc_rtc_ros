/*
 * Copyright 2016-2022 CNRS-UM LIRMM, CNRS-AIST JRL
 */

#include <mc_control/mc_global_controller.h>
#include <mc_rtc/config.h>
#include <mc_rtc/io_utils.h>
#include <mc_rtc/logging.h>
#include <mc_rtc/ros.h>
#include <mc_rtc/version.h>

#include <ros/ros.h>

#include <chrono>
#include <iostream>

namespace
{
template<typename T>
T getParam(ros::NodeHandle & n, const std::string & param)
{
  T out;
  std::string true_param;
  n.searchParam(param, true_param);
  n.getParam(true_param, out);
  return out;
}

template<typename T>
T getParam(ros::NodeHandle & n, const std::string & param, const T & def)
{
  if(n.hasParam(param))
  {
    return getParam<T>(n, param);
  }
  return def;
}
} // namespace

int main()
{
  auto nh_p = mc_rtc::ROSBridge::get_node_handle();
  if(!nh_p)
  {
    return 1;
  }
  auto nh = *nh_p;

  std::string conf = "";
  if(nh.hasParam("mc_rtc_ticker/conf"))
  {
    conf = getParam<std::string>(nh, "mc_rtc_ticker/conf");
    mc_rtc::log::info("Configuring mc_rtc with {}", conf);
  }

  if(mc_rtc::MC_RTC_VERSION != mc_rtc::version())
  {
    mc_rtc::log::error("mc_rtc_ticker was compiled with {} but mc_rtc is at version {}, you might face subtle issues "
                       "or unexpected crashes, please recompile mc_rtc_ticker",
                       mc_rtc::MC_RTC_VERSION, mc_rtc::version());
  }

  mc_control::MCGlobalController controller(conf);
  double dt = controller.timestep();

  /* Copy the control robot's joint values as encoder values for both the
   * control and real robot */
  auto set_encoder = [](mc_rbdyn::Robot & robot, mc_rbdyn::Robot & realRobot, bool warn = false) {
    const auto & rjo = robot.refJointOrder();
    auto & mbc = robot.mbc();
    auto q = robot.encoderValues();
    if(q.size() != rjo.size())
    {
      q.resize(rjo.size());
    }
    unsigned i = 0;
    for(const auto & jn : rjo)
    {
      if(robot.hasJoint(jn))
      {
        for(auto & qj : mbc.q[robot.jointIndexByName(jn)])
        {
          q[i++] = qj;
        }
      }
      else
      {
        if(robot.module().stance().count(jn))
        {
          auto stance_q = robot.module().stance().at(jn)[0];
          q[i++] = stance_q;
          if(warn)
          {
            mc_rtc::log::warning(
                "[mc_rtc_ticker] Joint {} is in ref_joint_order but not in robot {}, using its default stance: {}", jn,
                robot.name(), stance_q);
          }
        }
        else
        {
          if(warn)
          {
            mc_rtc::log::warning("[mc_rtc_ticker] Joint {} is in ref_joint_order but not in robot {}, and no default "
                                 "stance is provided, assuming 0.0",
                                 jn, robot.name());
          }
          q[i++] = 0;
        }
      }
    }
    robot.encoderValues(q);
    realRobot.encoderValues(q);
  };

  auto init_encoders = [](mc_rbdyn::Robot & robot, mc_rbdyn::Robot & realRobot, const std::vector<double> & initq) {
    mc_rtc::log::info("[mc_rtc_ticker] Loading encoder values from ROS param for robot {}: [{}]", robot.name(),
                      mc_rtc::io::to_string(initq));
    const auto & rjo = robot.refJointOrder();
    if(initq.size() != rjo.size())
    {
      mc_rtc::log::error_and_throw("[mc_rtc_ticker] Failed to initialize encoder values for robot \"{}\", you provided "
                                   "{} values but {} were expected",
                                   robot.name(), initq.size(), robot.refJointOrder().size());
    }
    robot.encoderValues(initq);
    realRobot.encoderValues(initq);

    auto & q = robot.mbc().q;
    for(size_t i = 0; i < rjo.size(); ++i)
    {
      const auto & jn = rjo[i];
      if(robot.hasJoint(jn))
      {
        q[robot.jointIndexByName(jn)][0] = initq[i];
      }
    }
    realRobot.mbc().q = robot.mbc().q;
    robot.forwardKinematics();
    realRobot.forwardKinematics();
  };

  // Load initial encoder values for the MainRobot from rosparam (legacy)
  if(nh.hasParam("mc_rtc_ticker/init_state"))
  {
    auto q = getParam<std::vector<double>>(nh, "mc_rtc_ticker/init_state");
    if(q.size() == controller.robot().refJointOrder().size())
    {
      init_encoders(controller.robot(), controller.realRobot(), q);
    }
    else
    {
      set_encoder(controller.robot(), controller.realRobot());
    }
  }
  else
  {
    for(auto & robot : controller.robots())
    {
      set_encoder(robot, controller.realRobot(robot.name()), true);
    }
  }

  // Load initial floating base attitude for the MainRobot from rosparam
  if(nh.hasParam("mc_rtc_ticker/init_pos"))
  {
    std::vector<double> pos = getParam<std::vector<double>>(nh, "mc_rtc_ticker/init_pos");
    if(pos.size() == 7)
    {
      std::array<double, 7> p;
      std::copy_n(pos.begin(), 7, p.begin());
      mc_rtc::log::info("[mc_rtc_ticker] Using initial attitude from ROS param for robot {}: {}",
                        controller.robot().name(), mc_rtc::io::to_string(p));
      controller.init(controller.robot().encoderValues(), p);
    }
    else
    {
      controller.init(controller.robot().encoderValues());
    }
  }
  else
  {
    controller.init(controller.robot().encoderValues());
  }

  // Load encoder/floating base attitude for additional robots either from rosparam or from the default module's
  // configuration
  for(auto & robot : controller.robots())
  {
    if(nh.hasParam("mc_rtc_ticker/init_state/" + robot.name()))
    {
      init_encoders(robot, controller.realRobot(robot.name()),
                    getParam<std::vector<double>>(nh, "mc_rtc_ticker/init_state/" + robot.name()));
    }
    else
    {
      set_encoder(robot, controller.realRobot(robot.name()));
    }

    if(nh.hasParam("mc_rtc_ticker/init_pos/" + robot.name()))
    {
      std::vector<double> initAttitude = getParam<std::vector<double>>(nh, "mc_rtc_ticker/init_pos/" + robot.name());
      mc_rtc::log::info("[mc_rtc_ticker] Using initial attitude from ROS param for robot {}: {}",
                        controller.robot().name(), mc_rtc::io::to_string(initAttitude));
      if(initAttitude.size() != 7)
      {
        mc_rtc::log::error_and_throw("[mc_rtc_ticker] Invalid attitude for robot {}: expected 7 values but {} provided",
                                     initAttitude.size());
      }
      auto & realRobot = controller.realRobot(robot.name());
      robot.mbc().q[0] = {std::begin(initAttitude), std::end(initAttitude)};
      mc_rtc::log::info("[mc_rtc_ticker] init q: {}", mc_rtc::io::to_string(robot.mbc().q[0]));
      realRobot.mbc().q[0] = robot.mbc().q[0];
      robot.forwardKinematics();
      realRobot.forwardKinematics();
    }
  }

  controller.running = true;

  bool stepByStep = getParam(nh, "mc_rtc_ticker/stepByStep", false);
  size_t nextStep = 0;
  auto gui = controller.controller().gui();
  auto toogleStepByStep = [&]() {
    if(stepByStep)
    {
      stepByStep = false;
    }
    else
    {
      nextStep = 0;
      stepByStep = true;
    }
  };
  if(gui)
  {
    gui->addElement({"mc_rtc_ticker"},
                    mc_rtc::gui::Checkbox(
                        "Step by step", [&]() { return stepByStep; }, [&]() { toogleStepByStep(); }));
    auto buttonText = [&](size_t n) {
      size_t n_ms = static_cast<size_t>(std::ceil(static_cast<double>(n) * 1000.0 * dt));
      return "+" + std::to_string(n_ms) + "ms";
    };
    gui->addElement({"mc_rtc_ticker"}, mc_rtc::gui::ElementsStacking::Horizontal,
                    mc_rtc::gui::Button(buttonText(1), [&]() { nextStep = 1; }),
                    mc_rtc::gui::Button(buttonText(5), [&]() { nextStep = 5; }),
                    mc_rtc::gui::Button(buttonText(10), [&]() { nextStep = 10; }),
                    mc_rtc::gui::Button(buttonText(50), [&]() { nextStep = 20; }),
                    mc_rtc::gui::Button(buttonText(100), [&]() { nextStep = 100; }));
  }

  const bool bench = getParam(nh, "mc_rtc_ticker/bench", false);
  std::chrono::time_point<std::chrono::system_clock> begin, end;
  std::vector<std::chrono::duration<double>> solve_dt(0);
  if(bench)
  {
    /* Reserve a million entry */
    solve_dt.reserve(1000000);
  }
  auto report_dt = [&solve_dt](bool partial) {
    mc_rtc::log::info("[mc_rtc_ticker] Ran for {} iterations: ", solve_dt.size());
    size_t start_i = partial ? solve_dt.size() - 1000 : 0;
    auto min_t = solve_dt[start_i];
    auto max_t = solve_dt[start_i];
    auto second_max_t = solve_dt[start_i];
    std::chrono::duration<double> average_t(0);
    for(size_t i = start_i; i < solve_dt.size(); ++i)
    {
      const auto & t = solve_dt[i];
      if(t > max_t)
      {
        second_max_t = max_t;
        max_t = t;
      }
      if(t < min_t)
      {
        min_t = t;
      }
      average_t += t;
    }
    if(partial)
    {
      average_t = average_t / 1000;
    }
    else
    {
      average_t = average_t / solve_dt.size();
    }
    mc_rtc::log::info("[mc_rtc_ticker] avg: {}ms, min: {}ms, max: {}ms, second max: {}ms", average_t.count() * 1000,
                      min_t.count() * 1000, max_t.count() * 1000, second_max_t.count() * 1000);
  };

  ros::Rate rt(1 / dt);
  std::thread spin_th;
  if(bench)
  {
    spin_th = std::thread([&rt]() {
      while(ros::ok())
      {
        ros::spinOnce();
        rt.sleep();
      }
    });
  }

  auto runController = [&]() { controller.run(); };

  auto updateGUI = [&]() {
    controller.running = false;
    controller.run();
    controller.running = true;
  };

  while(ros::ok())
  {
    if(bench)
    {
      begin = std::chrono::system_clock::now();
    }
    for(auto & robot : controller.robots())
    {
      set_encoder(robot, controller.realRobot(robot.name()));
    }
    if(stepByStep)
    {
      if(nextStep > 0)
      {
        nextStep--;
        runController();
      }
      else
      {
        updateGUI();
      }
    }
    else
    {
      runController();
    }
    if(bench)
    {
      end = std::chrono::system_clock::now();
      solve_dt.push_back(end - begin);
      if(solve_dt.size() % 1000 == 0)
      {
        report_dt(true);
      }
    }
    else
    {
      ros::spinOnce();
      rt.sleep();
    }
  }

  if(bench)
  {
    report_dt(false);
    spin_th.join();
  }

  return 0;
}
