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

#include "inet/physicallayer/analogmodel/packetlevel/DimensionalTransmission.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/ieee802154/bitlevel/Ieee802154UwbIrTransmitter.h"

namespace inet {
namespace physicallayer {

Define_Module(Ieee802154UwbIrTransmitter);

Ieee802154UwbIrTransmitter::Ieee802154UwbIrTransmitter()
{
}

void Ieee802154UwbIrTransmitter::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        cfg = Ieee802154UwbIrMode::cfg_mandatory_16M;
    }
}

std::ostream& Ieee802154UwbIrTransmitter::printToStream(std::ostream& stream, int level) const
{
    return stream << "Ieee802154UwbIrTransmitter";
}

simtime_t Ieee802154UwbIrTransmitter::getFrameDuration(int psduLength) const
{
    return cfg.preambleLength + (psduLength * 8 + 48) * cfg.data_symbol_duration;
}

simtime_t Ieee802154UwbIrTransmitter::getMaxFrameDuration() const
{
	return cfg.preambleLength + Ieee802154UwbIrMode::MaxPSDULength * cfg.data_symbol_duration;
}

simtime_t Ieee802154UwbIrTransmitter::getPhyMaxFrameDuration() const
{
    simtime_t phyMaxFrameDuration = 0;
    simtime_t TSHR, TPHR, TPSDU, TCCApreamble;
    TSHR = getThdr();
    TPHR = getThdr();
    phyMaxFrameDuration = phyMaxFrameDuration + TSHR;
    return phyMaxFrameDuration;
}

simtime_t Ieee802154UwbIrTransmitter::getThdr() const
{
	switch (cfg.channel) {
        default:
            switch (cfg.prf) {
                case Ieee802154UwbIrMode::NOMINAL_4_M:
                    //throw cRuntimeError("This optional mode is not implemented.");
                    return 0;
                    break;
                case Ieee802154UwbIrMode::NOMINAL_16_M:
                    return 16.4E-6;
                case Ieee802154UwbIrMode::NOMINAL_64_M:
                    return 16.8E-6;
                case Ieee802154UwbIrMode::PRF_OFF:
                    return 0;
            }
            break;
	}
	return 0;
}

void Ieee802154UwbIrTransmitter::generateSyncPreamble(std::map<simsec, WpHz>& data, simtime_t& time, const simtime_t startTime) const
{
    // NSync repetitions of the Si symbol
    for (short n = 0; n < cfg.NSync; n = n + 1) {
        for (short pos = 0; pos < cfg.CLength; pos = pos + 1) {
            if (Ieee802154UwbIrMode::C31[Ieee802154UwbIrMode::Ci - 1][pos] != 0) {
                if(n==0 && pos==0)
                    // we slide the first pulse slightly in time to get the first point "inside" the signal
                    time = 1E-12 + n * cfg.sync_symbol_duration + pos * cfg.spreadingdL * cfg.pulse_duration;
                else
                    time = n * cfg.sync_symbol_duration + pos * cfg.spreadingdL * cfg.pulse_duration;
                //generatePulse(data, time, startTime, C31[Ci - 1][pos], IEEE802154A::maxPulse, IEEE802154A::mandatory_pulse);
                generatePulse(data, time, startTime, 1, Ieee802154UwbIrMode::maxPulse, cfg.pulse_duration); // always positive polarity
            }
        }
    }
}

void Ieee802154UwbIrTransmitter::generateSFD(std::map<simsec, WpHz>& data, simtime_t& time, const simtime_t startTime) const
{
    const simtime_t sfdStart = cfg.NSync * cfg.sync_symbol_duration;
    for (short n = 0; n < 8; n = n + 1) {
        if (Ieee802154UwbIrMode::shortSFD[n] != 0) {
            for (short pos = 0; pos < cfg.CLength; pos = pos + 1) {
                if (Ieee802154UwbIrMode::C31[Ieee802154UwbIrMode::Ci - 1][pos] != 0) {
                    time = sfdStart + n * cfg.sync_symbol_duration + pos * cfg.spreadingdL*cfg.pulse_duration;
                    //generatePulse(data, time, startTime, C31[Ci - 1][pos] * shortSFD[n]); // change pulse polarity
                    generatePulse(data, time, startTime, 1, Ieee802154UwbIrMode::maxPulse, cfg.pulse_duration); // always positive polarity
                }
            }
        }
    }
}

