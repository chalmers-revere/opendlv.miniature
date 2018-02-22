/**
 * ps4controller - Using a Ps4 controller to accelerate, brake, and steer a miniature vehicle
 * Copyright (C) 2017 Chalmers
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
#include <string>

#include <opendavinci/odcore/strings/StringToolbox.h>

#include "PS4Controller.h"

#if !defined(WIN32) && !defined(__gnu_hurd__) && !defined(__APPLE__)
#include <linux/joystick.h>
#include <fcntl.h>
#endif


namespace opendlv {
namespace proxy {
namespace miniature {

PS4Controller::PS4Controller(const int32_t &argc, char **argv)
    : TimeTriggeredConferenceClientModule(argc, argv,
        "proxy-miniature-ps4controller")
    , m_ps4controllerDevice(0)
    , m_axes() 
    , m_buttons()
    , m_latestJsNumber(0)
    , m_latestJsValue(0)
    , m_gsteerReq(0)
    , m_accelerationReq(0)
{}

PS4Controller::~PS4Controller() {}

void PS4Controller::setUp() {
    stringstream info;

    std::string const PS4CONTROLLER_DEVICE_NODE = getKeyValueConfiguration().getValue<std::string>(getName() + ".ps4controllerdevicenode");

    std::cout << "[" << getName() << "] Trying to open ps4controller " << PS4CONTROLLER_DEVICE_NODE << std::endl;
    
    string const ACC_MIN = getKeyValueConfiguration().getValue<string>(getName() + ".deceleration.max");
    string const ACC_MAX = getKeyValueConfiguration().getValue<string>(getName() + ".acceleration.max");
    string const MAX_STE = getKeyValueConfiguration().getValue<string>(getName() + ".steering.max");

    m_MAX_DECELERATION = std::stod(ACC_MIN,NULL);
    m_MAX_ACCELERATION = std::stod(ACC_MAX,NULL);
    m_MAX_STEERING_ANGLE = std::stod(MAX_STE,NULL);

    // Setup ps4controller control.
    #if !defined(WIN32) && !defined(__gnu_hurd__) && !defined(__APPLE__)
    int16_t num_of_axes = 0;
    int16_t num_of_buttons = 0;
    int16_t name_of_ps4controller[80];

    m_ps4controllerDevice = open(PS4CONTROLLER_DEVICE_NODE.c_str(), O_RDONLY);
    if (m_ps4controllerDevice == -1) {
        std::cerr << "[" << getName() << "] Could not open ps4controller " 
        << PS4CONTROLLER_DEVICE_NODE << std::endl;
        exit(1);
    }

    ioctl(m_ps4controllerDevice, JSIOCGAXES, &num_of_axes);
    ioctl(m_ps4controllerDevice, JSIOCGBUTTONS, &num_of_buttons);
    ioctl(m_ps4controllerDevice, JSIOCGNAME(80), &name_of_ps4controller);

    m_axes = std::unique_ptr<int32_t[]>((int32_t *)calloc(num_of_axes, sizeof(int32_t)));
    m_buttons = std::unique_ptr<int32_t[]>((int32_t *)calloc(num_of_buttons, sizeof(int32_t)));

    info.str("");
    std::cout << "[" << getName() 
        << "] PS4Controller found " << name_of_ps4controller
        << ", number of axes: " << num_of_axes
        << ", number of buttons: " << num_of_buttons << std::endl;
    toLogger(odcore::data::LogMessage::LogLevel::INFO, info.str());
  
    // Use non blocking reading.
    fcntl(m_ps4controllerDevice, F_SETFL, O_NONBLOCK);

    #else
    std::cout << "[" << getName() << "] This code will not work on this computer architecture." << std::endl;
    #endif
}

void PS4Controller::tearDown() {
    // Deactivate ps4controller control.
    close(m_ps4controllerDevice);
    m_gsteerReq.setSteeringAngle(0);
    m_accelerationReq.setPercent(0);
    sendRequest();
}

odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode PS4Controller::body()
{
    /* Lessons learned from live test:
     * - Care must be taken when activating the brakes, since braking too much 
     * is *very* uncomfortable -and potentially dangerous-, on the other hand a 
     * smaller maximum value for the deceleration could be dangerous as well, since it
     * could impair the possiblity to perform an emergency brake while using the controller.
     */

    while (getModuleStateAndWaitForRemainingTimeInTimeslice() == odcore::data::dmcp::ModuleStateMessage::RUNNING) {
        updatePS4ControllerStatus();
        prepareRequest();
        sendRequest();
    }
    return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}

void PS4Controller::updatePS4ControllerStatus()
{
    #if !defined(WIN32) && !defined(__gnu_hurd__) && !defined(__APPLE__)
    struct js_event js;

    while (read(m_ps4controllerDevice, &js, sizeof(struct js_event)) > 0) {
        int32_t jsNumber = (int32_t)js.number;
        int32_t jsValue = (int32_t)js.value;
        switch (js.type) {
            case JS_EVENT_AXIS:
                m_axes[jsNumber] = jsValue;
            break;
            case JS_EVENT_BUTTON:
                m_buttons[jsNumber] = jsValue;
            break;
            default:
            break;
        }
    }

    /* EAGAIN is returned when the queue is empty */
    if (errno != EAGAIN) {
        std::cerr << "[" << getName() << "] An error occurred in the PS4 joystick event handling" << std::endl;
    }

    #else
    #endif

}

void PS4Controller::prepareRequest()
{
    // std::cout << "[" << getName() << "] Stored input" << std::endl;
    // for (int32_t i = 0; i < num_of_axes; ++i) {
    //   std::cout << "Axis: " << i << ", Value: " << m_axes[i] << std::endl;
    // }
    // for (int32_t i = 0; i < num_of_buttons; ++i) {
    //   std::cout << "Button: " << i << ", Value: " << m_buttons[i] << std::endl;
    // }
    cerr << " L "<<m_axes[0]<<" R "<<m_axes[5] << std::endl;
    // steering - LEFT ANALOG STICK X - positive to the right, negative to the left
    m_gsteerReq.setSteeringAngle(-m_axes[0] / MAX_AXES_VALUE * m_MAX_STEERING_ANGLE * static_cast<float>(M_PI) / 180.0f);

    float percent=0.0f;
    // propulsion - RIGHT ANALOG STICK Y - negative up, positive down
    if(m_axes[5] < 0) { // user is accelerating
        percent = -m_axes[5] / MAX_AXES_VALUE * m_MAX_ACCELERATION; // final value should be positive
    } else if (m_axes[5] >= 0) { // user is decelerating
        percent = m_axes[5] / MAX_AXES_VALUE * m_MAX_DECELERATION; // final value should be negative
    }
    m_accelerationReq.setPercent(percent);
}

void PS4Controller::sendRequest()
{
    odcore::data::Container sr(m_gsteerReq);
    getConference().send(sr);

    odcore::data::Container ar(m_accelerationReq);
    getConference().send(ar);

    if (isVerbose()) {
//        std::cout << "m_latestJsNumber: " << m_latestJsNumber << std::endl;
//        std::cout << "m_latestJsValue: " << m_latestJsValue << std::endl;
        std::cout << "Sending: "<< std::endl
        << m_gsteerReq.toString() << std::endl
        << m_accelerationReq.toString() << std::endl;
    }
}


} 
} 
} 

