//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_APSKCODE_H
#define __INET_APSKCODE_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/ICode.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IInterleaver.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IScrambler.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/ConvolutionalCode.h"

namespace inet {

namespace physicallayer {

class INET_API ApskCode : public ICode
{
  protected:
    const ConvolutionalCode *convolutionalCode;
    const IInterleaving *interleaving;
    const IScrambling *scrambling;

  public:
    ApskCode(const ConvolutionalCode *convCode, const IInterleaving *interleaving, const IScrambling *scrambling);
    ~ApskCode();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    const ConvolutionalCode *getConvolutionalCode() const { return convolutionalCode; }
    const IInterleaving *getInterleaving() const { return interleaving; }
    const IScrambling *getScrambling() const { return scrambling; }
};
} /* namespace physicallayer */
} /* namespace inet */

#endif

