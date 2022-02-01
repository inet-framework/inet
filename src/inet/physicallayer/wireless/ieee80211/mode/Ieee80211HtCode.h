//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211HTCODE_H
#define __INET_IEEE80211HTCODE_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/ICode.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/AdditiveScrambling.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211ConvolutionalCode.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211HtInterleaving.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmModulation.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211HtCode : public ICode
{
  protected:
    const Ieee80211ConvolutionalCode *forwardErrorCorrection;
    const Ieee80211HtInterleaving *interleaving;
    const AdditiveScrambling *scrambling;

  public:
    Ieee80211HtCode(const Ieee80211ConvolutionalCode *forwardErrorCorrection, const Ieee80211HtInterleaving *interleaving, const AdditiveScrambling *scrambling);

    const Ieee80211ConvolutionalCode *getForwardErrorCorrection() const { return forwardErrorCorrection; }
    const AdditiveScrambling *getScrambling() const { return scrambling; }
    const Ieee80211HtInterleaving *getInterleaving() const { return interleaving; }

    virtual ~Ieee80211HtCode();
};

class INET_API Ieee80211HtCompliantCodes
{
  public:
    // Convolutional codes (TODO LDPC codes).
    // Note: 1/2, 2/3, 3/4 rates are defined in Ieee80211OfdmCompliantCodes.
    static const Ieee80211ConvolutionalCode htConvolutionalCode5_6;

    static const Ieee80211HtCode *getCompliantCode(const Ieee80211ConvolutionalCode *convolutionalCode, const Ieee80211OfdmModulation *stream1Modulation, const Ieee80211OfdmModulation *stream2Modulation, const Ieee80211OfdmModulation *stream3Modulation, const Ieee80211OfdmModulation *stream4Modulation, Hz bandwidth, bool withScrambling = true);
};

} /* namespace physicallayer */
} /* namespace inet */

#endif

