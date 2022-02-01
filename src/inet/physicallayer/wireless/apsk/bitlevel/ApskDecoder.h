//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_APSKDECODER_H
#define __INET_APSKDECODER_H

#include "inet/physicallayer/wireless/apsk/bitlevel/ApskCode.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IDecoder.h"

namespace inet {

namespace physicallayer {

class INET_API ApskDecoder : public cSimpleModule, public IDecoder
{
  protected:
    const ApskCode *code;
    const IScrambler *descrambler;
    const IFecCoder *fecDecoder;
    const IInterleaver *deinterleaver;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

  public:
    ApskDecoder();
    virtual ~ApskDecoder();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual const ApskCode *getCode() const { return code; }
    virtual const IReceptionPacketModel *decode(const IReceptionBitModel *bitModel) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

