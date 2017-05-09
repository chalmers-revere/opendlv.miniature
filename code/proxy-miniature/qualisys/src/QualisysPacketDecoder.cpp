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

#include <bitset>
#include <limits.h>

#include "Buffer.h"
#include "QualisysPacketDecoder.h"

#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/data/TimeStamp.h>

#include <odvdminiature/GeneratedHeaders_ODVDMiniature.h>


namespace opendlv {
namespace proxy {
namespace miniature {

QualisysPacketDecoder::QualisysPacketDecoder(
      odcore::io::conference::ContainerConference &a_conference, 
      bool a_debug) 
    : m_conference(a_conference)
    , m_debug(a_debug)
{}

QualisysPacketDecoder::~QualisysPacketDecoder() {}

void QualisysPacketDecoder::nextPacket(odcore::data::Packet const &a_packet)
{
  std::string const dataString = a_packet.getData();
  Buffer buffer;
  buffer.AppendStringRaw(dataString);
  if (m_debug) {
    auto data = buffer.GetData();
    std::cout << "Raw: " << std::endl;
    for(std::size_t i = 0; i < data.size(); i++) {
      std::cout << std::bitset<CHAR_BIT>(data[i]) << " ";
    }
    std::cout << std::endl;
  }

  std::shared_ptr<Buffer::Iterator> it = buffer.GetIterator();

  int32_t const packetLength = it->ReadInteger32();
  int32_t const packetType = it->ReadInteger32();

  if (m_debug) {
    std::cout 
        << "Received packet with length: " << packetLength 
        << " of type: " << packetType << std::endl;
  }

  if (packetType != 3) {
    std::cout 
        << "Unexpected answer from QTM RT server: Unrecognized packet type."
        << std::endl;
    return;
  }

  double const sensorTimestampMicroseconds = 
      ((int64_t) it->ReadInteger64() / 1e6);
  int32_t const frameNumber = it->ReadInteger32();
  int32_t const componentCount = it->ReadInteger32();

  if (m_debug) {
    std::cout 
        << "Time count in microseconds: " << sensorTimestampMicroseconds 
        << " Frame: " << frameNumber 
        << " componentCount: " << componentCount
        << std::endl;
  }

  if (componentCount != 1) {
    std::cout 
        << "Unexpected answer from QTM RT server: More than one component."
        << " Got: " << componentCount
        << " Expecting: 1"
        << std::endl;
    return;
  }

  int32_t const componentSize = it->ReadInteger32();
  int32_t const componentType = it->ReadInteger32();
  int32_t const markerCount = it->ReadInteger32();
  int16_t const qualityDrop = it->ReadInteger16();
  int16_t const qualitySync = it->ReadInteger16();
  float const quality = ((float) (qualityDrop + qualitySync)) / 2000.0f;
  if (m_debug) {
    std::cout 
        << "componentSize (bytes): " << componentSize 
        << " componentType: " << componentType 
        << " markerCount: " << markerCount 
        << " qualityDrop: " << qualityDrop
        << " qualitySync: " << qualitySync
        << " quality: " << quality
        << std::endl;
    
  }
  if (componentType != 2) {
    std::cout 
        << "Unexpected answer from QTM RT server: Unrecognized component type."
        << " Got: " << componentType
        << " Expecting: 2" 
        << std::endl;
    return;
  }

  std::vector<opendlv::model::Cartesian3> markers;
  for (int32_t j = 0; j < markerCount; j++)
  {
    float const x = it->ReadFloat32()/1e3f;
    float const y = it->ReadFloat32()/1e3f;
    float const z = it->ReadFloat32()/1e3f;
    int32_t const id = it->ReadInteger32();
    opendlv::model::Cartesian3 marker(x,y,z);
    if (m_debug) {
      std::cout << "ID: " << id << "|" << marker.toString() << std::endl;
    }
    markers.push_back(marker);
  }
  odcore::data::TimeStamp now;
  opendlv::proxy::QtmFrame frame(markers, now, quality, frameNumber);
  if (m_debug) {
    std::cout << "Sent: " << frame.toString() << std::endl;
  }
  odcore::data::Container c(frame);
  m_conference.send(c);
}

}
}
}
