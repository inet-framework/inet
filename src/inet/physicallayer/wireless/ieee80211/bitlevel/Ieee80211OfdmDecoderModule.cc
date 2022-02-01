//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmDecoderModule.h"

#include "inet/physicallayer/wireless/common/modulation/BpskModulation.h"
#include "inet/physicallayer/wireless/common/modulation/Qam16Modulation.h"
#include "inet/physicallayer/wireless/common/modulation/Qam64Modulation.h"
#include "inet/physicallayer/wireless/common/modulation/QpskModulation.h"

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

std::ostream& Ieee80211OfdmDecoderModule::printToStream(std::ostream& stream, int level, int evFlags) const
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

