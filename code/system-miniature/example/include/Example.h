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

#ifndef SYSTEM_MINIATURE_EXAMPLE_H
#define SYSTEM_MINIATURE_EXAMPLE_H


#include <memory>

#include <opendavinci/odcore/base/module/TimeTriggeredConferenceClientModule.h>

namespace opendlv {
namespace system {
namespace miniature {

class Example : public odcore::base::module::TimeTriggeredConferenceClientModule {
 public:
  Example(const int &, char **);
  Example(const Example &) = delete;
  Example &operator=(const Example &) = delete;
  virtual ~Example();
  virtual void nextContainer(odcore::data::Container &);

 private:
  void setUp();
  void tearDown();
  virtual odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode body();

  std::vector<uint16_t> m_digitalPins;
};

}
}
}

#endif
