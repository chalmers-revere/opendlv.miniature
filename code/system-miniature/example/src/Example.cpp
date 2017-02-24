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
    , m_digitalPins()
{
}

Example::~Example() 
{
}

void Example::setUp()
{
  odcore::base::KeyValueConfiguration kv = getKeyValueConfiguration();
  std::string const digitalPinsString = 
      kv.getValue<std::string>("system-miniature-example.digital-pins");
  std::vector<std::string> digitalPinsVector = 
      odcore::strings::StringToolbox::split(digitalPinsString, ',');
  for (auto pin : digitalPinsVector) {
    m_digitalPins.push_back(std::stoi(pin)); 
  }
}

void Example::tearDown()
{
}

odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode Example::body()
{
  while (getModuleStateAndWaitForRemainingTimeInTimeslice() == 
      odcore::data::dmcp::ModuleStateMessage::RUNNING) {
    for (auto pin : m_digitalPins) {
      int32_t value = std::rand() % 2;
      opendlv::proxy::DigitalRequest request(pin, value);
      odcore::data::Container c(request);
      getConference().send(c);
      std::cout << "[" << getName() << "] Sending DigitalRequest: " 
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
  } else if (dataType == opendlv::proxy::DigitalReading::ID()) {
    opendlv::proxy::DigitalReading reading = 
        a_c.getData<opendlv::proxy::DigitalReading>();
    std::cout << "[" << getName() << "] Received a DigitalReading: "
        << reading.toString() << "." << std::endl;
  }
}


}
}
}
