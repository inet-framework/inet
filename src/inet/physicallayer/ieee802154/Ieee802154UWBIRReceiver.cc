//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "inet/physicallayer/ieee802154/Ieee802154UWBIRReceiver.h"
#include "inet/physicallayer/analogmodel/DimensionalReception.h"
#include "inet/physicallayer/analogmodel/DimensionalNoise.h"
#include "inet/physicallayer/common/BandListening.h"
#include "inet/physicallayer/common/ListeningDecision.h"
#include "inet/physicallayer/common/ReceptionDecision.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee802154UWBIRReceiver);

Ieee802154UWBIRReceiver::Ieee802154UWBIRReceiver() :
    ReceiverBase()
{
}

void Ieee802154UWBIRReceiver::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        cfg = Ieee802154UWBIRMode::cfg_mandatory_16M;
    }
}

void Ieee802154UWBIRReceiver::printToStream(std::ostream& stream) const
{
    stream << "Ieee802154UWBIRReceiver";
}

const IListening *Ieee802154UWBIRReceiver::createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const
{
    return new BandListening(radio, startTime, endTime, startPosition, endPosition, cfg.centerFrequency, cfg.bandwidth);
}

const IListeningDecision *Ieee802154UWBIRReceiver::computeListeningDecision(const IListening *listening, const IInterference *interference) const
{
    return new ListeningDecision(listening, true);
}

bool Ieee802154UWBIRReceiver::computeIsReceptionPossible(const IListening *listening, const IReception *reception) const
{
    return true;
}

bool Ieee802154UWBIRReceiver::computeIsReceptionAttempted(const IListening *listening, const IReception *reception, const IInterference *interference) const
{
    return true;
}

const IReceptionDecision *Ieee802154UWBIRReceiver::computeReceptionDecision(const IListening *listening, const IReception *reception, const IInterference *interference) const
{
    RadioReceptionIndication *indication = new RadioReceptionIndication();
    std::vector<bool> *bits = decode(reception, interference->getInterferingReceptions(), interference->getBackgroundNoise());
    int bitLength = bits->size() - 48 - 8;
    bool isReceptionSuccessful = true;
    for (int i = 0; i < bitLength; i++) {
        bool bitValue = bits->at(i);
        EV_INFO << "Received bit at " << i << " is " << (int)bitValue << endl;
    }
    // KLUDGE: check fake CRC
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j + i < bitLength; j += 8)
            bits->at(bitLength + i) = bits->at(bitLength + i) ^ bits->at(j + i);
        isReceptionSuccessful &= !bits->at(bitLength + i);
    }
    delete bits;
    return new ReceptionDecision(reception, indication, true, true, isReceptionSuccessful);
}

std::vector<bool> *Ieee802154UWBIRReceiver::decode(const IReception *reception, const std::vector<const IReception *> *interferingReceptions, const INoise *backgroundNoise) const
{
    simtime_t now, offset;
    simtime_t aSymbol, shift, burst;
    // times are absolute
    offset  = reception->getStartTime() + cfg.preambleLength;
    shift   = cfg.shift_duration;
    aSymbol = cfg.data_symbol_duration;
    burst   = cfg.burst_duration;
    now     = offset + cfg.pulse_duration / 2;
    std::pair<double, double> energyZero, energyOne;
    // Loop to decode each bit value
    int symbol;
    double packetSNIR = 0;
    std::vector<bool> *bits = new std::vector<bool>();
    simtime_t duration = reception->getEndTime() - reception->getStartTime();
    for (symbol = 0; cfg.preambleLength + symbol * aSymbol < duration; symbol++) {
        // sample in window zero
        now = now + cfg.getHoppingPos(symbol) * cfg.burst_duration;
        energyZero = integrateWindow(now, burst, reception, interferingReceptions, backgroundNoise);
        // sample in window one
        now = now + shift;
        energyOne  = integrateWindow(now, burst, reception, interferingReceptions, backgroundNoise);
        int decodedBit;
        if (energyZero.second > energyOne.second) {
            decodedBit = 0;
            packetSNIR = packetSNIR + energyZero.first;
        }
        else {
            decodedBit = 1;
            packetSNIR = packetSNIR + energyOne.first;
        }
        bits->push_back(decodedBit);
        now = offset + (symbol + 1) * aSymbol + cfg.pulse_duration / 2;
    }
    // TODO: review this whole SNR computation, seems to be wrong (from MiXiM)
    packetSNIR = packetSNIR / (symbol + 1);
    // TODO: double snirLastPacket = 10 * log10(packetSNIR);  // convert to dB
    // TODO: return SNIR?
    return bits;
}

