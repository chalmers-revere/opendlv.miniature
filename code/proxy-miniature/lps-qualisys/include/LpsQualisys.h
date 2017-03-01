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

#ifndef PROXY_MINIATURE_LPS_QUALISYS_H
#define PROXY_MINIATURE_LPS_QUALISYS_H

#include <memory>

#include <opendavinci/odcore/base/module/DataTriggeredConferenceClientModule.h>
#include <opendavinci/odcore/io/tcp/TCPConnection.h>

namespace opendlv {
namespace proxy {
namespace miniature {

class LpsQualisys : public odcore::base::module::DataTriggeredConferenceClientModule {
   public:
    LpsQualisys(int32_t const &, char **);
    LpsQualisys(LpsQualisys const &) = delete;
    LpsQualisys &operator=(LpsQualisys const &) = delete;
    virtual ~LpsQualisys();

   private:
    virtual void setUp();
    virtual void tearDown();
    virtual void nextContainer(odcore::data::Container &);


    std::shared_ptr<odcore::io::tcp::TCPConnection> m_qualisys;


};

} 
}
}

#endif
