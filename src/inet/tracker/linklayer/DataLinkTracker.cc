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

#include "inet/tracker/linklayer/DataLinkTracker.h"

#include "inet/linklayer/base/MacProtocolBase.h"

#ifdef INET_WITH_IEEE80211
#include "inet/linklayer/ieee80211/mac/contract/ICoordinationFunction.h"
#endif // WITH_IEEE80211

namespace inet {

namespace tracker {

Define_Module(DataLinkTracker);

bool DataLinkTracker::isLinkStart(cModule *module) const
{
    std::cout << "START: " << module->getFullPath() << std::endl;
    return dynamic_cast<MacProtocolBase *>(module) != nullptr
#ifdef INET_WITH_IEEE80211
           || dynamic_cast<ieee80211::ICoordinationFunction *>(module) != nullptr
#endif // WITH_IEEE80211
        ;
}

bool DataLinkTracker::isLinkEnd(cModule *module) const
{
    std::cout << "END: " << module->getFullPath() << std::endl;
    return dynamic_cast<MacProtocolBase *>(module) != nullptr
#ifdef INET_WITH_IEEE80211
           || dynamic_cast<ieee80211::ICoordinationFunction *>(module) != nullptr
#endif // WITH_IEEE80211
        ;
}

} // namespace tracker

} // namespace inet

