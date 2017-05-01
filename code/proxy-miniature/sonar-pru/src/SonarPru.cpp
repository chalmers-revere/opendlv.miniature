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

#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <pruss/prussdrv.h>
#include <pruss/pruss_intc_mapping.h>

#include <opendavinci/odcore/base/KeyValueConfiguration.h>
#include <opendavinci/odcore/data/Container.h>

#include <odvdminiature/GeneratedHeaders_ODVDMiniature.h>
#include <opendavinci/odcore/strings/StringToolbox.h>

#include "SonarPru.h"

namespace opendlv {
namespace proxy {
namespace miniature {

SonarPru::SonarPru(const int &argc, char **argv)
    : TimeTriggeredConferenceClientModule(argc, argv, "proxy-miniature-sonar-pru")
    , m_debug(false)
    , m_initialized(false)
    , m_pruIndex()
    , m_pruData()
{
}

SonarPru::~SonarPru() 
{
}

void SonarPru::setUp() 
{
  m_initialized = true;

  odcore::base::KeyValueConfiguration kv = getKeyValueConfiguration();

  m_debug = (kv.getValue<int32_t>("proxy-miniature-sonar-pru.debug") == 1);
  m_pruIndex = kv.getValue<uint16_t>("proxy-miniature-sonar-pru.pruIndex");
  std::string firmwarePath = kv.getValue<std::string>("proxy-miniature-sonar-pru.firmwarePath");

  std::cout << "Initializing PRU" << m_pruIndex << std::endl;

  tpruss_intc_initdata prussIntcInitData = PRUSS_INTC_INITDATA;
  prussdrv_init();

  if (prussdrv_open(PRU_EVTOUT_0)) {
    std::cerr << "PRU" << m_pruIndex << " open failed" << std::endl;
    m_initialized = false;
    return;
  }

  prussdrv_pruintc_init(&prussIntcInitData);

  void *pruDataMem;
  prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, &pruDataMem);

  m_pruData = (unsigned int *) pruDataMem;

  std::cout << "Loading PRU binary " << firmwarePath << std::endl;
  prussdrv_exec_program(m_pruIndex, firmwarePath.c_str());
}

void SonarPru::tearDown() 
{
  if (m_initialized) {
    prussdrv_pru_disable(m_pruIndex);
    prussdrv_exit();
    std::cout << "PRU" << m_pruIndex << " was disabled." << std::endl;
  }
}


odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode SonarPru::body()
{
  while (getModuleStateAndWaitForRemainingTimeInTimeslice() == 
        odcore::data::dmcp::ModuleStateMessage::RUNNING) {

    if (!m_initialized) {
      continue;
    }

    prussdrv_pru_wait_event (PRU_EVTOUT_0);
    prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);

    // Roundtrip 1 cm: 58.44 us
    double distance = static_cast<double>(m_pruData[0]) / 58.44;
  
    opendlv::proxy::ProximityReading message(distance);
    odcore::data::Container c(message);
    getConference().send(c);

    if (m_debug) {
      std::cout << "Distance " << distance << std::endl;
    }
  }

  return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}

}
}
}
