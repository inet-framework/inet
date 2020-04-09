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

#ifndef __INET_FCSCHECKERBASE_H
#define __INET_FCSCHECKERBASE_H

#include "inet/linklayer/common/FcsMode_m.h"
#include "inet/queueing/base/PacketFilterBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API FcsCheckerBase : public PacketFilterBase
{
  protected:
    virtual bool checkDisabledFcs(const Packet *packet, uint32_t fcs) const;
    virtual bool checkDeclaredCorrectFcs(const Packet *packet, uint32_t fcs) const;
    virtual bool checkDeclaredIncorrectFcs(const Packet *packet, uint32_t fcs) const;
    virtual bool checkComputedFcs(const Packet *packet, uint32_t fcs) const;
    virtual bool checkFcs(const Packet *packet, FcsMode fcsMode, uint32_t fcs) const;
};

} // namespace inet

#endif // ifndef __INET_FCSCHECKERBASE_H

