/**
 * Copyright (C) 2016 Chalmers Revere
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


#include <cstdlib>
#include <iostream>

#include <opendavinci/odcore/base/KeyValueConfiguration.h>
#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/strings/StringToolbox.h>

#include <odvdminiature/GeneratedHeaders_ODVDMiniature.h>

#include "Example.h"

namespace opendlv {
namespace system {
namespace miniature {

Example::Example(const int &argc, char **argv)
    : TimeTriggeredConferenceClientModule(argc, argv, "system-miniature-example")
    , m_gpioPins()
    , m_pwmPins()
{
}

Example::~Example() 
{
}

void Example::setUp()
{
  odcore::base::KeyValueConfiguration kv = getKeyValueConfiguration();
  std::string const gpioPinsString = 
      kv.getValue<std::string>("system-miniature-example.gpio-pins");
  std::vector<std::string> gpioPinsVector = 
      odcore::strings::StringToolbox::split(gpioPinsString, ',');
  for (auto pin : gpioPinsVector) {
    m_gpioPins.push_back(std::stoi(pin)); 
  }
  std::string const pwmPinsString = 
      kv.getValue<std::string>("system-miniature-example.pwm-pins");
  std::vector<std::string> pwmPinsVector = 
      odcore::strings::StringToolbox::split(pwmPinsString, ',');
  for (auto pin : pwmPinsVector) {
    m_pwmPins.push_back(std::stoi(pin));
  }
}

void Example::tearDown()
{
}

odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode Example::body()
{
  while (getModuleStateAndWaitForRemainingTimeInTimeslice() == 
      odcore::data::dmcp::ModuleStateMessage::RUNNING) {
    for (auto pin : m_gpioPins) {
      bool value = static_cast<bool>(std::rand() % 2);
      opendlv::proxy::ToggleRequest::ToggleState state;
      if (value) {
        state = opendlv::proxy::ToggleRequest::On;
      } else {
        state = opendlv::proxy::ToggleRequest::Off;
      }
      opendlv::proxy::ToggleRequest request(pin, state);
      odcore::data::Container c(request);
      getConference().send(c);
      std::cout << "[" << getName() << "] Sending ToggleRequest: " 
          << request.toString() << std::endl;
    }
    for (auto pin : m_pwmPins) {
      int32_t rand = (std::rand() % 11) - 5 ;
      uint32_t value = 1500000 + rand * 100000;
      opendlv::proxy::PwmRequest request(pin, value);
      odcore::data::Container c(request);
      getConference().send(c);
      std::cout << "[" << getName() << "] Sending PwmRequest: " 
          << request.toString() << std::endl;
    }
  }
  return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}

void Example::nextContainer(odcore::data::Container &a_c)
{
  int32_t dataType = a_c.getDataType();
  if (dataType == opendlv::proxy::AnalogReading::ID()) {
    opendlv::proxy::AnalogReading reading = 
        a_c.getData<opendlv::proxy::AnalogReading>();
    std::cout << "[" << getName() << "] Received an AnalogReading: " 
        << reading.toString() << "." << std::endl;
  } else if (dataType == opendlv::proxy::ToggleReading::ID()) {
    opendlv::proxy::ToggleReading reading = 
        a_c.getData<opendlv::proxy::ToggleReading>();
    std::cout << "[" << getName() << "] Received a ToggleReading: "
        << reading.toString() << "." << std::endl;
  }
}


}
}
}
