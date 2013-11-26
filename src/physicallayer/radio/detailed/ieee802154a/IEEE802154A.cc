/* -*- mode:c++ -*- ********************************************************
 * file:        IEEE802154A.cc
 *
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

#include "IEEE802154A.h"

#include <cassert>

using std::vector;

// bit rate (850 kbps)
const int IEEE802154A::mandatory_bitrate = 850000;
// mandatory data symbol length (1.025 ms)
const_simtime_t IEEE802154A::mandatory_symbol = 0.00000102364; //102564
// 0.5 * mandatory_symbol (0.5 ms)
const_simtime_t IEEE802154A::mandatory_timeShift = 0.00000051282;
// mandatory pulse duration ( = 1 / bandwidth = 2 ns)
const_simtime_t IEEE802154A::mandatory_pulse = 0.000000002003203125;
// burst duration (32 ns)
const_simtime_t IEEE802154A::mandatory_burst = 0.00000003205;
// number of consecutive pulses forming a burst
const int IEEE802154A::mandatory_pulses_per_burst = 16;
// Center frequency of band 3 in UWB lower band (500 MHz wide channel)
const double IEEE802154A::mandatory_centerFreq = 4498; // MHz
// default sync preamble length
const_simtime_t IEEE802154A::mandatory_preambleLength = 0.0000715; // Tpre=71.5 µs
// Normalized pulse envelope peak
const double IEEE802154A::maxPulse = 1;
// Duration of a sync preamble symbol
const_simtime_t IEEE802154A::Tpsym = 0.001 * 0.001 * 0.001 * 993.6; // 993.6 ns

const_simtime_t IEEE802154A::tFirstSyncPulseMax = IEEE802154A::mandatory_pulse/2;

// Ci values
const short IEEE802154A::C31[8][31] = {
// C1
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0 },
		// C2
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0 },
		// C3
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0 },
		// C4
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0 },
		// C5
		{ -1, 0, +1, -1, 0, 0, +1, +1, +1, -1, +1, 0, 0, 0, -1, +1, 0, +1, +1,
				+1, 0, -1, 0, +1, 0, 0, 0, 0, -1, 0, 0 },
		// C6
		{ +1, +1, 0, 0, +1, 0, 0, -1, -1, -1, +1, -1, 0, +1, +1, -1, 0, 0, 0,
				+1, 0, +1, 0, -1, +1, 0, +1, 0, 0, 0, 0 },
		// C7
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0 },
		// C8
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0 },

};

const short IEEE802154A::shortSFD[8] = { 0, 1, 0, -1, 1, 0, 0, -1 };

short IEEE802154A::s_array[maxS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		0, 0, 0 };
int IEEE802154A::last_s = 15;

double IEEE802154A::signalStart = 0;

int IEEE802154A::psduLength = 0;

//const_simtime_t IEEE802154A::MaxFrameDuration = IEEE802154A::MaxPSDULength*IEEE802154A::mandatory_symbol + IEEE802154A::mandatory_preambleLength;

const IEEE802154A::config IEEE802154A::cfg_mandatory_16M = {
		3,					// channel
		NOMINAL_16_M, 		// PRF
		NON_RANGING, 		// Frame type
		PSR_DEFAULT,		// preamble length (number of preamble symbols)
		31,					// Spreading code length
		16,					// spreading delta L
		16,					// chips per burst
		850000,				// bit rate (bps)
		16,					// pulses per data burst
		993.6E-9,			// preamble symbol duration
		1023.64E-9,			// data symbol duration
		512.82E-9,			// burst time shift duration
		2.003E-9,			// pulse duration (chip)
		32.05E-9,			// burst duration (pulses per data burst * chip)
		71.5E-6,			// synchronization preamble duration
		4498				// center frequency
};

const IEEE802154A::config IEEE802154A::cfg_mandatory_4M = {
		3,					// channel
		NOMINAL_4_M, 		// PRF
		NON_RANGING, 		// Frame type
		PSR_DEFAULT,		// preamble length (number of preamble symbols)
		31,					// Spreading code length
		64,					// spreading delta L
		4,					// chips per burst
		850000,				// bit rate (bps)
		4,					// pulses per data burst
		3974.36E-9,			// preamble symbol duration
		1023.64E-9,			// data symbol duration
		512.82E-9,			// burst time shift duration
		2.003E-9,			// pulse duration (chip)
		8.01E-9,			// burst duration (pulses per data burst * chip)
		286.2E-6,			// synchronization preamble duration
		4498				// center frequency
};

IEEE802154A::config IEEE802154A::cfg = IEEE802154A::cfg_mandatory_16M;

void IEEE802154A::setConfig(config newCfg) {
	cfg = newCfg;
}

void IEEE802154A::setPSDULength(int _psduLength) {
	//assert(_psduLength < IEEE802154A::MaxPSDULength+1);
	IEEE802154A::psduLength = _psduLength;
}

simtime_t IEEE802154A::getMaxFrameDuration() {
	return IEEE802154A::MaxPSDULength*cfg.data_symbol_duration + cfg.preambleLength;
}

IEEE802154A::signalAndData IEEE802154A::generateIEEE802154AUWBSignal(
		simtime_t_cref signalStart, bool allZeros) {
	// 48 R-S parity bits, the 2 symbols phy header is not modeled as it includes its own parity bits
	// and is thus very robust
	unsigned int nbBits = IEEE802154A::psduLength * 8 + 48;
	IEEE802154A::signalStart = SIMTIME_DBL(signalStart);  // use the signalStart time value as a global offset for all Mapping values
	simtime_t signalDuration = cfg.preambleLength;
	signalDuration += static_cast<double> (nbBits) * cfg.data_symbol_duration;
	Signal* s = new Signal(signalStart, signalDuration);
	vector<bool>* bitValues = new vector<bool> ();

	signalAndData res;
	int bitValue;
	// data start time relative to signal->getReceptionStart();
	simtime_t dataStart = cfg.preambleLength; // = Tsync + Tsfd
	TimeMapping<Linear>* mapping = new TimeMapping<Linear> ();
	Argument* arg = new Argument();
	setBitRate(s);

	generateSyncPreamble(mapping, arg);
	generateSFD(mapping, arg);
	//generatePhyHeader(mapping, arg);

	// generate bit values and modulates them according to
	// the IEEE 802.15.4A specification
	simtime_t symbolStart = dataStart;
	simtime_t burstPos;
	for (unsigned int burst = 0; burst < nbBits; burst++) {
		if(allZeros) {
			bitValue = 0;
		} else {
		  bitValue = intuniform(0, 1, 0);
		}
		bitValues->push_back(static_cast<bool>(bitValue));
		burstPos = symbolStart + bitValue*cfg.shift_duration + getHoppingPos(burst)*cfg.burst_duration;
		generateBurst(mapping, arg, burstPos, +1);
		symbolStart = symbolStart + cfg.data_symbol_duration;
	}
	//assert(uwbirMacPkt->getBitValuesArraySize() == dataLength);
	//assert(bitValues->size() == nbBits);

	// associate generated pulse energies to the signal
	s->setTransmissionPower(mapping);
	delete arg;

	res.first = s;
	res.second = bitValues;
	return res;
}

void IEEE802154A::generateSyncPreamble(Mapping* mapping, Argument* arg) {
	// NSync repetitions of the Si symbol
	for (short n = 0; n < cfg.NSync; n = n + 1) {
		for (short pos = 0; pos < cfg.CLength; pos = pos + 1) {
			if (C31[Ci - 1][pos] != 0) {
				if(n==0 && pos==0) {
					// we slide the first pulse slightly in time to get the first point "inside" the signal
				  arg->setTime(1E-12 + n * cfg.sync_symbol_duration + pos * cfg.spreadingdL * cfg.pulse_duration);
				} else {
				  arg->setTime(n * cfg.sync_symbol_duration + pos * cfg.spreadingdL * cfg.pulse_duration);
				}
				//generatePulse(mapping, arg, C31[Ci - 1][pos],
				//		IEEE802154A::maxPulse, IEEE802154A::mandatory_pulse);
				generatePulse(mapping, arg, 1,			// always positive polarity
						IEEE802154A::maxPulse, cfg.pulse_duration);
			}
		}
	}
}

void IEEE802154A::generateSFD(Mapping* mapping, Argument* arg) {
	double sfdStart = NSync * Tpsym;
	for (short n = 0; n < 8; n = n + 1) {
		if (IEEE802154A::shortSFD[n] != 0) {
			for (short pos = 0; pos < cfg.CLength; pos = pos + 1) {
				if (C31[Ci - 1][pos] != 0) {
					arg->setTime(sfdStart + n*cfg.sync_symbol_duration + pos*cfg.spreadingdL*cfg.pulse_duration);
					//generatePulse(mapping, arg, C31[Ci - 1][pos] * shortSFD[n]); // change pulse polarity
					generatePulse(mapping, arg, 1); // always positive polarity
				}
			}
		}
	}
}

void IEEE802154A::generatePhyHeader(Mapping* /*mapping*/, Argument* /*arg*/) {
	// not implemented
}

