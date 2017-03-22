/**
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

#include <iostream>
#include <string>
#include <vector>

#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/data/TimeStamp.h>

#include <odvdrhino/GeneratedHeaders_ODVDRhino.h>
#include <odvdvehicle/GeneratedHeaders_ODVDVehicle.h>

#include "Can.h"
#include "Vehicle.h"

namespace opendlv {
namespace sim {
namespace rhino {

Can::Can(const int &argc, char **argv)
  : DataTriggeredConferenceClientModule(argc, argv, "sim-miniature-differential")
  , m_vehicle(new Vehicle)
  , m_enableActuationBrake(false)
  , m_enableActuationSteering(false)
  , m_enableActuationThrottle(false)
{
}

Can::~Can()
{
}

void Can::nextContainer(odcore::data::Container &a_container)
{
  if (a_container.getDataType() == opendlv::proxy::ActuationRequest::ID()) {
    auto actuationRequest = 
      a_container.getData<opendlv::proxy::ActuationRequest>();

    bool const isValid = actuationRequest.getIsValid();

    float const acceleration = actuationRequest.getAcceleration();
    if (acceleration < 0.0f) {
      bool brakeEnabled = false;
      if (m_enableActuationBrake && isValid) {
        brakeEnabled = true;
      }

      opendlv::proxy::rhino::BrakeRequest brakeRequest;
      brakeRequest.setEnableRequest(brakeEnabled);

      float const max_deceleration = 2.0f;
      if (acceleration < -max_deceleration) {
        if (brakeEnabled) {
          std::cout << "WARNING: Deceleration was limited to " 
            << max_deceleration << ". This should never happen, and "
            << "may be a safety violating behaviour!" 
            << std::endl;
        }
        brakeRequest.setDeceleration(-max_deceleration);
      } else {
        brakeRequest.setDeceleration(acceleration);
      }

      m_vehicle->SetBrakeRequest(brakeRequest);
    } else {
      bool throttleEnabled = false;
      if (m_enableActuationThrottle && isValid) {
        throttleEnabled = true;
      }

      opendlv::proxy::rhino::AccelerationRequest accelerationRequest;
      accelerationRequest.setEnableRequest(throttleEnabled);
      accelerationRequest.setAccelerationPedalPosition(acceleration);

      m_vehicle->SetAccelerationRequest(accelerationRequest);
    }

    bool steeringEnabled = false;
    if (m_enableActuationSteering && isValid) {
      steeringEnabled = true;
    }

    float const steering = actuationRequest.getSteering();
    opendlv::proxy::rhino::SteeringRequest steeringRequest;
    steeringRequest.setEnableRequest(steeringEnabled);
    steeringRequest.setRoadWheelAngle(steering);

    // Must be 33.535 to disable deltatorque.
    steeringRequest.setDeltaTorque(33.535);

    m_vehicle->SetSteeringRequest(steeringRequest);
  }
}

void Can::setUp()
{
  bool valueFound = false;

  m_enableActuationBrake =
    getKeyValueConfiguration().getOptionalValue<bool>(
        "sim-rhino-can.enableActuationBrake", valueFound);
  if (!valueFound) {
    m_enableActuationBrake = false;
  }
  if (!m_enableActuationBrake) {
    std::cout << "The brakes are not enabled for control." << std::endl;
  }

  m_enableActuationSteering = 
    getKeyValueConfiguration().getOptionalValue<bool>(
        "sim-rhino-can.enableActuationSteering", valueFound);
  if (!valueFound) {
    m_enableActuationSteering = false;
  }
  if (!m_enableActuationSteering) {
    std::cout << "The steering is not enabled for control." << std::endl;
  }

  m_enableActuationThrottle = 
    getKeyValueConfiguration().getOptionalValue<bool>(
        "sim-rhino-can.enableActuationThrottle", valueFound);
  if (!valueFound) {
    m_enableActuationThrottle = false;
  }
  if (!m_enableActuationThrottle) {
    std::cout << "The throttle is not enabled for control." << std::endl;
  }
}

void Can::tearDown()
{
}

odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode Can::body()
{
  odcore::data::TimeStamp lastUpdate;
  while (getModuleStateAndWaitForRemainingTimeInTimeslice() == 
      odcore::data::dmcp::ModuleStateMessage::RUNNING) {

    odcore::data::TimeStamp now;
    float deltaTime = static_cast<float>((now.getMicroseconds() - lastUpdate.getMicroseconds()) / 1000000.0);
    lastUpdate = now;

    std::cout << "Delta time: " << deltaTime << std::endl; // Remove later.

    m_vehicle->Update(deltaTime);

    auto manualControl = m_vehicle->GetManualControl();
    odcore::data::Container manualControlContainer(manualControl);
    getConference().send(manualControlContainer);

    auto axles = m_vehicle->GetAxles();
    odcore::data::Container axlesContainer(axles);
    getConference().send(axlesContainer);
    
    auto propulsion = m_vehicle->GetPropulsion();
    odcore::data::Container propulsionContainer(propulsion);
    getConference().send(propulsionContainer);
    
    auto vehicleState = m_vehicle->GetVehicleState();
    odcore::data::Container vehicleStateContainer(vehicleState);
    getConference().send(vehicleStateContainer);
    
    auto wheels = m_vehicle->GetWheels();
    odcore::data::Container wheelsContainer(wheels);
    getConference().send(wheelsContainer);
    
    auto steering = m_vehicle->GetSteering();
    odcore::data::Container steeringContainer(steering);
    getConference().send(steeringContainer);
    
    auto driveline = m_vehicle->GetDriveline();
    odcore::data::Container drivelineContainer(driveline);
    getConference().send(drivelineContainer);
  }

  return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}

}
}
} 
