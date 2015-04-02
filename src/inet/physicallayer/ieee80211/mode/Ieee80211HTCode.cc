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

#include "inet/physicallayer/ieee80211/mode/Ieee80211HTCode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OFDMCode.h"

namespace inet {
namespace physicallayer {

Ieee80211HTCode::Ieee80211HTCode(
        const IForwardErrorCorrection* forwardErrorCorrection,
        const Ieee80211HTInterleaving* interleaving,
        const AdditiveScrambling* scrambling) :
                forwardErrorCorrection(forwardErrorCorrection),
                interleaving(interleaving),
                scrambling(scrambling)
{

}

const Ieee80211HTCode* Ieee80211HTCompliantCodes::getCompliantCode(const Ieee80211ConvolutionalCode *convolutionalCode, const Ieee80211OFDMModulation *stream1Modulation, const Ieee80211OFDMModulation *stream2Modulation, const Ieee80211OFDMModulation *stream3Modulation, const Ieee80211OFDMModulation *stream4Modulation, Hz bandwidth, bool withScrambling)
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
    return withScrambling ? new Ieee80211HTCode(convolutionalCode, new Ieee80211HTInterleaving(numberOfCodedBitsPerSpatialStreams, bandwidth), &Ieee80211OFDMCompliantCodes::ofdmScrambling) :
                            new Ieee80211HTCode(convolutionalCode, new Ieee80211HTInterleaving(numberOfCodedBitsPerSpatialStreams, bandwidth), nullptr);
}

Ieee80211HTCode::~Ieee80211HTCode()
{
    // NOTE: We assume that convolutional code and scrambling are static variables
    delete interleaving;
}

const Ieee80211ConvolutionalCode Ieee80211HTCompliantCodes::htConvolutionalCode5_6(5,6);

} /* namespace physicallayer */
} /* namespace inet */
