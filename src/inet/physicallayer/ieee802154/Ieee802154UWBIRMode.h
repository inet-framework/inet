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

#ifndef __INET_IEEE802154UWBIRMODE_H
#define	__INET_IEEE802154UWBIRMODE_H

#include "inet/physicallayer/base/PhysicalLayerDefs.h"

namespace inet {

namespace physicallayer {

// This class was created by porting some C++ code from the IEEE802154A class in MiXiM.
class INET_API Ieee802154UWBIRMode
{
  public:
    /**@brief Total triangular pulse peak energy in mW (0 dBm / 50 MHz over 500 MHz) */
    static const double maxPulse;
    // mandatory pulse duration ( = 1 / bandwidth = 2 ns)
    static const double mandatory_pulse;
    static const short C31[8][31];
    static const short Ci = 5;
    static const short shortSFD[8];
    /**@brief Maximum size of message that is accepted by the Phy layer (in bytes). */
    static const int MaxPSDULength = 128;
    // TODO: change to non-static, because it depends on the mode
    static const int maxS = 20000;
    static short s_array[maxS];
    static int last_s;
    static const Ieee802154UWBIRMode cfg_mandatory_4M;
    static const Ieee802154UWBIRMode cfg_mandatory_16M;

    enum UWBPRF
    {
        PRF_OFF, NOMINAL_4_M, NOMINAL_16_M, NOMINAL_64_M
    };

    enum Ranging
    {
        NON_RANGING, ALL_RANGING, PHY_HEADER_ONLY
    };

    enum UWBPreambleSymbolRepetitions
    {
        PSR_SHORT = 16, PSR_DEFAULT = 64, PSR_MEDIUM = 1024, PSR_LONG = 4096
    };

    enum DataRate
    {
        DATA_RATE_0, DATA_RATE_1, DATA_RATE_2, DATA_RATE_3, DATA_RATE_4
    };

    int channel;
    UWBPRF prf;
    Ranging ranging;
    UWBPreambleSymbolRepetitions NSync;
    int CLength;
    int spreadingdL; // spreading deltaL
    int Ncpb;
    bps bitrate;
    int nbPulsesPerBurst;
//        simtime_t sync_symbol_duration;
//        simtime_t data_symbol_duration;
//        simtime_t shift_duration;
//        simtime_t pulse_duration;
//        simtime_t burst_duration;
//        simtime_t preambleLength;
    double sync_symbol_duration;
    double data_symbol_duration;
    double shift_duration;
    double pulse_duration;
    double burst_duration;
    double preambleLength;
    Hz centerFrequency;
    Hz bandwidth;

  public:
    int s(int n) const;
    int getHoppingPos(int sym) const;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IEEE802154UWBIRMODE_H

