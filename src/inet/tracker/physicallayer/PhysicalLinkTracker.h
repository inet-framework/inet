//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifndef __INET_PHYSICALLINKTRACKER_H
#define __INET_PHYSICALLINKTRACKER_H

#include "inet/tracker/base/LinkTrackerBase.h"

namespace inet {

namespace tracker {

class INET_API PhysicalLinkTracker : public LinkTrackerBase
{
  protected:
    virtual bool isLinkStart(cModule *module) const override;
    virtual bool isLinkEnd(cModule *module) const override;
    virtual const char *getTags() const override {
        switch (activityLevel) {
            case ACTIVITY_LEVEL_SERVICE: return "physical_layer service_level";
            case ACTIVITY_LEVEL_PEER: return "physical_layer peer_level";
            case ACTIVITY_LEVEL_PROTOCOL: return "physical_layer protocol_level";
            default: throw cRuntimeError("Unknown activity level");
        }
    }
};

} // namespace tracker

} // namespace inet

#endif

