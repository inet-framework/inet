//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FRAGMENTTAGBASEDDEFRAGMENTER_H
#define __INET_FRAGMENTTAGBASEDDEFRAGMENTER_H

#include "inet/protocolelement/fragmentation/base/DefragmenterBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API FragmentTagBasedDefragmenter : public DefragmenterBase
{
  public:
    virtual void pushPacket(Packet *fragmentPacket, cGate *gate) override;
};

} // namespace inet

#endif

