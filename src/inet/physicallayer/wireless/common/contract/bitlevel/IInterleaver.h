//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IINTERLEAVER_H
#define __INET_IINTERLEAVER_H

#include "inet/common/BitVector.h"
#include "inet/common/IPrintableObject.h"

namespace inet {
namespace physicallayer {

class INET_API IInterleaving : public IPrintableObject
{
};

class INET_API IInterleaver : public IPrintableObject
{
  public:
    virtual BitVector interleave(const BitVector& bits) const = 0;
    virtual BitVector deinterleave(const BitVector& bits) const = 0;
    virtual const IInterleaving *getInterleaving() const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif

