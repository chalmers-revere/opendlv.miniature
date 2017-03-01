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

#include <odvdminiature/GeneratedHeaders_ODVDMiniature.h>

#include "LpsQualisys.h"

namespace opendlv {
namespace proxy {
namespace miniature {

LpsQualisys::LpsQualisys(const int &argc, char **argv)
    : DataTriggeredConferenceClientModule(argc, argv, "proxy-miniature-lps-qualisys")
    , m_qualisys()
{
}

LpsQualisys::~LpsQualisys() 
{
}

void LpsQualisys::setUp() 
{
  odcore::base::KeyValueConfiguration kv = getKeyValueConfiguration();
  const string QUALISYS_IP = 
      kv.getValue<std::string>("proxy-miniature-lps-qualisys.ip");
  const uint32_t QUALISYS_PORT = 
      kv.getValue< uint32_t >("proxy-miniature-lps-qualisys.port");
  // const bool DEBUG = 
  //     (kv.getValue< uint32_t >("proxy-miniature-lps-qualisys.debug") == 1);

  // Separating string decoding for GPS messages received from Trimble unit from this class.
  // Therefore, we need to pass the getConference() reference to the other instance so that it can send containers.
  // m_qualisysStringDecoder = std::unique_ptr< TrimbleStringDecoder >(new TrimbleStringDecoder(getConference(),DEBUG));

  try {
      m_qualisys = shared_ptr<odcore::io::tcp::TCPConnection>(odcore::io::tcp::TCPFactory::createTCPConnectionTo(QUALISYS_IP, QUALISYS_PORT));
      m_qualisys->setRaw(true);

      // m_qualisysStringDecoder is handling data from the Trimble unit.
      // m_qualisys->setStringListener(m_qualisysStringDecoder.get());
      // m_qualisys->start();
  } catch (string &exception) {
      stringstream info;
      info << "[" << getName() << "] Could not connect to Qualisys: " << exception << endl;
      toLogger(odcore::data::LogMessage::LogLevel::INFO, info.str());
  }
}

void LpsQualisys::tearDown() 
{
}

void LpsQualisys::nextContainer(odcore::data::Container &)
{

}



}
}
}
