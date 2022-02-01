//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211OFDMENCODERMODULE_H
#define __INET_IEEE80211OFDMENCODERMODULE_H

#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmEncoder.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211OfdmEncoderModule : public IEncoder, public cSimpleModule
{
  protected:
    const Ieee80211OfdmEncoder *encoder = nullptr;
    const IScrambler *scrambler = nullptr;
    const IFecCoder *convolutionalCoder = nullptr;
    const IInterleaver *interleaver = nullptr;
    const Ieee80211OfdmCode *code = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override { throw cRuntimeError("This module doesn't handle self messages"); }

  public:
    virtual ~Ieee80211OfdmEncoderModule();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override { return encoder->printToStream(stream, level); }
    const Ieee80211OfdmCode *getCode() const override { return code; }
    virtual const ITransmissionBitModel *encode(const ITransmissionPacketModel *packetModel) const override;
};
} /* namespace physicallayer */
} /* namespace inet */

#endif

