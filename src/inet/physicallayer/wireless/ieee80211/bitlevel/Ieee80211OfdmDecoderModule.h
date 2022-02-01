//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211OFDMDECODERMODULE_H
#define __INET_IEEE80211OFDMDECODERMODULE_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/IDecoder.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/AdditiveScrambler.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/ConvolutionalCoder.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmDecoder.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmInterleaver.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmInterleaving.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211OfdmDecoderModule : public cSimpleModule, public IDecoder
{
  protected:
    const Ieee80211OfdmDecoder *ofdmDecoder = nullptr;
    const IScrambler *descrambler = nullptr;
    const IFecCoder *convolutionalDecoder = nullptr;
    const IInterleaver *deinterleaver = nullptr;
    const Ieee80211OfdmCode *code = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override { throw cRuntimeError("This module doesn't handle self messages"); }

  public:
    virtual ~Ieee80211OfdmDecoderModule();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    const Ieee80211OfdmCode *getCode() const { return code; }
    const IReceptionPacketModel *decode(const IReceptionBitModel *bitModel) const override;
};

} /* namespace physicallayer */
} /* namespace inet */

#endif

