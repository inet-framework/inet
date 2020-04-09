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

#ifndef __INET_CRCCHECKERBASE_H
#define __INET_CRCCHECKERBASE_H

#include "inet/queueing/base/PacketFilterBase.h"
#include "inet/transportlayer/common/CrcMode_m.h"

namespace inet {

using namespace inet::queueing;

class INET_API CrcCheckerBase : public PacketFilterBase
{
  protected:
    virtual bool checkDisabledCrc(const Packet *packet, uint16_t crc) const;
    virtual bool checkDeclaredCorrectCrc(const Packet *packet, uint16_t crc) const;
    virtual bool checkDeclaredIncorrectCrc(const Packet *packet, uint16_t crc) const;
    virtual bool checkComputedCrc(const Packet *packet, uint16_t crc) const;
    virtual bool checkCrc(const Packet *packet, CrcMode crcMode, uint16_t crc) const;
};

} // namespace inet

#endif // ifndef __INET_CRCCHECKERBASE_H

