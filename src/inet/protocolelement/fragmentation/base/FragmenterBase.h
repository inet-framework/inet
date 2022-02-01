//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FRAGMENTERBASE_H
#define __INET_FRAGMENTERBASE_H

#include "inet/protocolelement/fragmentation/contract/IFragmenterPolicy.h"
#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API FragmenterBase : public PacketPusherBase
{
  protected:
    bool deleteSelf;
    IFragmenterPolicy *fragmenterPolicy = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual IFragmenterPolicy *createFragmenterPolicy(const char *fragmenterPolicyClass) const;
    virtual Packet *createFragmentPacket(Packet *packet, b fragmentOffset, b fragmentLength, int fragmentNumber, int numFragments);

  public:
    virtual void pushPacket(Packet *packet, cGate *gate) override;
};

} // namespace inet

#endif

