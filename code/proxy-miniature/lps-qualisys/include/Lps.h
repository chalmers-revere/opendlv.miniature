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


#ifndef PROXY_MINIATURE_LPS_H
#define PROXY_MINIATURE_LPS_H

#include <memory>
#include <vector>

#include <odvdopendlvdata/GeneratedHeaders_ODVDOpenDLVData.h>

namespace opendlv {
namespace proxy {
namespace miniature {

/**
 * @brief A handler for marker-based local positioning systems. The
 * positioning is made in this class, but the frames are captured
 * elsewhere.
 */
class Lps {
  public:
    /**
     * @brief A marker representation.
     */
    struct Marker
    {
      float x;
      float y;
      float z;

      Marker(float, float, float);
    };

    Lps(std::vector<Marker>, int16_t, float);
    Lps(Lps const &) = delete;
    Lps &operator=(Lps const &) = delete;
    virtual ~Lps();
    std::shared_ptr<opendlv::model::State> Search(std::vector<Marker>);

  private:
    std::shared_ptr<opendlv::model::State> CalcState(std::vector<Marker>);

    std::vector<float> m_needleMarkerDistances;
    float m_needleNormRoll;
    float m_needleNormPitch;
    float m_needleNormYaw;
    float m_searchMarginHalf;
    int16_t m_frameId;
};

}
}
}

#endif
