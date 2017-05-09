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

#ifndef PROXY_MINIATURE_QUALISYS_H
#define PROXY_MINIATURE_QUALISYS_H

#include <memory>

#include <opendavinci/odcore/base/module/DataTriggeredConferenceClientModule.h>
#include <opendavinci/odcore/io/tcp/TCPConnection.h>
#include <opendavinci/odcore/io/udp/UDPReceiver.h>

#include "QualisysStringDecoder.h"
#include "QualisysPacketDecoder.h"

namespace opendlv {
namespace proxy {
namespace miniature {

class Qualisys : public odcore::base::module::DataTriggeredConferenceClientModule {
   public:
    Qualisys(int32_t const &, char **);
    Qualisys(Qualisys const &) = delete;
    Qualisys &operator=(Qualisys const &) = delete;
    virtual ~Qualisys();

   private:
    virtual void setUp();
    virtual void tearDown();
    virtual void nextContainer(odcore::data::Container &);

    void TcpSendMsg(std::string) const;


    std::shared_ptr<odcore::io::tcp::TCPConnection> m_qualisysTCP;
    std::shared_ptr<odcore::io::udp::UDPReceiver> m_qualisysUDP;
    std::unique_ptr<QualisysStringDecoder> m_qualisysStringDecoder;
    std::unique_ptr<QualisysPacketDecoder> m_qualisysPacketListener;

};

} 
}
}

#endif
