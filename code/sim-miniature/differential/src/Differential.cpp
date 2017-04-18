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
#include <math.h>

#include <opendavinci/odcore/base/KeyValueConfiguration.h>
#include <opendavinci/odcore/base/Lock.h>
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
  , m_mutex()
  , m_currentEgoState()
  , m_debug()
  , m_deltaTime()
  , m_leftWheelAngularVelocity(0.0)
  , m_rightWheelAngularVelocity(0.0)
{
}

Differential::~Differential()
{
}

void Differential::nextContainer(odcore::data::Container &a_c)
{
  odcore::base::Lock l(m_mutex);

  int32_t dataType = a_c.getDataType();
  if (dataType == automotive::miniature::SensorBoardData::ID()) {
    auto sensorBoardData = 
        a_c.getData<automotive::miniature::SensorBoardData>();
    if (m_debug) {
      std::cout << "[" << getName() << "] Received an SensorBoardData: " 
          << sensorBoardData.toString() << "." << std::endl;
    }
    ConvertSensorToAnalogReading(sensorBoardData);
  } else if (dataType == opendlv::proxy::ToggleReading::ID()) {
    auto reading = a_c.getData<opendlv::proxy::ToggleReading>();
    if (m_debug) {
      std::cout << "[" << getName() << "] Received a ToggleReading: "
          << reading.toString() << "." << std::endl;
    }
  } else if (dataType == opendlv::proxy::PwmRequest::ID()) {
    auto request = a_c.getData<opendlv::proxy::PwmRequest>();
    if (m_debug) {
      std::cout << "[" << getName() << "] Received a PwmRequest: "
          << request.toString() << "." << std::endl;
    }
    uint16_t pin = request.getPin();
    uint32_t dutyCycleNs = request.getDutyCycleNs();
    ConvertPwmToWheelAngularVelocity(pin, dutyCycleNs);
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
  
    odcore::base::Lock l(m_mutex);
  
    opendlv::data::environment::Point3 prevPosition = 
      m_currentEgoState.getPosition();
    opendlv::data::environment::Point3 prevRotation = 
      m_currentEgoState.getRotation();
    opendlv::data::environment::Point3 prevVelocity = 
      m_currentEgoState.getVelocity();

    double prevVelX = prevVelocity.getX();
    double prevVelY = prevVelocity.getY();
    
    // The division is needed due to a scaling problem, in order to convert into
    // meters. In your code below, everything will be meters.
    double prevPosX = prevPosition.getX() / 10.0;
    double prevPosY = prevPosition.getY() / 10.0;

    double prevYaw = atan2(prevRotation.getY(), prevRotation.getX());
    // NOTE: Do not change the code above.




    ///// TODO: Add kinematic equations below. Use the prepared class global
    ///// variables for wheel speeds (already saved, see the below method).

    double velX = 0.0; // Placeholder.
    double velY = 0.0; // Placeholder.
    //double yawRate = 0.0; // Placeholder.
   
    std::cout << "TODO: Add kinematic equations." << std::endl;
    ///// Kinematic equations above.




    ///// TODO: Integrate simulation below. The time step is already saved in
    ///// a global variable.

    double posX = prevPosX; // Placeholder.
    double posY = prevPosY; // Placeholder.
    double yaw = prevYaw; // Placeholder.
    
    std::cout << "TODO: Integrate simulation." << std::endl;
    ///// Integration above.



    

    // Due to a simulation scaling problem, the position is scaled. 
    // NOTE: Do not change the code below.
    posX = posX * 10.0;
    posY = posY * 10.0;

    double posZ = 0.0;
    double velZ = 0.0;
    double accZ = 0.0;

    double accX = (velX - prevVelX) / m_deltaTime;
    double accY = (velY - prevVelY) / m_deltaTime;

    opendlv::data::environment::Point3 position(posX, posY, posZ);
    opendlv::data::environment::Point3 rotation(1.0, 0.0, 0.0);
    opendlv::data::environment::Point3 velocity(velX, velY, velZ);
    opendlv::data::environment::Point3 acceleration(accX, accY, accZ);


    rotation.rotateZ(yaw);
    rotation.normalize();

    opendlv::data::environment::EgoState egoState(position, rotation, velocity,
        acceleration);

    m_currentEgoState = egoState;

    odcore::data::Container c(egoState);
    getConference().send(c);
  }

  return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}

void Differential::ConvertPwmToWheelAngularVelocity(uint16_t a_pin, 
    uint32_t a_dutyCycleNs)
{
  // NOTE: Do not change this method.
  int32_t const minReverseDutyCycleNs = 1000000; 
  int32_t const neutralDutyCycleNs = 1500000; 
  int32_t const maxForwardDutyCycleNs = 2000000; 

  double const minReverseAngularVelocity = -10.0;
  double const maxForwardAngularVelocity = 10.0;

  int32_t dutyCycleNs = (a_dutyCycleNs < minReverseDutyCycleNs || 
      a_dutyCycleNs > maxForwardDutyCycleNs) 
    ? neutralDutyCycleNs : a_dutyCycleNs;
  
  double wheelAngularVelocity;
  if (dutyCycleNs > neutralDutyCycleNs) {
    wheelAngularVelocity = maxForwardAngularVelocity * 
      (dutyCycleNs - neutralDutyCycleNs) / 
      static_cast<double>(maxForwardDutyCycleNs - neutralDutyCycleNs); 
  } else {
    wheelAngularVelocity = minReverseAngularVelocity * 
      (1.0 - (minReverseDutyCycleNs - dutyCycleNs) / 
      static_cast<double>(minReverseDutyCycleNs - neutralDutyCycleNs));
  }
      
  if (a_pin == 0) {
    m_leftWheelAngularVelocity = wheelAngularVelocity;
  } else if (a_pin == 1) {
    m_rightWheelAngularVelocity = wheelAngularVelocity;
  }
}

void Differential::ConvertSensorToAnalogReading(
  automotive::miniature::SensorBoardData const &a_sensorBoardData)
{
  // NOTE: Do not change this method.
  std::map<uint32_t, double> mapOfDistances = 
      a_sensorBoardData.getMapOfDistances();
 
  double const maxDistance = 4.0f * 10;

  for (auto distanceReading : mapOfDistances) {
    uint32_t sensorId = distanceReading.first;
    double distance = distanceReading.second;
    
    float voltage = 1.8f;
    if (distance > 0.0 && distance < maxDistance) {
      voltage = 1.8f * static_cast<float>(distance / maxDistance);
    }

    opendlv::proxy::AnalogReading analogReading(sensorId, voltage);
    odcore::data::Container c(analogReading);
    getConference().send(c);
  }
}

}
}
} 
