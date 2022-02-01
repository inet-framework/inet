//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IMODULATOR_H
#define __INET_IMODULATOR_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalBitModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalSymbolModel.h"

namespace inet {

namespace physicallayer {

class INET_API IModulator : public IPrintableObject
{
  public:
    virtual const IModulation *getModulation() const = 0;
    virtual const ITransmissionSymbolModel *modulate(const ITransmissionBitModel *bitModel) const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif

