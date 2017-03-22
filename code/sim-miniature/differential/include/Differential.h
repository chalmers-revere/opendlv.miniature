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

#ifndef SIM_RHINO_CAN_H
#define SIM_RHINO_CAN_H

#include <memory>
#include <string>

#include <opendavinci/odcore/base/module/DataTriggeredConferenceClientModule.h>

namespace opendlv {
namespace sim {
namespace rhino {

class Vehicle;

/**
 * Simulated Rhino gateway (vehicle).
 */
class Can : public odcore::base::module::DataTriggeredConferenceClientModule {
   public:
    Can(int32_t const &, char **);
    Can(Can const &) = delete;
    Can &operator=(Can const &) = delete;
    virtual ~Can();

   private:
    void nextContainer(odcore::data::Container &);
    virtual void setUp();
    virtual void tearDown();
    virtual odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode body();

    std::unique_ptr<Vehicle> m_vehicle;
    bool m_enableActuationBrake;
    bool m_enableActuationSteering;
    bool m_enableActuationThrottle;
};

}
}
}

#endif
