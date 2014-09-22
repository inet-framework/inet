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

#ifndef __INET_OFDMSYMBOL_H_
#define __INET_OFDMSYMBOL_H_

#include "ISymbol.h"
#include "APSKSymbol.h"

namespace inet {
namespace physicallayer {
// TODO: Revise name: It is Ieee80211 specific implementation (uses fixed size vector). Ieee80211OFDMSymbol?
class INET_API OFDMSymbol : public ISymbol
{
    protected:
        std::vector<const APSKSymbol *> subcarrierSymbols;

    public:
        friend std::ostream& operator<<(std::ostream& out, const OFDMSymbol& symbol);
        OFDMSymbol(const std::vector<const APSKSymbol *>& subcarrierSymbols) : subcarrierSymbols(subcarrierSymbols) {}
        OFDMSymbol() { subcarrierSymbols.resize(64); }
        const std::vector<const APSKSymbol *>& getSubCarrierSymbols() const { return subcarrierSymbols; }
        int symbolSize() const { return subcarrierSymbols.size(); }
        void pushAPSKSymbol(const APSKSymbol* apskSymbol, int subcarrierIndex);
        void clearSymbols() { subcarrierSymbols.clear(); }
        ~OFDMSymbol();
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* __INET_OFDMSYMBOL_H_ */
