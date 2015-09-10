//
// Copyright (C) 2006 Andras Babos and Andras Varga
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_OSPFNEIGHBORSTATEEXCHANGESTART_H
#define __INET_OSPFNEIGHBORSTATEEXCHANGESTART_H

#include "inet/routing/ospfv2/neighbor/OSPFNeighborState.h"

namespace inet {

namespace ospf {

class NeighborStateExchangeStart : public NeighborState
{
  public:
    virtual void processEvent(Neighbor *neighbor, Neighbor::NeighborEventType event) override;
    virtual Neighbor::NeighborStateType getState() const override { return Neighbor::EXCHANGE_START_STATE; }
};

} // namespace ospf

} // namespace inet

#endif // ifndef __INET_OSPFNEIGHBORSTATEEXCHANGESTART_H

