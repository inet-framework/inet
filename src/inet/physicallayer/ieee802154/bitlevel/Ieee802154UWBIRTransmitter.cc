/* -*- mode:c++ -*- ********************************************************
 * author:      Jerome Rousselot <jerome.rousselot@csem.ch>
 *
 * copyright:   (C) 2008 Centre Suisse d'Electronique et Microtechnique (CSEM) SA
 * 				Systems Engineering
 *              Real-Time Software and Networking
 *              Jaquet-Droz 1, CH-2002 Neuchatel, Switzerland.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 * description: this class holds constants specified in IEEE 802.15.4A UWB-IR Phy
 * acknowledgment: this work was supported (in part) by the National Competence
 * 			    Center in Research on Mobile Information and Communication Systems
 * 				NCCR-MICS, a center supported by the Swiss National Science
 * 				Foundation under grant number 5005-67322.
 ***************************************************************************/

#include "inet/physicallayer/ieee802154/bitlevel/Ieee802154UWBIRTransmitter.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/analogmodel/packetlevel/DimensionalTransmission.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee802154UWBIRTransmitter);

Ieee802154UWBIRTransmitter::Ieee802154UWBIRTransmitter()
{
}

void Ieee802154UWBIRTransmitter::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        cfg = Ieee802154UWBIRMode::cfg_mandatory_16M;
    }
}

std::ostream& Ieee802154UWBIRTransmitter::printToStream(std::ostream& stream, int level) const
{
    return stream << "Ieee802154UWBIRTransmitter";
}

simtime_t Ieee802154UWBIRTransmitter::getFrameDuration(int psduLength) const
{
    return cfg.preambleLength + (psduLength * 8 + 48) * cfg.data_symbol_duration;
}

simtime_t Ieee802154UWBIRTransmitter::getMaxFrameDuration() const
{
	return cfg.preambleLength + Ieee802154UWBIRMode::MaxPSDULength * cfg.data_symbol_duration;
}

simtime_t Ieee802154UWBIRTransmitter::getPhyMaxFrameDuration() const
{
    simtime_t phyMaxFrameDuration = 0;
    simtime_t TSHR, TPHR, TPSDU, TCCApreamble;
    TSHR = getThdr();
    TPHR = getThdr();
    phyMaxFrameDuration = phyMaxFrameDuration + TSHR;
    return phyMaxFrameDuration;
}

simtime_t Ieee802154UWBIRTransmitter::getThdr() const
{
	switch (cfg.channel) {
        default:
            switch (cfg.prf) {
                case Ieee802154UWBIRMode::NOMINAL_4_M:
                    //throw cRuntimeError("This optional mode is not implemented.");
                    return 0;
                    break;
                case Ieee802154UWBIRMode::NOMINAL_16_M:
                    return 16.4E-6;
                case Ieee802154UWBIRMode::NOMINAL_64_M:
                    return 16.8E-6;
                case Ieee802154UWBIRMode::PRF_OFF:
                    return 0;
            }
            break;
	}
	return 0;
}

void Ieee802154UWBIRTransmitter::generateSyncPreamble(Mapping* mapping, Argument* arg, const simtime_t startTime) const
{
    // NSync repetitions of the Si symbol
    for (short n = 0; n < cfg.NSync; n = n + 1) {
        for (short pos = 0; pos < cfg.CLength; pos = pos + 1) {
            if (Ieee802154UWBIRMode::C31[Ieee802154UWBIRMode::Ci - 1][pos] != 0) {
                if(n==0 && pos==0) {
                    // we slide the first pulse slightly in time to get the first point "inside" the signal
                  arg->setTime(1E-12 + n * cfg.sync_symbol_duration + pos * cfg.spreadingdL * cfg.pulse_duration);
                } else {
                  arg->setTime(n * cfg.sync_symbol_duration + pos * cfg.spreadingdL * cfg.pulse_duration);
                }
                //generatePulse(mapping, arg, startTime, C31[Ci - 1][pos], IEEE802154A::maxPulse, IEEE802154A::mandatory_pulse);
                generatePulse(mapping, arg, startTime, 1, Ieee802154UWBIRMode::maxPulse, cfg.pulse_duration); // always positive polarity
            }
        }
    }
}

void Ieee802154UWBIRTransmitter::generateSFD(Mapping* mapping, Argument* arg, const simtime_t startTime) const
{
    const simtime_t sfdStart = cfg.NSync * cfg.sync_symbol_duration;
    for (short n = 0; n < 8; n = n + 1) {
        if (Ieee802154UWBIRMode::shortSFD[n] != 0) {
            for (short pos = 0; pos < cfg.CLength; pos = pos + 1) {
                if (Ieee802154UWBIRMode::C31[Ieee802154UWBIRMode::Ci - 1][pos] != 0) {
                    arg->setTime(sfdStart + n * cfg.sync_symbol_duration + pos * cfg.spreadingdL*cfg.pulse_duration);
                    //generatePulse(mapping, arg, startTime, C31[Ci - 1][pos] * shortSFD[n]); // change pulse polarity
                    generatePulse(mapping, arg, startTime, 1, Ieee802154UWBIRMode::maxPulse, cfg.pulse_duration); // always positive polarity
                }
            }
        }
    }
}

void Ieee802154UWBIRTransmitter::generatePhyHeader(Mapping* mapping, Argument* arg, const simtime_t startTime) const
{
    // not implemented
}

void Ieee802154UWBIRTransmitter::generatePulse(Mapping* mapping, Argument* arg, const simtime_t startTime, short polarity, double peak, const simtime_t chip) const
{
    ASSERT(polarity == -1 || polarity == +1);
    arg->setTime(arg->getTime() + startTime);  // adjust argument so that we use absolute time values in Mapping
    mapping->setValue(*arg, 0);
    arg->setTime(arg->getTime() + chip / 2);
    // Maximum point at symbol half (triangular pulse)
    mapping->setValue(*arg, peak * polarity);
    arg->setTime(arg->getTime() + chip / 2);
    mapping->setValue(*arg, 0);
}

void Ieee802154UWBIRTransmitter::generateBurst(Mapping* mapping, Argument* arg, const simtime_t startTime, const simtime_t burstStart, short /*polarity*/) const
{
    // ASSERT(burstStart < cfg.preambleLength + (psduLength * 8 + 48 + 2) * cfg.data_symbol_duration);
    // 1. Start point = zeros
    simtime_t offset = burstStart;
    for (int pulse = 0; pulse < cfg.nbPulsesPerBurst; pulse++) {
        arg->setTime(offset);
        generatePulse(mapping, arg, startTime, 1, Ieee802154UWBIRMode::maxPulse, cfg.pulse_duration);
        offset = offset + cfg.pulse_duration;
    }
}

ConstMapping *Ieee802154UWBIRTransmitter::generateIEEE802154AUWBSignal(const simtime_t startTime, std::vector<bool> *bits) const
{
    // 48 R-S parity bits, the 2 symbols phy header is not modeled as it includes its own parity bits
    // and is thus very robust
    unsigned int bitLength = bits->size() + 48;
    // data start time relative to signal->getReceptionStart();
    simtime_t dataStart = cfg.preambleLength; // = Tsync + Tsfd
    TimeMapping<Linear>* mapping = new TimeMapping<Linear> ();
    Argument arg;

    generateSyncPreamble(mapping, &arg, startTime);
    generateSFD(mapping, &arg, startTime);
    //generatePhyHeader(mapping, &arg, startTime);

    // generate bit values and modulates them according to
    // the IEEE 802.15.4A specification
    simtime_t symbolStart = dataStart;
    simtime_t burstPos;
    for (unsigned int burst = 0; burst < bitLength; burst++) {
        int bit = burst < bits->size() ? bits->at(burst) : intuniform(0, 1, 0);
        burstPos = symbolStart + bit * cfg.shift_duration + cfg.getHoppingPos(burst) * cfg.burst_duration;
        generateBurst(mapping, &arg, startTime, burstPos, +1);
        symbolStart = symbolStart + cfg.data_symbol_duration;
    }
    return mapping;
}

const ITransmission *Ieee802154UWBIRTransmitter::createTransmission(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime) const
{
    int bitLength = macFrame->getBitLength();
    // KLUDGE: generate random bits until serializer is implemented
    std::vector<bool> *bits = new std::vector<bool>();
    for (int i = 0; i < bitLength; i++) {
        bool bitValue = intuniform(0, 1, 0);
        EV_INFO << "Transmitted bit at " << i << " is " << (int)bitValue << endl;
        bits->push_back(bitValue);
    }
    // KLUDGE: add a fake CRC
    for (int i = 0; i < 8; i++) {
        bits->push_back(0);
        for (int j = 0; j + i < bitLength; j += 8)
            bits->at(bitLength + i) = bits->at(bitLength + i) ^ bits->at(j + i);
    }
    const simtime_t duration = getFrameDuration(bits->size() / 8);
    const simtime_t endTime = startTime + duration;
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Coord startPosition = mobility->getCurrentPosition();
    const Coord endPosition = mobility->getCurrentPosition();
    const EulerAngles startOrientation = mobility->getCurrentAngularPosition();
    const EulerAngles endOrientation = mobility->getCurrentAngularPosition();
    const ConstMapping *powerMapping = generateIEEE802154AUWBSignal(startTime, bits);
    return new DimensionalTransmission(transmitter, macFrame, startTime, endTime, -1, -1, -1, startPosition, endPosition, startOrientation, endOrientation, -1, bitLength, cfg.bitrate, nullptr, cfg.centerFrequency, cfg.bandwidth, powerMapping);
}

} // namespace physicallayer

} // namespace inet

