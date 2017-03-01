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

#ifndef PROXY_MINIATURE_PWMPRU_H
#define PROXY_MINIATURE_PWMPRU_H

#include "pru.h"

namespace opendlv {
namespace proxy {
namespace miniature {

class PwmPru : private Pru {
   public:
    PwmPru(uint16_t);
    PwmPru(PwmPru const &) = delete;
    PwmPru &operator=(PwmPru const &) = delete;
    virtual ~PwmPru();

    void setDefaultPwmValue(uint16_t, uint64_t);
    void setPwmValue(uint16_t, uint64_t);
    void start();

   private:
    void setPruDuty(uint16_t, uint64_t);
    void updateDefaults();

    static uint16_t const nsPerCycle = 95;
    uint16_t pwmFrequency;
    uint16_t timeout;
};

} 
}
}

#endif
