//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

  public:
    virtual ~DefragmenterBase();
};

} // namespace inet

#endif

