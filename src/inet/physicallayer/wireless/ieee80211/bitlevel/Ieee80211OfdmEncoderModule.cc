//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmEncoderModule.h"

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
        const ConvolutionalCode *convolutionalCode = convolutionalCoder ? check_and_cast<const ConvolutionalCode *>(convolutionalCoder->getForwardErrorCorrection()) : nullptr;
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

