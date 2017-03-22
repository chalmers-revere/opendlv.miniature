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

#include <iostream>
#include <vector>

#include <opendavinci/odcore/data/Container.h>

#include <odvdopendlvdata/GeneratedHeaders_ODVDOpenDLVData.h>

#include "pwm.h"
#include "pwmpru.h"

namespace opendlv {
namespace proxy {
namespace miniature {

Pwm::Pwm(const int &argc, char **argv)
    : DataTriggeredConferenceClientModule(argc, argv, "proxy-miniature-pwm"),
    m_pwmPru()
{
}

Pwm::~Pwm() 
{
}

void Pwm::setUp() 
{
  int32_t initialValue0 = 1000000;
  uint16_t failsafeTimeout = 2000;

  m_pwmPru = new PwmPru();
  m_pwmPru->setFailsafeValue(0, initialValue0);
  m_pwmPru->setFailsafeTimeout(failsafeTimeout);
  m_pwmPru->start();
  
  m_pwmPru->setChannelValue(channel, initialValue0);
}

void Pwm::tearDown() 
{
}

void Pwm::nextContainer(odcore::data::Container &a_container)
{
  (void) a_container;
  
  uint8_t channel = 0;
  int32_t value = 1200000;
  m_pru_Pwm->setChannelValue(channel, value);
}

odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode Pwm::body()
{
  while (getModuleStateAndWaitForRemainingTimeInTimeslice() == odcore::data::dmcp::ModuleStateMessage::RUNNING) {
  }

  return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}

}
}
}
