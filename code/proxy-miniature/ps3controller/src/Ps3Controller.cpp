/**
 * ps3controller - Using a Ps3 controller to accelerate, brake, and steer a miniature vehicle
 * Copyright (C) 2017 Chalmers Revere
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <stdint.h>
#include <iostream>
#include <cmath>
#include <thread>
#include <unistd.h>
#include <string>

#include <opendavinci/odcore/strings/StringToolbox.h>

#include "Ps3Controller.h"

#if !defined(WIN32) && !defined(__gnu_hurd__) && !defined(__APPLE__)
#include <linux/joystick.h>
#include <fcntl.h>
#endif


namespace opendlv {
namespace proxy {
namespace miniature {

Ps3Controller::Ps3Controller(const int32_t &argc, char **argv)
    : TimeTriggeredConferenceClientModule(argc, argv,
        "proxy-miniature-ps3controller")
    , m_ps3controllerDevice(0)
    , m_axes() 
    , m_buttons()
    , m_numAxes(0)
    , m_numButtons(0) 
    , m_latestJsNumber(0)
    , m_latestJsValue(0)
    , m_gsteerReq(0)
    , m_gasPedal(0)
    , m_reversePedal(0)
{}

Ps3Controller::~Ps3Controller() {}

void Ps3Controller::setUp() {
  std::stringstream info;
  odcore::base::KeyValueConfiguration kv = getKeyValueConfiguration();

  std::string const Ps3CONTROLLER_DEVICE_NODE = 
      kv.getValue<std::string>(getName() + ".ps3controllerdevicenode");

  std::cout << "[" << getName() << "] Trying to open ps3controller " 
      << Ps3CONTROLLER_DEVICE_NODE << std::endl;
  
  // Setup ps3controller control.
  #if !defined(WIN32) && !defined(__gnu_hurd__) && !defined(__APPLE__)
  // int16_t num_of_axes = 0;
  // int16_t num_of_buttons = 0;
  int16_t name_of_ps3controller[80];

  m_ps3controllerDevice = open(Ps3CONTROLLER_DEVICE_NODE.c_str(), O_RDONLY);
  if (m_ps3controllerDevice == -1) {
    std::cerr << "[" << getName() << "] Could not open ps3controller " 
        << Ps3CONTROLLER_DEVICE_NODE << std::endl;
    exit(1);
  }

  ioctl(m_ps3controllerDevice, JSIOCGAXES, &m_numAxes);
  ioctl(m_ps3controllerDevice, JSIOCGBUTTONS, &m_numButtons);
  ioctl(m_ps3controllerDevice, JSIOCGNAME(80), &name_of_ps3controller);

  m_axes = std::unique_ptr<int32_t[]>((int32_t *)calloc(m_numAxes, sizeof(int32_t)));
  m_buttons = std::unique_ptr<int32_t[]>((int32_t *)calloc(m_numButtons, sizeof(int32_t)));

  std::cout << "[" << getName() 
      << "] Ps3Controller found " << name_of_ps3controller
      << ", number of axes: " << m_numAxes
      << ", number of buttons: " << m_numButtons << std::endl;
  
  // Use non blocking reading.
  fcntl(m_ps3controllerDevice, F_SETFL, O_NONBLOCK);
  
  #else
  std::cout << "[" << getName() 
      << "] This code will not work on this computer architecture." 
      << std::endl;
  #endif
}

void Ps3Controller::tearDown() {
  // Deactivate ps3controller control.
  close(m_ps3controllerDevice);
  m_gsteerReq.setSteeringAngle(0);
  m_gasPedal.setPercent(0);
  m_reversePedal.setPercent(0);
  sendReq();
}


odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode Ps3Controller::body()
{
  /* Lessons learned from live test:
  * - Care must be taken when activating the brakes, since braking too much 
  * is *very* uncomfortable -and potentially dangerous-, on the other hand a 
  * smaller maximum value for the deceleration could be dangerous as well, since it
  * could impair the possiblity to perform an emergency brake while using the controller.
  */

  while (getModuleStateAndWaitForRemainingTimeInTimeslice() 
      == odcore::data::dmcp::ModuleStateMessage::RUNNING) {
    readPs3Controller();
    updateReq();
    sendReq();
    // std::cout << "[" << getName() << "] Looped " << std::endl;
    
  }
  return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}

void Ps3Controller::readPs3Controller()
{

  #if !defined(WIN32) && !defined(__gnu_hurd__) && !defined(__APPLE__)
  struct js_event js;
  
  while (read(m_ps3controllerDevice, &js, sizeof(struct js_event)) > 0) {
    int32_t jsNumber = (int32_t)js.number;
    int32_t jsValue = (int32_t)js.value;
    switch (js.type) {
      case JS_EVENT_AXIS:
        m_axes[jsNumber] = jsValue;
        break;
      case JS_EVENT_BUTTON:
        m_buttons[jsNumber] = jsValue;
        break;
      case JS_EVENT_INIT:
        break;
      default:
        break;
    }
  }

  /* EAGAIN is returned when the queue is empty */
  if (errno != EAGAIN) {
      std::cerr << "[" << getName() 
          << "] An error occurred in the Ps3 joystick event handling" << std::endl;
  }

  #else
  #endif

}

void Ps3Controller::updateReq()
{
  // std::cout << "[" << getName() << "] Stored input" << std::endl;
  // for (int32_t i = 0; i < m_numAxes; ++i) {
  //   std::cout << "Axis: " << i << ", Value: " << m_axes[i] << std::endl;
  // }
  // for (int32_t i = 0; i < m_numButtons; ++i) {
  //   std::cout << "Button: " << i << ", Value: " << m_buttons[i] << std::endl;
  // }
  float const MAX_STEERING_ANGLE = 32.0f;

  m_gsteerReq.setSteeringAngle(-m_axes[0] / MAX_AXES_VALUE * MAX_STEERING_ANGLE * static_cast<float>(M_PI) / 180.0f);

  if(m_axes[3] < 0) {
    m_gasPedal.setPercent(-m_axes[3] / MAX_AXES_VALUE * 0.5f);
    m_reversePedal.setPercent(0);
  } else if (m_axes[3] >= 0) {
    m_reversePedal.setPercent(m_axes[3] / MAX_AXES_VALUE * 1.0f);
    m_gasPedal.setPercent(0);
  }
}

void Ps3Controller::sendReq()
{
  odcore::data::Container cgsr(m_gsteerReq);
  getConference().send(cgsr);

  if (m_reversePedal.getPercent() > 0) {
    odcore::data::Container crp(m_reversePedal);
    crp.setSenderStamp(2);
    getConference().send(crp);
  } else {
    odcore::data::Container cgp(m_gasPedal);
    cgp.setSenderStamp(1);
    getConference().send(cgp);
  }
  if (isVerbose()) {
    // std::cout << "m_latestJsNumber: " << m_latestJsNumber << std::endl;
    // std::cout << "m_latestJsValue: " << m_latestJsValue << std::endl;
    std::cout << "Sending: "<< std::endl
        << m_gsteerReq.toString() << std::endl
        << m_gasPedal.toString() << std::endl
        << m_reversePedal.toString() << std::endl;
  }
}


} 
} 
} 