/*
 * @brief Returns a pair with as first value the SNIR (if the signal is not null in this window, and 0 otherwise)
 * and as second value a "score" associated to this window. This score equals to the sum for all
 * 16 pulse peak positions of the voltage measured by the receiver ADC.
 */
// TODO: review this code regarding the dimensional API, could it be done simply by simply adding the signal, the interference and the noise and integrating that?
std::pair<double, double> Ieee802154UWBIRReceiver::integrateWindow(simtime_t_cref pNow, simtime_t_cref burst, const IReception *reception, const std::vector<const IReception *> *interferingReceptions, const INoise *backgroundNoise) const
{
    std::pair<double, double>       energy = std::make_pair(0.0, 0.0); // first: stores SNIR, second: stores total captured window energy
    Argument                        arg;
    simtime_t                       windowEnd = pNow + burst;
    const double                    peakPulsePower       = 1.3E-3; //1.3E-3 W peak power of pulse to reach  0dBm during burst; // peak instantaneous power of the transmitted pulse (A=0.6V) : 7E-3 W. But peak limit is 0 dBm
    // Triangular baseband pulses
    // we sample at each pulse peak
    // get the interpolated values of amplitude for each interferer
    // and add these to the peak with a random phase
    // we sample one point per pulse
    // caller has already set our time reference ("now") at the peak of the pulse
    for (simtime_t now = pNow; now < windowEnd; now += cfg.pulse_duration) {
        double signalValue      = 0; // electric field from tracked signal [V/m²]
        double resPower         = 0; // electric field at antenna = combination of all arriving electric fields [V/m²]
        double vEfield          = 0; // voltage at antenna caused by electric field Efield [V]
        double vmeasured        = 0; // voltage measured by energy-detector [V], including thermal noise
        double vmeasured_square = 0; // to the square [V²]
        double snir             = 0; // burst SNIR estimate
        double vThermalNoise    = 0; // thermal noise realization
        arg.setTime(now);
        // consider signal power
        const DimensionalReception *dimensionalSignalReception = check_and_cast<const DimensionalReception *>(reception);
        const ConstMapping *const signalPower = dimensionalSignalReception->getPower();
        double measure = signalPower->getValue(arg) * peakPulsePower; //TODO: de-normalize (peakPulsePower should be in AirFrame or in Signal, to be set at run-time)
        signalValue = measure * 0.5; // we capture half of the maximum possible pulse energy to account for self  interference
        resPower    = resPower + signalValue;
        // consider all interferers at this point in time
        for (std::vector<const IReception *>::const_iterator it = interferingReceptions->begin(); it != interferingReceptions->end(); it++) {
            const DimensionalReception *dimensionalInterferingReception = check_and_cast<const DimensionalReception *>(*it);
            const ConstMapping *const interferingPower = dimensionalInterferingReception->getPower();
            double measure = interferingPower->getValue(arg) * peakPulsePower; //TODO: de-normalize (peakPulsePower should be in AirFrame or in Signal, to be set at run-time)
//          measure = measure * uniform(0, +1); // random point of Efield at sampling (due to pulse waveform and self interference)
            // take a random point within pulse envelope for interferer
            resPower = resPower + measure * uniform(-1, +1);
        }
//      double attenuatedPower = resPower / 10; // 10 dB = 6 dB implementation loss + 5 dB noise factor
        vEfield          = sqrt(50*resPower); // P=V²/R
        // add thermal noise realization
        const DimensionalNoise *dimensionalBackgroundNoise = check_and_cast<const DimensionalNoise *>(backgroundNoise);
        vThermalNoise    = dimensionalBackgroundNoise->getPower()->getValue(arg);
        vmeasured        = vEfield + vThermalNoise;
        vmeasured_square = pow(vmeasured, 2);
        // signal + interference + noise
        energy.second    = energy.second + vmeasured_square;  // collect this contribution
        // Now evaluates signal to noise ratio
        // signal converted to antenna voltage squared
        // TODO: review this SNIR computation
        snir             = signalValue / 2.0217E-12;
        energy.first     = energy.first + snir;

    } // consider next point in time
    return energy;
}

} // namespace physicallayer

} // namespace inet

