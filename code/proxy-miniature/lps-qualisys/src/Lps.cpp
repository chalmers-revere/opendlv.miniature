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

#include <algorithm>
#include <cmath>
#include <limits>
#include <iostream>

#include "Lps.h"

namespace opendlv {
namespace proxy {
namespace miniature {

Lps::Marker::Marker(float a_x, float a_y, float a_z):
    x(a_x),
    y(a_y),
    z(a_z)
{
}

Lps::Lps(std::vector<Marker> a_needleMarkers, int16_t a_frameId, 
    float a_searchMargin):
    m_needleMarkerDistances(),
    m_needleNormRoll(),
    m_needleNormPitch(),
    m_needleNormYaw(),
    m_searchMarginHalf(0.5f * a_searchMargin),
    m_frameId(a_frameId)
{
  uint8_t const markerCount = a_needleMarkers.size();

  float rollTotal = 0.0f;
  float pitchTotal = 0.0f;
  float yawTotal = 0.0f;

  for (auto marker : a_needleMarkers) {
    float x = marker.x;
    float y = marker.y;
    float z = marker.z;
    float distance = sqrt(x*x + y*y + z*z);

    m_needleMarkerDistances.push_back(distance);

    float roll = atan2(y, z);
    float pitch = atan2(z, x);
    float yaw = atan2(y, x);

    rollTotal += roll;
    pitchTotal += pitch;
    yawTotal += yaw;
  }

  m_needleNormRoll = rollTotal / markerCount;
  m_needleNormPitch = pitchTotal / markerCount;
  m_needleNormYaw = yawTotal / markerCount;
}

Lps::~Lps()
{
}

std::shared_ptr<opendlv::model::State> Lps::Search(
    std::vector<Marker> a_haystackMarkers)
{
  uint32_t const haystackMarkerCount = a_haystackMarkers.size();

//  std::cout << "==== SEARCH" << std::endl;

  for (uint32_t i = 0; i < haystackMarkerCount; i++) {

//    std::cout << "I THINK ORIGO IS: " << i << std::endl;

    Marker origoCandidate = a_haystackMarkers[i];

    uint32_t const needleMarkerCount = m_needleMarkerDistances.size();
    std::vector<float> foundDistances(needleMarkerCount);
    std::vector<uint32_t> foundIndices(needleMarkerCount);
    for (uint32_t j = 0; j < needleMarkerCount; j++) {
      foundDistances[j] = std::numeric_limits<float>::max();
      foundIndices[j] = -1;
    }

    for (uint32_t j = 0; j < needleMarkerCount; j++) {
      float searchedDistance = m_needleMarkerDistances[j];

//      std::cout << " SEARCH FOR DISTANCE: " << searchedDistance << std::endl;

      for (uint32_t k = 0; k < haystackMarkerCount; k++) {
        if (k == i) {
          continue;
        }

        Marker candidate = a_haystackMarkers[k];
        
        float dx = candidate.x - origoCandidate.x;
        float dy = candidate.y - origoCandidate.y;
        float dz = candidate.z - origoCandidate.z;
      
        float distance = sqrt(dx*dx + dy*dy + dz*dz);

   //     std::cout << "  IS IT: " << dx << " " << dy << " = " << distance << "?" << std::endl;

        if (searchedDistance > distance - m_searchMarginHalf &&
            searchedDistance < distance + m_searchMarginHalf)
        {
          float prevFoundDistance = foundDistances[j];
          float currentError = std::abs(distance - searchedDistance);
          float prevError = std::abs(prevFoundDistance - searchedDistance);

          if (currentError < prevError) {
            foundDistances[j] = distance;
            foundIndices[j] = k;
//            std::cout << "   YES, " << searchedDistance << " == " << distance << std::endl;
          }
        }
      }

      std::vector<Marker> needleMarkers;
      needleMarkers.push_back(origoCandidate);

      bool isNeedleFound = true;
      for (int index : foundIndices) {
        if (index == -1) {
          isNeedleFound = false;
          break;
        }
        Marker candidate = a_haystackMarkers[index];
        needleMarkers.push_back(candidate);
      }

      // TODO: Check that we don't use the same node twice..

      if (isNeedleFound) {
        std::cout << "    FOUND!!!" << std::endl;
        return CalcState(needleMarkers);
      }
    }
  }

//  std::cout << "    NOT FOUND!!!" << std::endl;
  return nullptr;
}

std::shared_ptr<opendlv::model::State> Lps::CalcState(
    std::vector<Marker> a_needleMarkers)
{
//  std::cout << "== OBJECT " << a_scene_object->GetName() << std::endl;

  Marker origo = a_needleMarkers[0];
  float const x0 = origo.x;
  float const y0 = origo.y;
  float const z0 = origo.z;
    
//  std::cout << "  origo " << x << " " << y << std::endl;

  uint32_t const haystackMarkerCount = a_needleMarkers.size();

  bool doFlip = true;

  float rollTotal = 0.0f;
  float pitchTotal = 0.0f;
  float yawTotal = 0.0f;

  for (uint32_t i = 1; i < haystackMarkerCount; i++) {
    Marker marker = a_needleMarkers[i];
    
    float dx = marker.x - x0;
    float dy = marker.y - y0;
    float dz = marker.z - z0;

    if (dx > 0.0f) {
      doFlip = false;
    }

    float roll = atan2(dy, dz);
    float pitch = atan2(dz, dx);
    float yaw = atan2(dy, dx);
    
 //   std::cout << "  node " << i << " x " << marker->x << " y " << marker->y << " dx "
 //       << dx << " dy " << dy << " yaw " << yaw << std::endl;

    rollTotal += roll;
    pitchTotal += pitch;
    yawTotal += yaw;
  }

  float const rollMean = yawTotal / (haystackMarkerCount - 1);
  float const pitchMean = yawTotal / (haystackMarkerCount - 1);
  float const yawMean = yawTotal / (haystackMarkerCount - 1);
  
  float const roll = rollMean - m_needleNormRoll;
  float const pitch = pitchMean - m_needleNormPitch;
  float yaw = yawMean - m_needleNormYaw;

  if (doFlip) {
    yaw += 3.14f;
  }

//  std::cout << "  yaw norm " << yaw_norm << " total yaw " << yaw_tot << " yaw mean " << yaw_mean << " final yaw " << yaw << std::endl;

  opendlv::model::Cartesian3 position(x0, y0, z0);
  opendlv::model::Cartesian3 angularDisplacement(roll, pitch, yaw);
  std::shared_ptr<opendlv::model::State> state(
      new opendlv::model::State(position, angularDisplacement, m_frameId));

  return state;
}

}
}
}
