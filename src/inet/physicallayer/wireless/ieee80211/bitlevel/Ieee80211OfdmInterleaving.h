//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211OFDMINTERLEAVING_H
#define __INET_IEEE80211OFDMINTERLEAVING_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/IInterleaver.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211OfdmInterleaving : public IInterleaving
{
  protected:
    int numberOfCodedBitsPerSymbol;
    int numberOfCodedBitsPerSubcarrier;

  public:
    Ieee80211OfdmInterleaving(int numberOfCodedBitsPerSymbol, int numberOfCodedBitsPerSubcarrier);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    int getNumberOfCodedBitsPerSubcarrier() const { return numberOfCodedBitsPerSubcarrier; }
    int getNumberOfCodedBitsPerSymbol() const { return numberOfCodedBitsPerSymbol; }
};
} /* namespace physicallayer */
} /* namespace inet */

#endif

