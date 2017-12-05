/**
 * proxy-miniature-pwm-motor - Interface to motor through pwm.
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

#ifndef PROXY_MINIATURE_MOTOR_H
#define PROXY_MINIATURE_MOTOR_H

#include <string>


namespace opendlv {
namespace proxy {
namespace miniature {

class Motor {
 public:
  enum MotorType { Servo, Esc };
  Motor(std::string const &, MotorType const &, uint8_t const &);
  Motor(Motor const &) = default;
  Motor &operator=(Motor const &) = default;
  virtual ~Motor();
  
  std::string getName();
  MotorType getType();
  uint8_t getChannel();
  void setPower(float const);
  float getPower();
  std::string toString();


 private:
  std::string m_name;
  MotorType m_type;
  uint8_t m_channel;
  float m_power;
};

}
}
}

#endif