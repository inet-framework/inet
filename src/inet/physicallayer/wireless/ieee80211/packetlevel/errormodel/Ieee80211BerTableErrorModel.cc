//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/packetlevel/errormodel/Ieee80211BerTableErrorModel.h"

#include "inet/physicallayer/wireless/common/base/packetlevel/FlatTransmissionBase.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211TransmissionBase.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/errormodel/Ieee80211NistErrorModel.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211BerTableErrorModel);

Ieee80211BerTableErrorModel::Ieee80211BerTableErrorModel() :
    berTableFile(nullptr)
{
}

Ieee80211BerTableErrorModel::~Ieee80211BerTableErrorModel()
{
    delete berTableFile;
}

void Ieee80211BerTableErrorModel::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        const char *fname = par("berTableFile");
        if (fname == nullptr)
            throw cRuntimeError("BER file parameter is mandatory");
        // TODO remove and cleanup opMode from here and also from BerParseFile, this should depend on the received signal
        char opMode;
        const char *opModeString = par("opMode");
        if (!strcmp("b", opModeString))
            opMode = 'b';
        else if (!strcmp("g(erp)", opModeString))
            opMode = 'g';
        else if (!strcmp("g(mixed)", opModeString))
            opMode = 'g';
        else if (!strcmp("a", opModeString))
            opMode = 'a';
        else if (!strcmp("p", opModeString))
            opMode = 'p';
        else
            throw cRuntimeError("Unknown opMode");
        berTableFile = new BerParseFile(opMode);
        berTableFile->parseFile(fname);
    }
}

double Ieee80211BerTableErrorModel::computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method("computePacketErrorRate");
    const ITransmission *transmission = snir->getReception()->getTransmission();
    const FlatTransmissionBase *flatTransmission = check_and_cast<const FlatTransmissionBase *>(transmission);
    double bitrate = flatTransmission->getBitrate().get();
    b dataLength = flatTransmission->getDataLength();
    return berTableFile->getPer(bitrate, getScalarSnir(snir), B(dataLength).get());
}

double Ieee80211BerTableErrorModel::computeBitErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method("computeBitErrorRate");
    return NaN;
}

double Ieee80211BerTableErrorModel::computeSymbolErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method("computeSymbolErrorRate");
    return NaN;
}

} // namespace physicallayer

} // namespace inet

