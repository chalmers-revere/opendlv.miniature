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

#include <opendavinci/odcore/base/module/DataTriggeredConferenceClientModule.h>

#include <odvdopendlvdata/GeneratedHeaders_ODVDOpenDLVData.h>

namespace opendlv {
namespace proxy {
namespace miniature {

class Lps : public odcore::base::module::DataTriggeredConferenceClientModule {
   public:
    Lps(int32_t const &, char **);
    Lps(Lps const &) = delete;
    Lps &operator=(Lps const &) = delete;
    virtual ~Lps();

   private:
    virtual void setUp();
    virtual void tearDown();
    virtual void nextContainer(odcore::data::Container &);
    

    void AnalyseNeedle(std::vector<opendlv::model::Cartesian3>);
    void Search(std::vector<opendlv::model::Cartesian3>);
    void FindState(std::vector<opendlv::model::Cartesian3>);

    std::vector<float> m_needleMarkerDistances;
    float m_needleNormRoll;
    float m_needleNormPitch;
    float m_needleNormYaw;
    float m_searchMarginHalf;
    int16_t m_frameId;
    bool m_debug;

};

} 
}
}

#endif
