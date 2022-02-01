//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211OFDMINTERLEAVER_H
#define __INET_IEEE80211OFDMINTERLEAVER_H

#include "inet/common/BitVector.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IInterleaver.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmInterleaving.h"

namespace inet {
namespace physicallayer {

/*
 * It is a IEEE 802.11 OFDM block interleaver/deinterleaver implementation.
 * The permutation equations and all the details can be found in:
 * Part 11: Wireless LAN Medium Access Control (MAC) and Physical Layer (PHY) Specifications,
 * 18.3.5.7 Data interleaving
 */
class INET_API Ieee80211OfdmInterleaver : public IInterleaver
{
  protected:
    int numberOfCodedBitsPerSymbol;
    int numberOfCodedBitsPerSubcarrier;
    int s;
    const Ieee80211OfdmInterleaving *interleaving = nullptr;

  public:
    Ieee80211OfdmInterleaver(const Ieee80211OfdmInterleaving *interleaving);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    BitVector interleave(const BitVector& bits) const override;
    BitVector deinterleave(const BitVector& bits) const override;
    int getNumberOfCodedBitsPerSymbol() const { return numberOfCodedBitsPerSymbol; }
    int getNumberOfCodedBitsPerSubcarrier() const { return numberOfCodedBitsPerSubcarrier; }
    const Ieee80211OfdmInterleaving *getInterleaving() const override { return interleaving; }
};

} /* namespace physicallayer */
} /* namespace inet */

#endif

