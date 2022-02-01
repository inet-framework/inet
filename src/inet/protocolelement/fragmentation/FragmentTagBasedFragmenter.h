//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FRAGMENTTAGBASEDFRAGMENTER_H
#define __INET_FRAGMENTTAGBASEDFRAGMENTER_H

#include "inet/protocolelement/common/HeaderPosition.h"
#include "inet/protocolelement/fragmentation/base/FragmenterBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API FragmentTagBasedFragmenter : public FragmenterBase
{
  protected:
    Packet *createFragmentPacket(Packet *packet, b fragmentOffset, b fragmentLength, int fragmentNumber, int numFragments) override;
};

} // namespace inet

#endif

