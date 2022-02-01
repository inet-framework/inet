//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IENCODER_H
#define __INET_IENCODER_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/ICode.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalBitModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalPacketModel.h"

namespace inet {
namespace physicallayer {

class INET_API IEncoder : public IPrintableObject
{
  public:
    virtual const ITransmissionBitModel *encode(const ITransmissionPacketModel *packetModel) const = 0;
    virtual const ICode *getCode() const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif

