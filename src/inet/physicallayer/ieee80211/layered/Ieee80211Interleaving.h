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

#ifndef __INET_IEEE80211INTERLEAVING_H_
#define __INET_IEEE80211INTERLEAVING_H_

#include "inet/physicallayer/contract/IInterleaver.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211Interleaving : public IInterleaving
{
    protected:
        int numberOfCodedBitsPerSymbol;
        int numberOfCodedBitsPerSubcarrier;

    public:
        Ieee80211Interleaving(int numberOfCodedBitsPerSymbol, int numberOfCodedBitsPerSubcarrier) :
            numberOfCodedBitsPerSymbol(numberOfCodedBitsPerSymbol),
            numberOfCodedBitsPerSubcarrier(numberOfCodedBitsPerSubcarrier) {}
        void printToStream(std::ostream& stream) const { stream << "Ieee80211Interleaver"; } // TODO: extend this info
        int getNumberOfCodedBitsPerSubcarrier() const { return numberOfCodedBitsPerSubcarrier; }
        int getNumberOfCodedBitsPerSymbol() const { return numberOfCodedBitsPerSymbol; }
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* __INET_IEEE80211INTERLEAVING_H_ */
