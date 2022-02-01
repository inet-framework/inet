//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FRAGMENTNUMBERHEADERBASEDDEFRAGMENTER_H
#define __INET_FRAGMENTNUMBERHEADERBASEDDEFRAGMENTER_H

#include "inet/protocolelement/common/HeaderPosition.h"
#include "inet/protocolelement/fragmentation/base/DefragmenterBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API FragmentNumberHeaderBasedDefragmenter : public DefragmenterBase
{
  protected:
    HeaderPosition headerPosition = HP_UNDEFINED;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual void pushPacket(Packet *fragmentPacket, cGate *gate) override;
};

} // namespace inet

#endif

