/* -*- mode:c++ -*- ********************************************************
 * file:        IEEE802154A.h
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

#ifndef _IEEE802154A_H
#define	_IEEE802154A_H

#include <vector>
#include <utility>

#include "MiXiMDefs.h"
#include "Signal_.h"

/**
 * @brief This class regroups static methods needed to generate
 * a pulse-level representation of an IEEE 802.15.4A UWB PHY frame
 * using the mandatory mode (high PRF).
 *
 *  The main function of interest is
 * static signalAndData generateIEEE802154AUWBSignal(simtime_t signalStart, bool allZeros=false).
 *
 * @ingroup ieee802154a
 */
class MIXIM_API IEEE802154A
{
    public:
        /**@brief bit rate (850 kbps) */
        static const int mandatory_bitrate;
        /**@brief mandatory data symbol length (1025 ns) */
        static const_simtime_t mandatory_symbol;
        /**@brief 0.5 * mandatory_symbol (0.5 ms) */
        static const_simtime_t mandatory_timeShift;
        /**@brief mandatory pulse duration ( = 1 / bandwidth = 2 ns) */
        static const_simtime_t mandatory_pulse;
        /**@brief burst duration */
        static const_simtime_t mandatory_burst;
        /**@brief number of consecutive pulses forming a burst */
        static const int mandatory_pulses_per_burst;
        /**@brief Center frequency of band 3 in UWB lower band (500 MHz wide channel) */
        static const double mandatory_centerFreq; // in MHz !
        /**@brief default sync preamble length */
        static const_simtime_t mandatory_preambleLength;
        /**@brief Total triangular pulse peak energy in mW (0 dBm / 50 MHz over 500 MHz) */
        static const double maxPulse;

        static const short C31[8][31];

        static const short Ci = 5;

        static const short shortSFD[8];

        static const_simtime_t MaxFrameDuration;

        static const int maxS = 20000;
        static short s_array[maxS];
        static int last_s;

        static double signalStart; // we cannot use a simtime_t here because the scale exponent is not yet known at initialization.

        /**@brief Number of Repetitions of the sync symbol in the SYNC preamble */
        static const int NSync = 64; // default sync preamble length
        /**@brief Length of the preamble code */
        static const short CLength = 31;
        /**@brief sync preamble spreading factor L */
        static const short spreadingdL = 16;
        /**@brief duration of a synchronization preamble symbol */
        static const_simtime_t Tpsym;

        /**@brief bit length of a Reed-Solomon symbol */
        static const short RSSymbolLength = 6;
        /**@brief Maximum number of erroneous symbols that the Reed-Solomon code RS_6(63,55) can correct in ieee802.15.4a */
        static const short RSMaxSymbolErrors = 4; // =(n-k)/2

        /**@brief Maximum size of message that is accepted by the Phy layer (in bytes). */
        static const int MaxPSDULength = 128;

        /**@brief Position of the first pulse max in the frame. */
        static const_simtime_t tFirstSyncPulseMax;

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

    public:
        /**@brief currently unused */
        struct config
        {
                int channel;
                UWBPRF prf;
                Ranging ranging;
                UWBPreambleSymbolRepetitions NSync;
                int CLength;
                int spreadingdL; // spreading deltaL
                int Ncpb;
                int bitrate;
                int nbPulsesPerBurst;

//    	simtime_t sync_symbol_duration;
//    	simtime_t data_symbol_duration;
//    	simtime_t shift_duration;
//    	simtime_t pulse_duration;
//    	simtime_t burst_duration;
//    	simtime_t preambleLength;
                double sync_symbol_duration;
                double data_symbol_duration;
                double shift_duration;
                double pulse_duration;
                double burst_duration;
                double preambleLength;

                double centerFrequency;
        };

        typedef std::pair<Signal *, std::vector<bool> *> signalAndData;

        /* @brief Sets the configuration of the IEEE802.15.4A standard.
         *  Use this (and the struct config) to implement optional modes of the standard. */
        static void setConfig(config newCfg);

        static config getConfig()
        {
            return cfg;
        }

        // sets the number of data bytes to encode in the data structure.
        static void setPSDULength(int _psduLength);

        /* @brief Generates a frame starting at time signalStart and composed of
         * psduLength bytes of data (this value must be set before by using setPSDULength()).
         * If allZeros is set to true, all bit values are equal to zero.
         * If it is set to false or undefined, bit values are generated randomly.
         * The returned structure is a pair. Its first component is the Signal*,
         * and the second component is a vector of bool with the the generated bit values.
         * */
        static signalAndData generateIEEE802154AUWBSignal(simtime_t_cref signalStart, bool allZeros = false);

        static simtime_t getMaxFrameDuration();

// Constants from standard
        /* @brief Always 16 symbols for the PHY header */
        static const int Nhdr = 16;

    protected:
        static void generateSyncPreamble(Mapping* mapping, Argument* arg);
        static void generateSFD(Mapping* mapping, Argument* arg);
        static void generatePhyHeader(Mapping* mapping, Argument* arg);
        static void generateBurst(Mapping* mapping, Argument* arg, simtime_t_cref burstStart, short polarity);
        static void generatePulse(Mapping* mapping, Argument* arg, short polarity, double peak = IEEE802154A::maxPulse,
                simtime_t_cref chip = IEEE802154A::mandatory_pulse);
        static void setBitRate(Signal* s);
        static int s(int n);

    public:
        static int psduLength;
        static config cfg;
        static const config cfg_mandatory_16M;
        static const config cfg_mandatory_4M;

        // Compute derived parameters
        static simtime_t getPhyMaxFrameDuration();
        static simtime_t getThdr();
        static int getHoppingPos(int sym);

};

#endif	/* _IEEE802154A_H */

