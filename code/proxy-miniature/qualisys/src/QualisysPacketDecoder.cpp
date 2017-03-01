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

#include "Buffer.h"
#include "QualisysPacketDecoder.h"

#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/data/TimeStamp.h>

#include <odvdminiature/GeneratedHeaders_ODVDMiniature.h>


namespace opendlv {
namespace proxy {
namespace miniature {

QualisysPacketDecoder::QualisysPacketDecoder(odcore::io::conference::ContainerConference &a_conference, bool a_debug) 
    : m_conference(a_conference)
    , m_debug(a_debug)
{}

QualisysPacketDecoder::~QualisysPacketDecoder() {}

void QualisysPacketDecoder::nextPacket(odcore::io::Packet const &a_packet)
{
  std::string const dataString = a_packet.getData();
  std::vector<unsigned char> data(dataString.begin(), dataString.end());
  Buffer const buffer(data);
  std::shared_ptr<Buffer::Iterator> it = buffer.GetIterator();

  int32_t const packetLength = it->ReadInteger();
  int32_t const packetType = it->ReadInteger();

  if (m_debug) {
    std::cout << "Received packet with length: " << packetLength 
        << " of type: " << packetType << std::endl;
  }

  if (packetType != 3) {
    std::cout 
        << "Unexpected answer from QTM RT server: Unrecognized packet type."
        << std::endl;
    return;
  }

  double const sensorTimestampMicroseconds = ((double) it->ReadLong()) / 1e6;
  (void) sensorTimestampMicroseconds;
  int32_t const index = it->ReadInteger();
  int32_t const componentCount = it->ReadInteger();

  // if (m_start_time == -1) {
  //   model::Timestamp now;
  //   m_start_time = now.GetTime() - sensorTimestamp;
  // }

  // model::Timestamp timestamp(m_start_time + sensorTimestamp);

  // if (componentCount != 1) {
  //   std::cout 
  //       << "Unexpected answer from QTM RT server: More than one component."
  //       << " Got: " << componentCount
  //       << " Expecting: 1"
  //       << std::endl;
  //   return;
  // }

  int32_t const componentSize = it->ReadInteger();
  int32_t const componentType = it->ReadInteger();
  int32_t const markerCount = it->ReadInteger();
  int16_t const qualityDrop = it->ReadShort();
  int16_t const qualitySync = it->ReadShort();
  (void) componentSize;

  float const quality = ((float) (qualityDrop + qualitySync)) / 2000.0f;

  if (componentType != 2) {
    std::cout 
        << "Unexpected answer from QTM RT server: Unrecognized component type."
        << " Got: " << componentType
        << " Expecting: 2" 
        << std::endl;
  }

  std::vector<opendlv::model::Cartesian3> markers;
  for (int32_t j = 0; j < markerCount; j++)
  {
    float const x = it->ReadFloat() / 1000.0f;
    float const y = it->ReadFloat() / 1000.0f;
    float const z = it->ReadFloat() / 1000.0f;
    int32_t const id = it->ReadInteger();
    (void) id;
    opendlv::model::Cartesian3 marker(x,y,z);
    markers.push_back(marker);
  }
  odcore::data::TimeStamp now;
  opendlv::proxy::QtmFrame frame(markers, now, quality, index);
  odcore::data::Container c(frame);
  m_conference.send(c);
}

}
}
}
