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

#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OfdmDecoderModule.h"
#include "inet/physicallayer/modulation/BpskModulation.h"
#include "inet/physicallayer/modulation/Qam16Modulation.h"
#include "inet/physicallayer/modulation/Qam64Modulation.h"
#include "inet/physicallayer/modulation/QpskModulation.h"

namespace inet {
namespace physicallayer {

Define_Module(Ieee80211OfdmDecoderModule);

void Ieee80211OfdmDecoderModule::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        descrambler = dynamic_cast<const IScrambler *>(getSubmodule("descrambler"));
        convolutionalDecoder = dynamic_cast<const IFecCoder *>(getSubmodule("fecDecoder"));
        deinterleaver = dynamic_cast<const IInterleaver *>(getSubmodule("deinterleaver"));
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER) {
        const ConvolutionalCode *convolutionalCode = convolutionalDecoder ? dynamic_cast<const ConvolutionalCode *>(convolutionalDecoder->getForwardErrorCorrection()) : nullptr;
        const Ieee80211OfdmInterleaving *interleaving = deinterleaver ? dynamic_cast<const Ieee80211OfdmInterleaving *>(deinterleaver->getInterleaving()) : nullptr;
        const AdditiveScrambling *scrambling = descrambler ? dynamic_cast<const AdditiveScrambling *>(descrambler->getScrambling()) : nullptr;
        code = new Ieee80211OfdmCode(convolutionalCode, interleaving, scrambling);
        ofdmDecoder = new Ieee80211OfdmDecoder(code);
    }
}

std::ostream& Ieee80211OfdmDecoderModule::printToStream(std::ostream& stream, int level) const
{
    return ofdmDecoder->printToStream(stream, level);
}

const IReceptionPacketModel *Ieee80211OfdmDecoderModule::decode(const IReceptionBitModel *bitModel) const
{
    return ofdmDecoder->decode(bitModel);
}

Ieee80211OfdmDecoderModule::~Ieee80211OfdmDecoderModule()
{
    delete code;
    delete ofdmDecoder;
}

} /* namespace physicallayer */
} /* namespace inet */

