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

#ifndef PROXY_MINIATURE_SONARPRU_H
#define PROXY_MINIATURE_SONARPRU_H

#include <memory>
#include <string>
#include <utility>

#include <opendavinci/odcore/base/module/TimeTriggeredConferenceClientModule.h>

namespace opendlv {
namespace proxy {
namespace miniature {

/**
 * Interface to a sensor sensor using the BeagleBone Black PRU.
 */
class SonarPru : public odcore::base::module::TimeTriggeredConferenceClientModule {
   public:
    SonarPru(int32_t const &, char **);
    SonarPru(SonarPru const &) = delete;
    SonarPru &operator=(SonarPru const &) = delete;
    virtual ~SonarPru();

   private:
    virtual void setUp();
    virtual void tearDown();
    virtual odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode body();
   
    bool m_debug;
    bool m_initialized;
    uint16_t m_pruIndex;
    unsigned int *m_pruData;
};

} 
}
}

#endif
