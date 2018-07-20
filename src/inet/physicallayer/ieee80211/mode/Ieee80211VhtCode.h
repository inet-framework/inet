//
// Copyright (C) 2015 OpenSim Ltd.
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

#ifndef __INET_IEEE80211VHTSIGNALCODE_H
#define __INET_IEEE80211VHTSIGNALCODE_H

#include "inet/physicallayer/common/bitlevel/AdditiveScrambling.h"
#include "inet/physicallayer/contract/bitlevel/ICode.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211ConvolutionalCode.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211VhtInterleaving.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OfdmModulation.h"

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
        // Convolutional codes (TODO: LDPC codes).
        static const Ieee80211VhtCode *getCompliantCode(const Ieee80211ConvolutionalCode *convolutionalCode, const Ieee80211OfdmModulation *stream1Modulation, const Ieee80211OfdmModulation *stream2Modulation, const Ieee80211OfdmModulation *stream3Modulation, const Ieee80211OfdmModulation *stream4Modulation, const Ieee80211OfdmModulation *stream5Modulation, const Ieee80211OfdmModulation *stream6Modulation, const Ieee80211OfdmModulation *stream7Modulation, const Ieee80211OfdmModulation *stream8Modulation, Hz bandwidth, bool withScrambling = true);

};

} /* namespace physicallayer */
} /* namespace inet */

#endif // ifndef __INET_IEEE80211HTSIGNALCODE_H
