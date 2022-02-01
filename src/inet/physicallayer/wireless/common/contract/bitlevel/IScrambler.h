//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ISCRAMBLER_H
#define __INET_ISCRAMBLER_H

#include "inet/common/BitVector.h"
#include "inet/common/IPrintableObject.h"
#include "inet/common/ShortBitVector.h"

namespace inet {
namespace physicallayer {

class INET_API IScrambling : public IPrintableObject
{
  public:
    const ShortBitVector& getGeneratorPolynomial() const;
    const ShortBitVector& getSeed() const;
};

class INET_API IScrambler : public IPrintableObject
{
  public:
    virtual BitVector scramble(const BitVector& bits) const = 0;
    virtual BitVector descramble(const BitVector& bits) const = 0;
    virtual const IScrambling *getScrambling() const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif

