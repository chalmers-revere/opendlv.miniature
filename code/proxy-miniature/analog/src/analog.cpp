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

#include "analog.h"

namespace opendlv {
namespace proxy {
namespace miniature {

Analog::Analog(const int &argc, char **argv)
    : DataTriggeredConferenceClientModule(argc, argv, "proxy-miniature-analog")
{
}

Analog::~Analog() 
{
}

void Analog::setUp() 
{
}

void Analog::tearDown() 
{
}

void Analog::nextContainer(odcore::data::Container &a_container)
{
  (void) a_container;
}

odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode Analog::body()
{
  while (getModuleStateAndWaitForRemainingTimeInTimeslice() == odcore::data::dmcp::ModuleStateMessage::RUNNING) {
  }

  return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}

}
}
}
