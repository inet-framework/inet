//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LENGTHBASEDFRAGMENTERPOLICY_H
#define __INET_LENGTHBASEDFRAGMENTERPOLICY_H

#include "inet/protocolelement/fragmentation/contract/IFragmenterPolicy.h"

namespace inet {

class INET_API LengthBasedFragmenterPolicy : public cSimpleModule, public IFragmenterPolicy
{
  protected:
    b minFragmentLength = b(-1);
    b maxFragmentLength = b(-1);
    b roundingLength = b(-1);
    b fragmentHeaderLength = b(-1);

  protected:
    virtual void initialize(int stage) override;

    virtual std::vector<b> computeFragmentLengths(Packet *packet) const override;
};

} // namespace inet

#endif

