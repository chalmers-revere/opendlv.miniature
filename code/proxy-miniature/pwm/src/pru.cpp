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

#include <prussdrv.h>
#include <pruss_intc_mapping.h>

#include "pru.h"

namespace opendlv {
namespace proxy {
namespace miniature {

uint32_t *Pru::m_sharedMameory = nullptr;

Pru::Pru(uint8_t a_pruId):
  m_pruId(a_pruId)
{
  prussdrv_init();

  uint32_t res = prussdrv_open(PRU_EVTOUT_0);
  if (res) {
    std::cerr << "ERROR: Could not open PRU_EVTOUT_0" << std::endl;
  }

  tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
  prussdrv_pruintc_init(&pruss_intc_initdata);

  if (!m_sharedMemory) {
    static void *memPtr;
    prussdrv_map_prumem(PRUSS0_SHARED_DATARAM, &memPtr);
    m_sharedMemory = (uint32_t *) memPtr;
  }
}

Pru::~Pru() 
{
}

}
}
}
