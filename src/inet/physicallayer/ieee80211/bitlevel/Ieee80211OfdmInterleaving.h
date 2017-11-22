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

#ifndef __INET_IEEE80211OFDMINTERLEAVING_H
#define __INET_IEEE80211OFDMINTERLEAVING_H

#include "inet/physicallayer/contract/bitlevel/IInterleaver.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211OfdmInterleaving : public IInterleaving
{
  protected:
    int numberOfCodedBitsPerSymbol;
    int numberOfCodedBitsPerSubcarrier;

  public:
    Ieee80211OfdmInterleaving(int numberOfCodedBitsPerSymbol, int numberOfCodedBitsPerSubcarrier);

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    int getNumberOfCodedBitsPerSubcarrier() const { return numberOfCodedBitsPerSubcarrier; }
    int getNumberOfCodedBitsPerSymbol() const { return numberOfCodedBitsPerSymbol; }
};
} /* namespace physicallayer */
} /* namespace inet */

#endif // ifndef __INET_IEEE80211OFDMINTERLEAVING_H

