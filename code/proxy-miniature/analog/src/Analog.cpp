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

#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <opendavinci/odcore/base/KeyValueConfiguration.h>
#include <opendavinci/odcore/data/Container.h>

#include <odvdminiature/GeneratedHeaders_ODVDMiniature.h>
#include <opendavinci/odcore/strings/StringToolbox.h>

#include "Analog.h"

namespace opendlv {
namespace proxy {
namespace miniature {

Analog::Analog(const int &argc, char **argv)
    : TimeTriggeredConferenceClientModule(argc, argv, "proxy-miniature-analog")
    , m_conversionConst()
    , m_debug()
    , m_pins()
{
}

Analog::~Analog() 
{
}

void Analog::setUp() 
{
  odcore::base::KeyValueConfiguration kv = getKeyValueConfiguration();

  m_conversionConst = 
      kv.getValue<float>("proxy-miniature-analog.conversion-constant");
  m_debug = (kv.getValue<int32_t>("proxy-miniature-analog.debug") == 1);
  std::string pinsString = 
      kv.getValue<std::string>("proxy-miniature-analog.pins");
  std::vector<std::string> pinsVecString = 
      odcore::strings::StringToolbox::split(pinsString, ',');
  for(std::string const& str : pinsVecString) {
    m_pins.push_back(std::stoi(str));
  }
}

void Analog::tearDown() 
{
}


odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode Analog::body()
{
  while (getModuleStateAndWaitForRemainingTimeInTimeslice() == 
        odcore::data::dmcp::ModuleStateMessage::RUNNING) {
    std::vector<std::pair<uint16_t, float>> reading = getReadings();
    for (std::pair<uint16_t, float> const& pair : reading) {
      opendlv::proxy::AnalogReading message(pair.first, pair.second);
      odcore::data::Container c(message);
      getConference().send(c);
    }
    if(m_debug) {
      std::cout << "[" << getName() << "] ";
      for (std::pair<uint16_t, float> const& pair : reading) {
        std::cout << "Pin " << pair.first << ": " << pair.second << " ";
      }
      std::cout << std::endl;
    }
  }
  return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}

std::vector<std::pair<uint16_t, float>> Analog::getReadings() {
  std::vector<std::pair<uint16_t, float>> reading;
  for(uint16_t const pin : m_pins) {
    std::string filename = "/sys/bus/iio/devices/iio:device0/in_voltage" 
        + std::to_string(pin) + "_raw";
    std::ifstream file(filename, std::ifstream::in);
    std::string line;
    if(file.is_open()){
      std::getline(file, line);
      uint16_t rawReading = std::stoi(line);
      reading.push_back(std::make_pair(pin, rawReading*m_conversionConst));
    } else {
      std::cerr << "[" << getName() 
          << "] Could not read from analog input. (pin: " << pin 
          << ", filename: " << filename << ")" << std::endl;
      reading.push_back(std::make_pair(pin,std::nanf("")));
    }
    file.close();
  }
  return reading;
}

}
}
}
