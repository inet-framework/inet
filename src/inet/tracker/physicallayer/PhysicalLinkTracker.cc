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

#include "inet/tracker/physicallayer/PhysicalLinkTracker.h"

#include "inet/linklayer/ethernet/base/EthernetMacBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"

namespace inet {

namespace tracker {

Define_Module(PhysicalLinkTracker);

bool PhysicalLinkTracker::isLinkStart(cModule *module) const
{
    return dynamic_cast<inet::physicallayer::IRadio *>(module) != nullptr ||
           dynamic_cast<EthernetMacBase *>(module); // NOTE: this module also serves as a PHY
}

bool PhysicalLinkTracker::isLinkEnd(cModule *module) const
{
    return dynamic_cast<inet::physicallayer::IRadio *>(module) != nullptr ||
           dynamic_cast<EthernetMacBase *>(module); // NOTE: this module also serves as a PHY
}

} // namespace tracker

} // namespace inet

