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

#include "inet/physicallayer/ieee80211/mode/Ieee80211HtCode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OfdmCode.h"

namespace inet {
namespace physicallayer {

Ieee80211HtCode::Ieee80211HtCode(
        const Ieee80211ConvolutionalCode* forwardErrorCorrection,
        const Ieee80211HtInterleaving* interleaving,
        const AdditiveScrambling* scrambling) :
                forwardErrorCorrection(forwardErrorCorrection),
                interleaving(interleaving),
                scrambling(scrambling)
{

}

const Ieee80211HtCode* Ieee80211HtCompliantCodes::getCompliantCode(const Ieee80211ConvolutionalCode *convolutionalCode, const Ieee80211OfdmModulation *stream1Modulation, const Ieee80211OfdmModulation *stream2Modulation, const Ieee80211OfdmModulation *stream3Modulation, const Ieee80211OfdmModulation *stream4Modulation, Hz bandwidth, bool withScrambling)
{
    std::vector<unsigned int> numberOfCodedBitsPerSpatialStreams;
    if (stream1Modulation)
        numberOfCodedBitsPerSpatialStreams.push_back(stream1Modulation->getSubcarrierModulation()->getCodeWordSize());
    if (stream2Modulation)
        numberOfCodedBitsPerSpatialStreams.push_back(stream2Modulation->getSubcarrierModulation()->getCodeWordSize());
    if (stream3Modulation)
        numberOfCodedBitsPerSpatialStreams.push_back(stream3Modulation->getSubcarrierModulation()->getCodeWordSize());
    if (stream4Modulation)
        numberOfCodedBitsPerSpatialStreams.push_back(stream4Modulation->getSubcarrierModulation()->getCodeWordSize());
    return withScrambling ? new Ieee80211HtCode(convolutionalCode, new Ieee80211HtInterleaving(numberOfCodedBitsPerSpatialStreams, bandwidth), &Ieee80211OfdmCompliantCodes::ofdmScrambling) :
                            new Ieee80211HtCode(convolutionalCode, new Ieee80211HtInterleaving(numberOfCodedBitsPerSpatialStreams, bandwidth), nullptr);
}

Ieee80211HtCode::~Ieee80211HtCode()
{
    // NOTE: We assume that convolutional code and scrambling are static variables
    delete interleaving;
}

const Ieee80211ConvolutionalCode Ieee80211HtCompliantCodes::htConvolutionalCode5_6(5,6);

} /* namespace physicallayer */
} /* namespace inet */
