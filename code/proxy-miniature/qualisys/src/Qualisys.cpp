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

#include <opendavinci/odcore/base/KeyValueConfiguration.h>
#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/io/tcp/TCPFactory.h>
#include <opendavinci/odcore/io/udp/UDPFactory.h>

#include "Qualisys.h"
#include "Buffer.h"

namespace opendlv {
namespace proxy {
namespace miniature {

Qualisys::Qualisys(const int &argc, char **argv)
    : DataTriggeredConferenceClientModule(
          argc, argv, "proxy-miniature-qualisys")
    , m_qualisysTCP()
    , m_qualisysUDP()
    , m_qualisysStringDecoder()
    , m_qualisysPacketListener()
{
}

Qualisys::~Qualisys() 
{
}

void Qualisys::setUp() 
{
  odcore::base::KeyValueConfiguration kv = getKeyValueConfiguration();
  
  bool const DEBUG = (kv.getValue<int32_t>("proxy-miniature-qualisys.debug") == 1);
  std::string const QUALISYS_IP = 
      kv.getValue<std::string>("proxy-miniature-qualisys.ip");
  uint32_t const QUALISYS_PORT = 
      kv.getValue<uint32_t>("proxy-miniature-qualisys.port");
  std::string const CLIENT_IP = 
      kv.getValue<std::string>("proxy-miniature-qualisys.client-ip");
  uint32_t const CLIENT_PORT = 
      kv.getValue<uint32_t>("proxy-miniature-qualisys.client-port");

  m_qualisysStringDecoder = 
      std::unique_ptr<QualisysStringDecoder>(new QualisysStringDecoder());
  m_qualisysPacketListener = 
      std::unique_ptr<QualisysPacketDecoder>(new QualisysPacketDecoder(
          getConference(), DEBUG));

  try {
    m_qualisysTCP = 
        std::shared_ptr<odcore::io::tcp::TCPConnection>(
            odcore::io::tcp::TCPFactory::createTCPConnectionTo(
                QUALISYS_IP, QUALISYS_PORT));
    m_qualisysTCP->setRaw(true);
    m_qualisysTCP->setStringListener(m_qualisysStringDecoder.get());
    m_qualisysTCP->start();
  } catch (std::string &exception) {
    std::cerr << "[" << getName() << "] Could not TCP connect to Qualisys: " 
        << exception << std::endl;
  }
  try {
    m_qualisysUDP = std::shared_ptr<odcore::io::udp::UDPReceiver>(
        odcore::io::udp::UDPFactory::createUDPReceiver(
            CLIENT_IP, CLIENT_PORT));
    m_qualisysUDP->setPacketListener(m_qualisysPacketListener.get());
    m_qualisysUDP->start();
  } catch (std::string &exception) {
    std::cerr << "[" << getName() << "] Could not open UDP socket: " 
        << exception << std::endl;
  }


  uint8_t freq = 60;

  TcpSendMsg("Version 1.12");
  TcpSendMsg("ByteOrder");
  TcpSendMsg("GetState");
  TcpSendMsg("StreamFrames Frequency:" + std::to_string(freq) + " UDP:" 
      + std::to_string(CLIENT_PORT) + " 3DNoLabels");

}

void Qualisys::tearDown() 
{
  TcpSendMsg("StreamFrames Stop");
  if (m_qualisysTCP.get() != NULL) {
    m_qualisysTCP->stop();
    m_qualisysTCP->setStringListener(NULL);
  }
  if (m_qualisysUDP.get() != NULL) {
    m_qualisysUDP->stop();
    m_qualisysUDP->setPacketListener(NULL);
  }
}

void Qualisys::nextContainer(odcore::data::Container &)
{
}

void Qualisys::TcpSendMsg(std::string const a_msg) const
{
  Buffer buffer;

  int32_t messageType = 1;
  int32_t bytesLength = 9 + a_msg.length();

  buffer.AppendInteger32(bytesLength);
  buffer.AppendInteger32(messageType);
  buffer.AppendStringRaw(a_msg);
  buffer.AppendByte(0);
  std::cout << "Sent: " << a_msg << std::endl;
  std::vector<unsigned char> bytes = buffer.GetData();
  std::string bytesString(bytes.begin(),bytes.end());
  m_qualisysTCP->send(bytesString);
}



}
}
}