void IEEE802154A::generatePulse(Mapping* mapping, Argument* arg,
		short polarity, double peak, simtime_t_cref chip) {
	assert(polarity == -1 || polarity == +1);
	arg->setTime(arg->getTime() + IEEE802154A::signalStart);  // adjust argument so that we use absolute time values in Mapping
	mapping->setValue(*arg, 0);
	arg->setTime(arg->getTime() + chip / 2);
	// Maximum point at symbol half (triangular pulse)
	mapping->setValue(*arg, peak * polarity);
	arg->setTime(arg->getTime() + chip / 2);
	mapping->setValue(*arg, 0);
}

void IEEE802154A::generateBurst(Mapping* mapping, Argument* arg,
		simtime_t_cref burstStart, short /*polarity*/) {
	assert(burstStart < cfg.preambleLength+(psduLength*8+48+2)*cfg.data_symbol_duration);
	// 1. Start point = zeros
	simtime_t offset = burstStart;
	for (int pulse = 0; pulse < cfg.nbPulsesPerBurst; pulse++) {
		arg->setTime(offset);
		generatePulse(mapping, arg, 1);
		offset = offset + cfg.pulse_duration;
	}
}

void IEEE802154A::setBitRate(Signal* s) {
	Argument arg = Argument();
	// set a constant value for bitrate
	TimeMapping<Linear>* bitrate = new TimeMapping<Linear> ();
	arg.setTime(IEEE802154A::signalStart); // absolute time (required for compatibility with MiXiM base RSAM code)
	bitrate->setValue(arg, cfg.bitrate);
	arg.setTime(s->getDuration());
	bitrate->setValue(arg, cfg.bitrate);
	s->setBitrate(bitrate);
}

