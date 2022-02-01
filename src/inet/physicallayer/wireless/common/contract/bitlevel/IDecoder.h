//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IDECODER_H
#define __INET_IDECODER_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalBitModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalPacketModel.h"

namespace inet {

namespace physicallayer {

class INET_API IDecoder : public IPrintableObject
{
  public:
    virtual const IReceptionPacketModel *decode(const IReceptionBitModel *bitModel) const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif

