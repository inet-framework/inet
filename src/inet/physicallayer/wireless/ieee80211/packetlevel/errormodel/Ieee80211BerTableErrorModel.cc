//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/packetlevel/errormodel/Ieee80211BerTableErrorModel.h"

#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/errormodel/Ieee80211NistErrorModel.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Radio.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Transmission.h"

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
    ErrorModelBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        std::string fname = getEnvir()->getConfig()->substituteVariables(par("berTableFile"));
        if (fname.empty())
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
            throw cRuntimeError("Unknown opMode: '%s'", opModeString);
        berTableFile = new BerParseFile(opMode);
        berTableFile->parseFile(fname.c_str());
    }
}

double Ieee80211BerTableErrorModel::computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method("computePacketErrorRate");
    auto transmission = check_and_cast<const Ieee80211Transmission *>(snir->getReception()->getTransmission());
    bps bitrate = transmission->getMode()->getDataMode()->getNetBitrate();
    // a packet-level transmission has no bit model: the PHY header gives the length of the PSDU
    B dataLength = Ieee80211Radio::peekIeee80211PhyHeaderAtFront(transmission->getPacket())->getLengthField();
    return berTableFile->getPer(bitrate.get<bps>(), getScalarSnir(snir), dataLength.get());
}

double Ieee80211BerTableErrorModel::computeBitErrorRate(const ISnir *snir, IRadioSignal::SignalPart part, b length, simtime_t startTime, simtime_t endTime) const
{
    Enter_Method("computeBitErrorRate");
    checkRegionPart(part);
    // the table gives the error rate of the data, as the whole-part rate uses it
    if (part == IRadioSignal::SIGNAL_PART_HEADER)
        return 0;
    auto transmission = check_and_cast<const Ieee80211Transmission *>(snir->getReception()->getTransmission());
    bps bitrate = transmission->getMode()->getDataMode()->getNetBitrate();
    B dataLength = Ieee80211Radio::peekIeee80211PhyHeaderAtFront(transmission->getPacket())->getLengthField();
    // each bit of the PSDU has its share of the table value of the PSDU; the table can give a value above 1
    double packetErrorRate = std::min(1.0, berTableFile->getPer(bitrate.get<bps>(), getScalarSnir(snir, startTime, endTime), dataLength.get()));
    return -std::expm1(std::log1p(-packetErrorRate) / b(dataLength).get<b>());
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