simtime_t IEEE802154A::getThdr() {
	switch (cfg.channel) {
	default:
		switch (cfg.prf) {
		case NOMINAL_4_M:
			//error("This optional mode is not implemented.");
			return 0;
			break;
		case NOMINAL_16_M:
			return 16.4E-6;
		case NOMINAL_64_M:
			return 16.8E-6;
		case PRF_OFF:
			return 0;
		}
		break;
	}
	return 0;
}

int IEEE802154A::s(int n) {

	assert(n < maxS);

	for (; last_s < n; last_s = last_s + 1) {
		// compute missing values as necessary
		s_array[last_s] = (s_array[last_s - 14] + s_array[last_s - 15]) % 2;
	}
	assert(s_array[n] == 0 || s_array[n] == 1);
	return s_array[n];

}

int IEEE802154A::getHoppingPos(int sym) {
	//int m = 3;  // or 5 with 4M
	int pos = 0;
	int kNcpb = 0;
	switch(cfg.prf) {
	case NOMINAL_4_M:
		kNcpb = sym * cfg.Ncpb;
		pos = s(kNcpb) + 2*s(1+kNcpb) + 4*s(2+kNcpb) + 8*s(3+kNcpb) + 16*s(4+kNcpb);
		break;
	case NOMINAL_16_M:
		pos = s(kNcpb) + 2*s(1+kNcpb) + 4*s(2+kNcpb);
		break;
	case NOMINAL_64_M:
	case PRF_OFF:
	default:
		assert(0==1);  // unimplemented or invalid PRF value
		break;
	}
	// assert(pos > -1 && pos < 8); // TODO: update to reflect number of hopping pos for current config
	return pos;
}

simtime_t IEEE802154A::getPhyMaxFrameDuration() {
	simtime_t phyMaxFrameDuration = SIMTIME_ZERO;
	simtime_t TSHR, TPHR, TPSDU, TCCApreamble;
	TSHR = IEEE802154A::getThdr();
	TPHR = IEEE802154A::getThdr();
	phyMaxFrameDuration = phyMaxFrameDuration + TSHR;
	return phyMaxFrameDuration;
}
