//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_IEEE80211OFDMSYMBOL_H
#define __INET_IEEE80211OFDMSYMBOL_H

#include "inet/physicallayer/apskradio/bitlevel/ApskSymbol.h"
#include "inet/physicallayer/contract/bitlevel/ISymbol.h"

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

#endif // ifndef __INET_IEEE80211OFDMSYMBOL_H

