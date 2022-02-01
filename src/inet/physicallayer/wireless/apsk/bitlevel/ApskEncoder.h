//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_APSKENCODER_H
#define __INET_APSKENCODER_H

#include "inet/physicallayer/wireless/apsk/bitlevel/ApskCode.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IEncoder.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IFecCoder.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IInterleaver.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IScrambler.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalBitModel.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalPacketModel.h"

namespace inet {

namespace physicallayer {

class INET_API ApskEncoder : public IEncoder, public cSimpleModule
{
  protected:
    const ApskCode *code;
    const IScrambler *scrambler;
    const IFecCoder *fecEncoder;
    const IInterleaver *interleaver;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

  public:
    ApskEncoder();
    virtual ~ApskEncoder();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual const ApskCode *getCode() const override { return code; }
    virtual const ITransmissionBitModel *encode(const ITransmissionPacketModel *packetModel) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

