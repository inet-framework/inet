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

#ifndef __INET_DEFRAGMENTERBASE_H
#define __INET_DEFRAGMENTERBASE_H

#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API DefragmenterBase : public PacketPusherBase
{
  protected:
    bool deleteSelf = false;

    int expectedFragmentNumber = 0;
    Packet *defragmentedPacket = nullptr;

  protected:
    virtual void initialize(int stage) override;

    virtual bool isDefragmenting() { return defragmentedPacket != nullptr; }
    virtual void startDefragmentation(Packet *fragmentPacket);
    virtual void continueDefragmentation(Packet *fragmentPacket);
    virtual void endDefragmentation(Packet *fragmentPacket);

    virtual void defragmentPacket(Packet *fragmentPacket, bool firstFragment, bool lastFragment, bool expectedFragment);
};

} // namespace inet

#endif // ifndef __INET_DEFRAGMENTERBASE_H

