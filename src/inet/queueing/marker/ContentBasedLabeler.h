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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_CONTENTBASEDLABELER_H
#define __INET_CONTENTBASEDLABELER_H

#include "inet/common/packet/PacketFilter.h"
#include "inet/queueing/base/PacketLabelerBase.h"

namespace inet {
namespace queueing {

class INET_API ContentBasedLabeler : public PacketLabelerBase
{
  protected:
    std::vector<PacketFilter *> filters;

  protected:
    virtual void initialize(int stage) override;
    virtual void markPacket(Packet *packet) override;

  public:
    virtual ~ContentBasedLabeler();
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_CONTENTBASEDLABELER_H

