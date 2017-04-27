/**
 * proxy-miniature-pwm - Interface to pwm.
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

#include "Pwm.h"

namespace opendlv {
namespace proxy {
namespace miniature {

Pwm::Pwm(const int &argc, char **argv)
    : DataTriggeredConferenceClientModule(argc, argv, "proxy-miniature-pwm")
    , m_debug()
    , m_initialised()
    , m_path()
    , m_pins()
    , m_periodsNs()
    , m_dutyCyclesNs()
{
}

Pwm::~Pwm() 
{
}

void Pwm::setUp()
{
  odcore::base::KeyValueConfiguration kv = getKeyValueConfiguration();

  m_debug = (kv.getValue<int32_t>("proxy-miniature-pwm.debug") == 1);

  m_path = kv.getValue<std::string>("proxy-miniature-pwm.systemPath");

  std::string const pinsString = 
      kv.getValue<std::string>("proxy-miniature-pwm.pins");
  std::vector<std::string> pinsVector = 
      odcore::strings::StringToolbox::split(pinsString, ',');

  std::string const periodNsString = 
      kv.getValue<std::string>("proxy-miniature-pwm.periodsNs");
  std::vector<std::string> periodNsStringVector = 
      odcore::strings::StringToolbox::split(periodNsString, ',');
  
  std::string const dutyCycleNsString =
      kv.getValue<std::string>("proxy-miniature-pwm.dutyCyclesNs");
  std::vector<std::string> dutyCycleNsStringVector =
      odcore::strings::StringToolbox::split(dutyCycleNsString, ',');

  if (pinsVector.size() == periodNsStringVector.size() 
      && pinsVector.size() == dutyCycleNsStringVector.size()) {
    for (uint32_t i = 0; i < pinsVector.size(); i++) {
      uint16_t pin = std::stoi(pinsVector.at(i));
      int32_t periodNs = std::stoi(periodNsStringVector.at(i));
      int32_t dutyCycleNs = std::stoi(dutyCycleNsStringVector.at(i));
      m_pins.push_back(pin);
      m_periodsNs.push_back(periodNs);
      m_dutyCyclesNs.push_back(dutyCycleNs);
    }
    if (m_debug) {
      std::cout << "[" << getName() << "] " << "Initialised pins:";
      for (uint32_t i = 0; i < pinsVector.size(); i++) {
        std:: cout << "|Pin " << m_pins.at(i) << " Period " << m_periodsNs.at(i) 
            << " Duty cycle" << m_dutyCyclesNs.at(i);
      }
      std::cout << "." << std::endl;
    }
  } else {
    cerr << "[" << getName() 
        << "] Number of pins do not equals to number of periods or duty cycles." 
        << std::endl;
  }

  OpenPwm();

  m_initialised = true;
}

void Pwm::tearDown()
{
  ClosePwm();
}

void Pwm::nextContainer(odcore::data::Container &a_container)
{
  if (!m_initialised) {
    return;
  }
  if (a_container.getDataType() == opendlv::proxy::PwmRequest::ID() &&
      a_container.getSenderStamp() == getIdentifier()) {
    opendlv::proxy::PwmRequest request = 
        a_container.getData<opendlv::proxy::PwmRequest>();
    uint16_t pin = request.getPin();
    uint32_t dutyCycleNs = request.getDutyCycleNs();
    SetDutyCycleNs(pin, dutyCycleNs);
  }
}

void Pwm::OpenPwm()
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

void Pwm::ClosePwm()
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

void Pwm::Reset()
{
  for (uint8_t i = 0; i < m_pins.size(); i++) {
    SetEnabled(m_pins.at(i), false);
    SetPeriodNs(m_pins.at(i), m_periodsNs.at(i));
    SetDutyCycleNs(m_pins.at(i), m_dutyCyclesNs.at(i));
    SetEnabled(m_pins.at(i), true);
  }
}

void Pwm::SetEnabled(uint16_t const a_pin, bool const a_value)
{
  std::string filename = m_path + "/pwm" + std::to_string(a_pin) + "/enable";
  std::ofstream file(filename, std::ofstream::out);
  if (file.is_open()) {
    file << std::to_string((static_cast<int32_t>(a_value)));
    file.flush();
  } else {
    cerr << "[" << getName() << "] Could not open " << filename 
        << "." << std::endl;
  }
  file.close();
}

bool Pwm::GetEnabled(uint16_t const a_pin) const
{
  std::string filename = m_path + "/pwm" + std::to_string(a_pin) + "/enable";
  std::string line;

  std::ifstream file(filename, std::ifstream::in);
  if (file.is_open()) {
    std::getline(file, line);
    bool value = (line.compare("1") == 0);
    file.close();
    return value;
  } else {
    cerr << "[" << getName() << "] Could not open " << filename 
        << "." << std::endl;
    file.close();
    return NULL;
  }
}

void Pwm::SetDutyCycleNs(uint16_t const a_pin, uint32_t const a_value)
{
  std::string filename = m_path + "/pwm" + std::to_string(a_pin) + "/duty_cycle";

  std::ofstream file(filename, std::ofstream::out);
  if (file.is_open()) {
    file << std::to_string(a_value);
    file.flush();
  } else {
    cerr << "[" << getName() << "] Could not open " << filename 
        << "." << std::endl;
  }

  file.close();
}

uint32_t Pwm::GetDutyCycleNs(uint16_t const a_pin) const
{
  std::string filename = m_path + "/pwm" + std::to_string(a_pin) + "/duty_cycle";
  std::string line;

  std::ifstream file(filename, std::ifstream::in);
  if (file.is_open()) {
    std::getline(file, line);
    uint32_t value = std::stoi(line);
    file.close();
    return value;
  } else {
    cerr << "[" << getName() << "] Could not open " << filename 
        << "." << std::endl;
    file.close();
    return 0;
  }
}

void Pwm::SetPeriodNs(uint16_t const a_pin, uint32_t const a_value)
{
  std::string filename = m_path + "/pwm" + std::to_string(a_pin) + "/period";

  std::ofstream file(filename, std::ofstream::out);
  if (file.is_open()) {
    file << std::to_string(a_value);
    file.flush();
  } else {
    cerr << "[" << getName() << "] Could not open " << filename 
        << "." << std::endl;
  }
  file.close();
}

uint32_t Pwm::GetPeriodNs(uint16_t const a_pin) const
{
  std::string filename = m_path + "/pwm" + std::to_string(a_pin) + "/period";
  std::string line;

  std::ifstream file(filename, std::ifstream::in);
  if (file.is_open()) {
    std::getline(file, line);
    uint32_t value = std::stoi(line);
    file.close();
    return value;
  } else {
    cerr << "[" << getName() << "] Could not open " << filename 
        << "." << std::endl;
    file.close();
    return 0;
  }
}

}
}
}
