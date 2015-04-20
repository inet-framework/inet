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

#include "Ieee80211OFDMDecoderModule.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/modulation/QPSKModulation.h"
#include "inet/physicallayer/modulation/QAM16Modulation.h"
#include "inet/physicallayer/modulation/QAM64Modulation.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211OFDMDecoderModule);

void Ieee80211OFDMDecoderModule::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        descrambler = dynamic_cast<const IScrambler *>(getSubmodule("descrambler"));
        convolutionalDecoder = dynamic_cast<const IFECCoder *>(getSubmodule("fecDecoder"));
        deinterleaver = dynamic_cast<const IInterleaver *>(getSubmodule("deinterleaver"));
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER) {
        const ConvolutionalCode *convolutionalCode = convolutionalDecoder ? dynamic_cast<const ConvolutionalCode *>(convolutionalDecoder->getForwardErrorCorrection()) : nullptr;
        const Ieee80211OFDMInterleaving *interleaving = deinterleaver ? dynamic_cast<const Ieee80211OFDMInterleaving *>(deinterleaver->getInterleaving()) : nullptr;
        const AdditiveScrambling *scrambling = descrambler ? dynamic_cast<const AdditiveScrambling *>(descrambler->getScrambling()) : nullptr;
        code = new Ieee80211OFDMCode(convolutionalCode, interleaving, scrambling);
        ofdmDecoder = new Ieee80211OFDMDecoder(code);
    }
}

std::ostream& Ieee80211OFDMDecoderModule::printToStream(std::ostream& stream, int level) const
{
    return ofdmDecoder->printToStream(stream, level);
}

const IReceptionPacketModel *Ieee80211OFDMDecoderModule::decode(const IReceptionBitModel *bitModel) const
{
    return ofdmDecoder->decode(bitModel);
}

Ieee80211OFDMDecoderModule::~Ieee80211OFDMDecoderModule()
{
    delete code;
    delete ofdmDecoder;
}
} /* namespace physicallayer */
} /* namespace inet */

