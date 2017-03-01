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

#ifndef PROXY_MINIATURE_PRU_H
#define PROXY_MINIATURE_PRU_H

#include <string>

namespace opendlv {
namespace proxy {
namespace miniature {

class Pru {
   public:
    Pru(uint8_t);
    Pru(Pru const &) = delete;
    Pru &operator=(Pru const &) = delete;
    virtual ~Pru();

    void getSharedMemory(uint32_t *, uint32_t, uint32_t);
	  uint32_t getSharedMemoryInteger(uint32_t);
    void run(std::string const &);
	  void setSharedMemoryInteger(uint32_t, uint32_t);
	  void setSharedMemory(u_int32_t *, uint32_t, uint32_t);
	  void stop();

   private:
    uint8_t m_pruId;
	  static uint32_t* m_sharedMemory;
    static uint32_t const OFFSET_SHAREDRAM = 2048;
};

} 
}
}

#endif
