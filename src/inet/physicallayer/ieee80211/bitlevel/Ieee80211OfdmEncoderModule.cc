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

#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OfdmEncoderModule.h"

namespace inet {
namespace physicallayer {

Define_Module(Ieee80211OfdmEncoderModule);

void Ieee80211OfdmEncoderModule::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        scrambler = dynamic_cast<IScrambler *>(getSubmodule("scrambler"));
        convolutionalCoder = dynamic_cast<IFecCoder *>(getSubmodule("fecEncoder"));
        interleaver = dynamic_cast<IInterleaver *>(getSubmodule("interleaver"));
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER) {
        const ConvolutionalCode *convolutionalCode = convolutionalCoder? check_and_cast<const ConvolutionalCode *>(convolutionalCoder->getForwardErrorCorrection()) : nullptr;
        const Ieee80211OfdmInterleaving *interleaving = interleaver ? check_and_cast<const Ieee80211OfdmInterleaving *>(interleaver->getInterleaving()) : nullptr;
        const AdditiveScrambling *scrambling = scrambler ? check_and_cast<const AdditiveScrambling *>(scrambler->getScrambling()) : nullptr;
        code = new Ieee80211OfdmCode(convolutionalCode, interleaving, scrambling);
        encoder = new Ieee80211OfdmEncoder(code);
    }
}

const ITransmissionBitModel *Ieee80211OfdmEncoderModule::encode(const ITransmissionPacketModel *packetModel) const
{
    return encoder->encode(packetModel);
}

Ieee80211OfdmEncoderModule::~Ieee80211OfdmEncoderModule()
{
    delete code;
    delete encoder;
}

} /* namespace physicallayer */
} /* namespace inet */

