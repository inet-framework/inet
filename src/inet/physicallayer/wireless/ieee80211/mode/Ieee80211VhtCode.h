//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211VHTCODE_H
#define __INET_IEEE80211VHTCODE_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/ICode.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/AdditiveScrambling.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211ConvolutionalCode.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211VhtInterleaving.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmModulation.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211VhtCode : public ICode
{
  protected:
    const Ieee80211ConvolutionalCode *forwardErrorCorrection;
    const Ieee80211VhtInterleaving *interleaving;
    const AdditiveScrambling *scrambling;

  public:
    Ieee80211VhtCode(const Ieee80211ConvolutionalCode *forwardErrorCorrection, const Ieee80211VhtInterleaving *interleaving, const AdditiveScrambling *scrambling);

    const Ieee80211ConvolutionalCode *getForwardErrorCorrection() const { return forwardErrorCorrection; }
    const AdditiveScrambling *getScrambling() const { return scrambling; }
    const Ieee80211VhtInterleaving *getInterleaving() const { return interleaving; }

    virtual ~Ieee80211VhtCode();
};

class INET_API Ieee80211VhtCompliantCodes
{
  public:
    // Convolutional codes (TODO LDPC codes).
    static const Ieee80211VhtCode *getCompliantCode(const Ieee80211ConvolutionalCode *convolutionalCode, const Ieee80211OfdmModulation *stream1Modulation, const Ieee80211OfdmModulation *stream2Modulation, const Ieee80211OfdmModulation *stream3Modulation, const Ieee80211OfdmModulation *stream4Modulation, const Ieee80211OfdmModulation *stream5Modulation, const Ieee80211OfdmModulation *stream6Modulation, const Ieee80211OfdmModulation *stream7Modulation, const Ieee80211OfdmModulation *stream8Modulation, Hz bandwidth, bool withScrambling = true);
};

} /* namespace physicallayer */
} /* namespace inet */

#endif

