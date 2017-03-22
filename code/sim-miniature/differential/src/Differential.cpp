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

#include <opendavinci/odcore/base/KeyValueConfiguration.h>
#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/data/TimeStamp.h>

#include <opendlv/data/environment/Point3.h>

#include <odvdminiature/GeneratedHeaders_ODVDMiniature.h>

#include "Differential.h"

namespace opendlv {
namespace sim {
namespace miniature {

Differential::Differential(const int &argc, char **argv)
  : TimeTriggeredConferenceClientModule(
      argc, argv, "sim-miniature-differential")
  , m_currentState()
  , m_debug()
  , m_deltaTime()
  , m_leftWheelSpeed(0.0)
  , m_rightWheelSpeed(0.0)
{
}

Differential::~Differential()
{
}

void Differential::nextContainer(odcore::data::Container &a_c)
{
  int32_t dataType = a_c.getDataType();
  if (dataType == automotive::miniature::SensorBoardData::ID()) {
    automotive::miniature::SensorBoardData sensorBoardData = 
        a_c.getData<opendlv::proxy::AnalogReading>();
    if (m_debug) {
      std::cout << "[" << getName() << "] Received an SensorBoardData: " 
          << sensorBoardData.toString() << "." << std::endl;
    }
    ConvertSensorToAnalogReading(sensorBoardData);
  } else if (dataType == opendlv::proxy::ToggleReading::ID()) {
    opendlv::proxy::ToggleReading reading = 
        a_c.getData<opendlv::proxy::ToggleReading>();
    if (m_debug) {
      std::cout << "[" << getName() << "] Received a ToggleReading: "
          << reading.toString() << "." << std::endl;
    }
  } else if (dataType == opendlv::proxy::PwmRequest::ID()) {
    opendlv::proxy::PwmRequest request = 
        a_c.getData<opendlv::proxy::PwmRequest>();
    if (m_debug) {
      std::cout << "[" << getName() << "] Received a PwmRequest: "
          << request.toString() << "." << std::endl;
    }
    uint16_t pin = request.getPin();
    uint32_t dutyCycleNs = request.getDutyCycleNs();
    if (pin == 0 || pin == 1) {
      m_pwmActuators.at(pin) = dutyCycleNs;
    } else {
      std::cout << "[" << getName() << "] Received a PwmRequest with unsupported pin number: "
          << pin << "." << std::endl;
    }

  }
}

void Differential::setUp()
{
  odcore::base::KeyValueConfiguration kv = getKeyValueConfiguration();

  bool valueFound;
  m_debug = kv.getOptionalValue<bool>("sim-miniature-differential.debug", 
      valueFound);
  if (!valueFound) {
    m_debug = false;
  }

  m_deltaTime = 1 / getFrequency();
}

void Differential::tearDown()
{
}

odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode Differential::body()
{
  while (getModuleStateAndWaitForRemainingTimeInTimeslice() == 
      odcore::data::dmcp::ModuleStateMessage::RUNNING) {
    // // m_currentState = GetCurrentState();
    // odcore::data::Container c(m_currentState);
    // getConference().send(c);

  }

  return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}

void Differential::ConvertSensorToAnalogReading(
  automotive::miniature::SensorBoardData const &a_sensorBoardData)
{
  std::map<uint32_t, double> mapOfDistances = 
      a_sensorBoardData.getMapOfDistances();
  
  for (auto distanceReading : mapOfDistances) {
    uint32_t sensorId = distanceReading.first;
    double distance = distanceReading.second;

    opendlv::proxy::AnalogReading analogReading(sensorId, voltage);
    odcore::data::Container c(analogReading);
    send(c);
  }
}

// opendlv::data::environment::EgoState Differential::GetCurrentState()
// {
//   // opendlv::data::environment::Point3 previousPosition = m_currentState.getPosition();
//   // opendlv::data::environment::Point3 previousRotation = m_currentState.getRotation();
//   // opendlv::data::environment::Point3 previousVelocity = m_currentState.getVelocity();
//   // opendlv::data::environment::Point3 previousAcceleration = m_currentState.getAcceleration();

//   return m_currentState;
// }

}
}
} 
