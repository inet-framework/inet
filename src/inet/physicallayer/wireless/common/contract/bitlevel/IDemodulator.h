//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IDEMODULATOR_H
#define __INET_IDEMODULATOR_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalBitModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalSymbolModel.h"

namespace inet {

namespace physicallayer {

class INET_API IDemodulator : public IPrintableObject
{
  public:
    virtual const IReceptionBitModel *demodulate(const IReceptionSymbolModel *symbolModel) const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif

