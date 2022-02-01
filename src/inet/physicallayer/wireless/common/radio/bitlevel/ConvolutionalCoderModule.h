//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CONVOLUTIONALCODERMODULE_H
#define __INET_CONVOLUTIONALCODERMODULE_H

#include "inet/physicallayer/wireless/common/radio/bitlevel/ConvolutionalCoder.h"

namespace inet {
namespace physicallayer {

class INET_API ConvolutionalCoderModule : public cSimpleModule, public IFecCoder
{
  protected:
    ConvolutionalCoder *convolutionalCoder = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override { throw cRuntimeError("This module doesn't handle self messages"); }

  public:
    ~ConvolutionalCoderModule();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual BitVector encode(const BitVector& informationBits) const override { return convolutionalCoder->encode(informationBits); }
    virtual std::pair<BitVector, bool> decode(const BitVector& encodedBits) const override { return convolutionalCoder->decode(encodedBits); }
    virtual const ConvolutionalCode *getForwardErrorCorrection() const override { return convolutionalCoder->getForwardErrorCorrection(); }
};

} /* namespace physicallayer */
} /* namespace inet */

#endif

