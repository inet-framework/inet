//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FRAGMENTNUMBERHEADERBASEDFRAGMENTER_H
#define __INET_FRAGMENTNUMBERHEADERBASEDFRAGMENTER_H

#include "inet/protocolelement/common/HeaderPosition.h"
#include "inet/protocolelement/fragmentation/base/FragmenterBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API FragmentNumberHeaderBasedFragmenter : public FragmenterBase
{
  protected:
    HeaderPosition headerPosition = HP_UNDEFINED;

  protected:
    virtual void initialize(int stage) override;

    virtual Packet *createFragmentPacket(Packet *packet, b fragmentOffset, b fragmentLength, int fragmentNumber, int numFragments) override;
};

} // namespace inet

#endif

