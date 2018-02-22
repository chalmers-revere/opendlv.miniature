/**
 * ps4controller - Using a Ps4 controller to accelerate, brake, and steer a miniature vehicle.
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

#ifndef Ps4CONTROLLER_H
#define Ps4CONTROLLER_H

#include <opendavinci/odcore/base/module/TimeTriggeredConferenceClientModule.h>
#include <odvdminiature/GeneratedHeaders_ODVDMiniature.h>
#include <vector>
#include <unistd.h>

namespace opendlv {
namespace proxy {
namespace miniature {

/**
 * Using a PS4 controller to accelerate, brake, and steer a Drone.
 */
 
 
/*
    List of buttons and axes.
    Almost each button has an associated axis, 
    in order to sense the pressure applied to the button.
    The buttons generate a binary event, 
    the axes return a value between MIN_AXES_VALUE and MAX_AXES_VALUE.

    To familiarize with the controller buttons/axes and their values, 
    it is advised to use the code and sample program available at https://github.com/drewnoakes/joystick

    Axis 7 = BUTTON ARROW UP & DOWN
    Axis 6 = BUTTON ARROW RIGHT & LEFT

    Button 10 = LEFT ANALOG BUTTON
    Axis 0 = LEFT ANALOG STICK X
    Axis 1 = LEFT ANALOG STICK Y
    Button 11 = RIGHT ANALOG BUTTON
    Axis 2 = RIGHT ANALOG STICK X
    Axis 5 = RIGHT ANALOG STICK Y

    Button 1 = BUTTON X
    Button 2 = BUTTON CIRCLE
    Button 3 = BUTTON TRIANGLE
    Button 0 = BUTTON SQUARE

    Button 4 = BUTTON L1
    Button 6 & Axis 3 = BUTTON L2
    Button 5 = BUTTON R1
    Button 7 & Axis 4 = BUTTON R2

    Button 8 = BUTTON SHARE
    Button 9 = BUTTON OPTIONS
    Button 13 = BUTTON TOUCHPAD
*/

class PS4Controller 
    : public odcore::base::module::TimeTriggeredConferenceClientModule {
public:
    PS4Controller(int32_t const &, char **);
    PS4Controller(PS4Controller const &) = delete;
    PS4Controller &operator=(PS4Controller const &) = delete;
    virtual ~PS4Controller();

private:
    void setUp();
    void tearDown();
    odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode body();
    void updatePS4ControllerStatus();
    void prepareRequest();
    void sendRequest();

    /** Minimum value of axes range */
    static constexpr float MIN_AXES_VALUE = -32768.0f;
    /** Maximum value of axes range */
    static constexpr float MAX_AXES_VALUE = 32767.0f;

    int32_t m_ps4controllerDevice;
    std::unique_ptr<int32_t[]> m_axes;
    std::unique_ptr<int32_t[]> m_buttons;
    int32_t m_latestJsNumber;
    int32_t m_latestJsValue;
    
    opendlv::proxy::GroundSteeringRequest m_gsteerReq;
    opendlv::proxy::PedalPositionReading m_accelerationReq;

    float m_ACCELERATION_MIN = 0;
    float m_ACCELERATION_MAX = 0;
    float m_MAX_STEERING_ANGLE = 0;
};

}
}
} 

#endif /*PS4CONTROLLER_H*/
