//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211OFDMSYMBOL_H
#define __INET_IEEE80211OFDMSYMBOL_H

#include "inet/physicallayer/wireless/apsk/bitlevel/ApskSymbol.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISymbol.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211OfdmSymbol : public ISymbol
{
  protected:
    std::vector<const ApskSymbol *> subcarrierSymbols;

  public:
    friend std::ostream& operator<<(std::ostream& out, const Ieee80211OfdmSymbol& symbol);
    Ieee80211OfdmSymbol(const std::vector<const ApskSymbol *>& subcarrierSymbols) : subcarrierSymbols(subcarrierSymbols) {}
    Ieee80211OfdmSymbol() { subcarrierSymbols.resize(53, nullptr); } // (48 + 4 + 1), but one of them is skipped.
    const std::vector<const ApskSymbol *>& getSubCarrierSymbols() const { return subcarrierSymbols; }
    int symbolSize() const { return subcarrierSymbols.size(); }
    void pushApskSymbol(const ApskSymbol *apskSymbol, int subcarrierIndex);
    void clearSymbols() { subcarrierSymbols.resize(53, nullptr); }
};
} /* namespace physicallayer */
} /* namespace inet */

#endif

