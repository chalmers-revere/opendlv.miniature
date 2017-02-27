/**
 * proxy-miniature-gpio - Interface to gpio.
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

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <opendavinci/odcore/base/KeyValueConfiguration.h>
#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/strings/StringToolbox.h>

#include <odvdminiature/GeneratedHeaders_ODVDMiniature.h>

#include "Gpio.h"

namespace opendlv {
namespace proxy {
namespace miniature {

Gpio::Gpio(const int &argc, char **argv)
    : TimeTriggeredConferenceClientModule(argc, argv, "proxy-miniature-gpio")
    , m_debug()
    , m_initialised()
    , m_initialValuesDirections()
    , m_path()
    , m_pins()
{
}

Gpio::~Gpio() 
{
}

void Gpio::setUp()
{
  odcore::base::KeyValueConfiguration kv = getKeyValueConfiguration();

  m_debug = (kv.getValue<int32_t>("proxy-miniature-gpio.debug") == 1);

  m_path = kv.getValue<std::string>("proxy-miniature-gpio.systemPath");

  std::string const pinsString = 
      kv.getValue<std::string>("proxy-miniature-gpio.pins");
  std::vector<std::string> pinsVector = 
      odcore::strings::StringToolbox::split(pinsString, ',');


  std::string const initialValuesString = 
      kv.getValue<std::string>("proxy-miniature-gpio.values");

  std::vector<std::string> initialValuesVector = 
      odcore::strings::StringToolbox::split(initialValuesString, ',');
  
  std::string const initialDirectionsString =
      kv.getValue<std::string>("proxy-miniature-gpio.directions");

  std::vector<std::string> initialDirectionsVector =
      odcore::strings::StringToolbox::split(initialDirectionsString, ',');

  if (pinsVector.size() == initialValuesVector.size() 
      && pinsVector.size() == initialDirectionsVector.size()) {
    for (uint32_t i = 0; i < pinsVector.size(); i++) {
      uint16_t pin = std::stoi(pinsVector.at(i));
      bool value = static_cast<bool>(std::stoi(initialValuesVector.at(i)));
      std::string direction = initialDirectionsVector.at(i);
      if (direction.compare("out") == 0 || direction.compare("in") == 0) {
        m_pins.push_back(pin);
        m_initialValuesDirections.push_back(std::make_pair(value, direction));
      } else {
        cerr << "[" << getName() << "] " << "Invalid direction for pin " 
            << pin << "." << std::endl;
      }
    }
    if (m_debug) {
      std::cout << "[" << getName() << "] " << "Initialised pins: ";
      for (auto pin : m_pins) {
        std:: cout << pin << " ";
      }
      std::cout << "(Value, direction): ";
      for (auto pair : m_initialValuesDirections) {
        std::cout << "(" << pair.first << "," << pair.second << ") ";
      }
      std::cout << std::endl;
    }
  } else {
    cerr << "[" << getName() 
        << "] Number of pins do not equals to number of values or directions" 
        << std::endl;
  }

  OpenGpio();

  m_initialised = true;
}

void Gpio::tearDown()
{
  CloseGpio();
}

odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode Gpio::body()
{
  while (getModuleStateAndWaitForRemainingTimeInTimeslice() == 
      odcore::data::dmcp::ModuleStateMessage::RUNNING) {
    for (auto pin : m_pins) {
      // std::string direction = GetDirection(pin);
      bool value = GetValue(pin);
      opendlv::proxy::ToggleReading::ToggleState state;
      if (value) {
        state = opendlv::proxy::ToggleReading::On;
      } else {
        state = opendlv::proxy::ToggleReading::Off;
      }
      opendlv::proxy::ToggleReading reading(pin, state);
      odcore::data::Container c(reading);
      getConference().send(c);
    }
    if (m_debug) {
      std::cout << "Number of pins: " << m_pins.size() << std::endl;
      for (auto pin : m_pins) {
        std::cout << "[" << getName() << "] Pin: " << pin 
            << " Direction: " << GetDirection(pin) 
            << " Value: " << GetValue(pin) 
            << "." << std::endl;
      }
    }
  }
  return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}

void Gpio::nextContainer(odcore::data::Container &a_container)
{
  if (!m_initialised) {
    return;
  }
  (void) a_container;
  if (a_container.getDataType() == opendlv::proxy::ToggleRequest::ID()) {
    opendlv::proxy::ToggleRequest request = 
        a_container.getData<opendlv::proxy::ToggleRequest>();
    uint16_t pin = request.getPin();
    bool value = request.getState();
    if (GetDirection(pin).compare("out") == 0) {
      SetValue(pin, value);
    } else {
      cerr << "[" << getName() << "] The requested pin " << pin
          << " is read-only." 
          << std::endl;
    }
  }
}

void Gpio::OpenGpio()
{
  std::string filename = m_path + "/export";
  std::ofstream exportFile(filename, std::ofstream::out);
  
  if (exportFile.is_open()) {
    for (auto pin : m_pins) {
      exportFile << pin;
      exportFile.flush();
    }
    Reset();
  } else {
    cerr << "[" << getName() << "] Could not open " << filename << "." 
        << std::endl;
  }
  exportFile.close();
}

void Gpio::CloseGpio()
{
  std::string filename = m_path + "/unexport";
  std::ofstream unexportFile(filename, std::ofstream::out);
  
  if (unexportFile.is_open()) {
    for (auto pin : m_pins) {
      unexportFile << pin;
      unexportFile.flush();
    }
  } else {
    cerr << "[" << getName() << "] Could not open " << filename << "." 
        << std::endl;
  }
  unexportFile.close();
}

void Gpio::Reset()
{
  for (uint16_t i = 0; i < m_pins.size(); i++) {
    uint16_t pin = m_pins[i];
    bool initialValue = m_initialValuesDirections[i].first;
    std::string initialDirection = m_initialValuesDirections[i].second;
    SetDirection(pin, initialDirection);
    if (initialDirection.compare("out") == 0) {
      SetValue(pin, initialValue);
    }
  }
}

void Gpio::SetDirection(uint16_t const a_pin, std::string const a_str)
{
  std::string gpioDirectionFilename = m_path + "/gpio" + std::to_string(a_pin) 
      + "/direction";

  std::ofstream gpioDirectionFile(gpioDirectionFilename, std::ofstream::out);
  if (gpioDirectionFile.is_open()) {
    gpioDirectionFile << a_str;
    gpioDirectionFile.flush();
  } else {
    cerr << "[" << getName() << "] Could not open " << gpioDirectionFilename 
        << "." << std::endl;
  }

  gpioDirectionFile.close();
}

std::string Gpio::GetDirection(uint16_t const a_pin) const
{
  std::string gpioDirectionFilename = m_path + "/gpio" + std::to_string(a_pin) 
      + "/direction";
  std::string line;

  std::ifstream gpioDirectionFile(gpioDirectionFilename, std::ifstream::in);
  if (gpioDirectionFile.is_open()) {
    std::getline(gpioDirectionFile, line);
    std::string direction = line;
    gpioDirectionFile.close();
    return direction;
  } else {
    cerr << "[" << getName() << "] Could not open " << gpioDirectionFilename 
        << "." << std::endl;
    gpioDirectionFile.close();
    return "";
  }
}

void Gpio::SetValue(uint16_t const a_pin, bool const a_value)
{
  std::string gpioValueFilename = 
      m_path + "/gpio" + std::to_string(a_pin) + "/value";

  std::ofstream gpioValueFile(gpioValueFilename, std::ofstream::out);
  if (gpioValueFile.is_open()) {
    gpioValueFile << static_cast<uint16_t>(a_value);
    gpioValueFile.flush();
  } else {
    cerr << "[" << getName() << "] Could not open " << gpioValueFilename 
        << "." << std::endl;
  }
  gpioValueFile.close();
}

bool Gpio::GetValue(uint16_t const a_pin) const
{
  std::string gpioValueFilename = 
      m_path + "/gpio" + std::to_string(a_pin) + "/value";
  std::string line;

  std::ifstream gpioValueFile(gpioValueFilename, std::ifstream::in);
  if (gpioValueFile.is_open()) {
    std::getline(gpioValueFile, line);
    bool value = (std::stoi(line) == 1);
    gpioValueFile.close();
    return value;
  } else {
    cerr << "[" << getName() << "] Could not open " << gpioValueFilename 
        << "." << std::endl;
    gpioValueFile.close();
    return NULL;
  }
}

}
}
}
