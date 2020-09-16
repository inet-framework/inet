//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_SEQUENCENUMBERPACKETCLASSIFIERFUNCTION_H
#define __INET_SEQUENCENUMBERPACKETCLASSIFIERFUNCTION_H

#include "inet/queueing/contract/IPacketClassifierFunction.h"

namespace inet {

using namespace inet::queueing;

class INET_API SequenceNumberPacketClassifierFunction : public cObject, public IPacketClassifierFunction
{
  protected:
    virtual int classifyPacket(Packet *packet) const override;
};

} // namespace inet

#endif

