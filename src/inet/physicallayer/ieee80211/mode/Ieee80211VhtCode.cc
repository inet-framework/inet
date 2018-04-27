//
// Copyright (C) 2018 OpenSim Ltd.
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

#include "inet/physicallayer/ieee80211/mode/Ieee80211OfdmCode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211VhtCode.h"

namespace inet {
namespace physicallayer {

Ieee80211VhtCode::Ieee80211VhtCode(
        const Ieee80211ConvolutionalCode* forwardErrorCorrection,
        const Ieee80211VhtInterleaving* interleaving,
        const AdditiveScrambling* scrambling) :
                forwardErrorCorrection(forwardErrorCorrection),
                interleaving(interleaving),
                scrambling(scrambling)
{

}

const Ieee80211VhtCode* Ieee80211VhtCompliantCodes::getCompliantCode(const Ieee80211ConvolutionalCode *convolutionalCode, const Ieee80211OfdmModulation *stream1Modulation, const Ieee80211OfdmModulation *stream2Modulation, const Ieee80211OfdmModulation *stream3Modulation, const Ieee80211OfdmModulation *stream4Modulation, const Ieee80211OfdmModulation *stream5Modulation, const Ieee80211OfdmModulation *stream6Modulation, const Ieee80211OfdmModulation *stream7Modulation, const Ieee80211OfdmModulation *stream8Modulation, Hz bandwidth, bool withScrambling)
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
    if (stream5Modulation)
        numberOfCodedBitsPerSpatialStreams.push_back(stream5Modulation->getSubcarrierModulation()->getCodeWordSize());
    if (stream6Modulation)
        numberOfCodedBitsPerSpatialStreams.push_back(stream6Modulation->getSubcarrierModulation()->getCodeWordSize());
    if (stream7Modulation)
        numberOfCodedBitsPerSpatialStreams.push_back(stream7Modulation->getSubcarrierModulation()->getCodeWordSize());
    if (stream8Modulation)
        numberOfCodedBitsPerSpatialStreams.push_back(stream8Modulation->getSubcarrierModulation()->getCodeWordSize());
    return withScrambling ? new Ieee80211VhtCode(convolutionalCode, new Ieee80211VhtInterleaving(numberOfCodedBitsPerSpatialStreams, bandwidth), &Ieee80211OfdmCompliantCodes::ofdmScrambling) :
                            new Ieee80211VhtCode(convolutionalCode, new Ieee80211VhtInterleaving(numberOfCodedBitsPerSpatialStreams, bandwidth), nullptr);
}

Ieee80211VhtCode::~Ieee80211VhtCode()
{
    // NOTE: We assume that convolutional code and scrambling are static variables
    delete interleaving;
}

} /* namespace physicallayer */
} /* namespace inet */