void Ieee802154UwbIrTransmitter::generatePhyHeader(std::map<simsec, WpHz>& data, simtime_t& time, const simtime_t startTime) const
{
    // not implemented
}

void Ieee802154UwbIrTransmitter::generatePulse(std::map<simsec, WpHz>& data, simtime_t& time, const simtime_t startTime, short polarity, double peak, const simtime_t chip) const
{
    ASSERT(polarity == -1 || polarity == +1);
    time += startTime;  // adjust argument so that we use absolute time values in function
    data[simsec(time)] = WpHz(0);
    time += chip / 2;
    // Maximum point at symbol half (triangular pulse)
    // TODO: polarity doesn't really make sense this way!?
    data[simsec(time)] = W(peak * polarity) / GHz(10.6 - 3.1);
    time += chip / 2;
    data[simsec(time)] = WpHz(0);
}

void Ieee802154UwbIrTransmitter::generateBurst(std::map<simsec, WpHz>& data, simtime_t& time, const simtime_t startTime, const simtime_t burstStart, short /*polarity*/) const
{
    // ASSERT(burstStart < cfg.preambleLength + (psduLength * 8 + 48 + 2) * cfg.data_symbol_duration);
    // 1. Start point = zeros
    simtime_t offset = burstStart;
    for (int pulse = 0; pulse < cfg.nbPulsesPerBurst; pulse++) {
        time = offset;
        generatePulse(data, time, startTime, 1, Ieee802154UwbIrMode::maxPulse, cfg.pulse_duration);
        offset = offset + cfg.pulse_duration;
    }
}

Ptr<const IFunction<WpHz, Domain<simsec, Hz>>> Ieee802154UwbIrTransmitter::generateIEEE802154AUWBSignal(const simtime_t startTime, std::vector<bool> *bits) const
{
    // 48 R-S parity bits, the 2 symbols phy header is not modeled as it includes its own parity bits
    // and is thus very robust
    unsigned int bitLength = bits->size() + 48;
    // data start time relative to signal->getReceptionStart();
    simtime_t dataStart = cfg.preambleLength; // = Tsync + Tsfd
    std::map<simsec, WpHz> data;
    data[getLowerBound<simsec>()] = WpHz(0);
    data[getUpperBound<simsec>()] = WpHz(0);
    simtime_t time = 0;

    generateSyncPreamble(data, time, startTime);
    generateSFD(data, time, startTime);
    //generatePhyHeader(data, time, startTime);

    // generate bit values and modulates them according to
    // the IEEE 802.15.4A specification
    simtime_t symbolStart = dataStart;
    simtime_t burstPos;
    for (unsigned int burst = 0; burst < bitLength; burst++) {
        int bit = burst < bits->size() ? bits->at(burst) : intuniform(0, 1, 0);
        burstPos = symbolStart + bit * cfg.shift_duration + cfg.getHoppingPos(burst) * cfg.burst_duration;
        generateBurst(data, time, startTime, burstPos, +1);
        symbolStart = symbolStart + cfg.data_symbol_duration;
    }
    auto timeFunction = makeShared<Interpolated1DFunction<WpHz, simsec>>(data, &LinearInterpolator<simsec, WpHz>::singleton);
    auto frequencyFunction = makeShared<Boxcar1DFunction<double, Hz>>(GHz(3.1), GHz(10.6), 1);
    return makeShared<Combined2DFunction<WpHz, simsec, Hz>>(timeFunction, frequencyFunction);
}

const ITransmission *Ieee802154UwbIrTransmitter::createTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime) const
{
    int bitLength = packet->getBitLength();
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
    const Quaternion startOrientation = mobility->getCurrentAngularPosition();
    const Quaternion endOrientation = mobility->getCurrentAngularPosition();
    const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& powerFunction = generateIEEE802154AUWBSignal(startTime, bits);
    return new DimensionalTransmission(transmitter, packet, startTime, endTime, -1, -1, -1, startPosition, endPosition, startOrientation, endOrientation, nullptr, packet->getTotalLength(), b(-1), cfg.centerFrequency, cfg.bandwidth, cfg.bitrate, powerFunction);
}

} // namespace physicallayer
} // namespace inet

