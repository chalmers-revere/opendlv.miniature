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

#ifndef PROXY_MINIATURE_GPIO_H
#define PROXY_MINIATURE_GPIO_H


#include <memory>
#include <string>
#include <vector>
#include <utility>

#include <opendavinci/odcore/base/module/TimeTriggeredConferenceClientModule.h>

namespace opendlv {
namespace proxy {
namespace miniature {

/**
 * Interface to GPIO.
 */
class Gpio : public odcore::base::module::TimeTriggeredConferenceClientModule {
 public:
  Gpio(const int &, char **);
  Gpio(const Gpio &) = delete;
  Gpio &operator=(const Gpio &) = delete;
  virtual ~Gpio();
  virtual void nextContainer(odcore::data::Container &);

 private:
  void setUp();
  void tearDown();
  virtual odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode body();

  void OpenGpio();
  void CloseGpio();
  void Reset();
  void SetDirection(uint16_t const, std::string);
  std::string GetDirection(uint16_t const) const;
  void SetValue(uint16_t const, bool const);
  bool GetValue(uint16_t const) const;

  bool m_debug;
  bool m_initialised;
  std::vector<std::pair<bool, std::string>> m_initialValuesDirections;
  std::string m_path;
  std::vector<uint16_t> m_pins;
};

}
}
}

#endif
