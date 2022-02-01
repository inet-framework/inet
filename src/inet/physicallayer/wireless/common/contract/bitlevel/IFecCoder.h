//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IFECCODER_H
#define __INET_IFECCODER_H

#include "inet/common/BitVector.h"
#include "inet/common/IPrintableObject.h"

namespace inet {
namespace physicallayer {

class INET_API IForwardErrorCorrection : public IPrintableObject
{
  public:
    virtual double getCodeRate() const = 0;
    virtual int getEncodedLength(int decodedLength) const = 0;
    virtual int getDecodedLength(int encodedLength) const = 0;
    virtual double computeNetBitErrorRate(double grossBitErrorRate) const = 0;
};

class INET_API IFecCoder : public IPrintableObject
{
  public:
    virtual BitVector encode(const BitVector& informationBits) const = 0;
    virtual std::pair<BitVector, bool> decode(const BitVector& encodedBits) const = 0;
    virtual const IForwardErrorCorrection *getForwardErrorCorrection() const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif

