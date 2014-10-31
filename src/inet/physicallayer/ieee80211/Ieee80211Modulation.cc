/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "inet/physicallayer/ieee80211/Ieee80211Modulation.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211DataRate.h"

namespace inet {

namespace physicallayer {

ModulationType Ieee80211Modulation::GetDsssRate1Mbps()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_DSSS);
    mode.setBandwidth(22000000);
    mode.setDataRate(1000000);
    mode.setCodeRate(CODE_RATE_UNDEFINED);
    mode.setConstellationSize(2);
    mode.setIsMandatory(true);
    return mode;
}

ModulationType Ieee80211Modulation::GetDsssRate2Mbps()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_DSSS);
    mode.setBandwidth(22000000);
    mode.setDataRate(2000000);
    mode.setCodeRate(CODE_RATE_UNDEFINED);
    mode.setConstellationSize(2);
    mode.setIsMandatory(true);
    return mode;
}

/**
 * Clause 18 rates (HR/DSSS)
 */
ModulationType Ieee80211Modulation::GetDsssRate5_5Mbps()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_DSSS);
    mode.setBandwidth(22000000);
    mode.setDataRate(5500000);
    mode.setCodeRate(CODE_RATE_UNDEFINED);
    mode.setConstellationSize(4);
    mode.setIsMandatory(true);
    return mode;
}

ModulationType Ieee80211Modulation::GetDsssRate11Mbps()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_DSSS);
    mode.setBandwidth(22000000);
    mode.setDataRate(11000000);
    mode.setCodeRate(CODE_RATE_UNDEFINED);
    mode.setConstellationSize(4);
    mode.setIsMandatory(true);
    return mode;
}

/**
 * Clause 19.5 rates (ERP-OFDM)
 */
ModulationType Ieee80211Modulation::GetErpOfdmRate6Mbps()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_ERP_OFDM);
    mode.setBandwidth(20000000);
    mode.setDataRate(6000000);
    mode.setCodeRate(CODE_RATE_1_2);
    mode.setConstellationSize(2);
    mode.setIsMandatory(true);
    return mode;
}

ModulationType Ieee80211Modulation::GetErpOfdmRate9Mbps()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_ERP_OFDM);
    mode.setBandwidth(20000000);
    mode.setDataRate(9000000);
    mode.setCodeRate(CODE_RATE_3_4);
    mode.setConstellationSize(2);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType Ieee80211Modulation::GetErpOfdmRate12Mbps()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_ERP_OFDM);
    mode.setBandwidth(20000000);
    mode.setDataRate(12000000);
    mode.setCodeRate(CODE_RATE_1_2);
    mode.setConstellationSize(4);
    mode.setIsMandatory(true);
    return mode;
}

ModulationType Ieee80211Modulation::GetErpOfdmRate18Mbps()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_ERP_OFDM);
    mode.setBandwidth(20000000);
    mode.setDataRate(18000000);
    mode.setCodeRate(CODE_RATE_3_4);
    mode.setConstellationSize(4);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType Ieee80211Modulation::GetErpOfdmRate24Mbps()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_ERP_OFDM);
    mode.setBandwidth(20000000);
    mode.setDataRate(24000000);
    mode.setCodeRate(CODE_RATE_1_2);
    mode.setConstellationSize(16);
    mode.setIsMandatory(true);
    return mode;
}

ModulationType Ieee80211Modulation::GetErpOfdmRate36Mbps()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_ERP_OFDM);
    mode.setBandwidth(20000000);
    mode.setDataRate(36000000);
    mode.setCodeRate(CODE_RATE_3_4);
    mode.setConstellationSize(16);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType Ieee80211Modulation::GetErpOfdmRate48Mbps()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_ERP_OFDM);
    mode.setBandwidth(20000000);
    mode.setDataRate(48000000);
    mode.setCodeRate(CODE_RATE_2_3);
    mode.setConstellationSize(64);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType Ieee80211Modulation::GetErpOfdmRate54Mbps()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_ERP_OFDM);
    mode.setBandwidth(20000000);
    mode.setDataRate(54000000);
    mode.setCodeRate(CODE_RATE_3_4);
    mode.setConstellationSize(64);
    mode.setIsMandatory(false);
    return mode;
}

/**
 * Clause 17 rates (OFDM)
 */
ModulationType Ieee80211Modulation::GetOfdmRate6Mbps()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_OFDM);
    mode.setBandwidth(20000000);
    mode.setDataRate(6000000);
    mode.setCodeRate(CODE_RATE_1_2);
    mode.setConstellationSize(2);
    mode.setIsMandatory(true);
    return mode;
}

ModulationType Ieee80211Modulation::GetOfdmRate9Mbps()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_OFDM);
    mode.setBandwidth(20000000);
    mode.setDataRate(9000000);
    mode.setCodeRate(CODE_RATE_UNDEFINED);
    mode.setConstellationSize(2);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType Ieee80211Modulation::GetOfdmRate12Mbps()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_OFDM);
    mode.setBandwidth(20000000);
    mode.setDataRate(12000000);
    mode.setCodeRate(CODE_RATE_UNDEFINED);
    mode.setConstellationSize(4);
    mode.setIsMandatory(true);
    return mode;
}

ModulationType Ieee80211Modulation::GetOfdmRate18Mbps()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_OFDM);
    mode.setBandwidth(20000000);
    mode.setDataRate(18000000);
    mode.setCodeRate(CODE_RATE_UNDEFINED);
    mode.setConstellationSize(4);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType Ieee80211Modulation::GetOfdmRate24Mbps()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_OFDM);
    mode.setBandwidth(20000000);
    mode.setDataRate(24000000);
    mode.setCodeRate(CODE_RATE_1_2);
    mode.setConstellationSize(16);
    mode.setIsMandatory(true);
    return mode;
}

ModulationType Ieee80211Modulation::GetOfdmRate36Mbps()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_OFDM);
    mode.setBandwidth(20000000);
    mode.setDataRate(36000000);
    mode.setCodeRate(CODE_RATE_3_4);
    mode.setConstellationSize(16);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType Ieee80211Modulation::GetOfdmRate48Mbps()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_OFDM);
    mode.setBandwidth(20000000);
    mode.setDataRate(48000000);
    mode.setCodeRate(CODE_RATE_2_3);
    mode.setConstellationSize(64);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType Ieee80211Modulation::GetOfdmRate54Mbps()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_OFDM);
    mode.setBandwidth(20000000);
    mode.setDataRate(54000000);
    mode.setCodeRate(CODE_RATE_3_4);
    mode.setConstellationSize(64);
    mode.setIsMandatory(false);
    return mode;
}

/* 10 MHz channel rates */
ModulationType Ieee80211Modulation::GetOfdmRate3MbpsBW10MHz()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_OFDM);
    mode.setBandwidth(10000000);
    mode.setDataRate(3000000);
    mode.setCodeRate(CODE_RATE_1_2);
    mode.setConstellationSize(2);
    mode.setIsMandatory(true);
    return mode;
}

ModulationType Ieee80211Modulation::GetOfdmRate4_5MbpsBW10MHz()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_OFDM);
    mode.setBandwidth(10000000);
    mode.setDataRate(4500000);
    mode.setCodeRate(CODE_RATE_3_4);
    mode.setConstellationSize(2);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType Ieee80211Modulation::GetOfdmRate6MbpsBW10MHz()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_OFDM);
    mode.setBandwidth(10000000);
    mode.setDataRate(6000000);
    mode.setCodeRate(CODE_RATE_1_2);
    mode.setConstellationSize(4);
    mode.setIsMandatory(true);
    return mode;
}

ModulationType Ieee80211Modulation::GetOfdmRate9MbpsBW10MHz()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_OFDM);
    mode.setBandwidth(10000000);
    mode.setDataRate(9000000);
    mode.setCodeRate(CODE_RATE_3_4);
    mode.setConstellationSize(4);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType Ieee80211Modulation::GetOfdmRate12MbpsBW10MHz()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_OFDM);
    mode.setBandwidth(10000000);
    mode.setDataRate(12000000);
    mode.setCodeRate(CODE_RATE_1_2);
    mode.setConstellationSize(16);
    mode.setIsMandatory(true);
    return mode;
}

ModulationType Ieee80211Modulation::GetOfdmRate18MbpsBW10MHz()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_OFDM);
    mode.setBandwidth(10000000);
    mode.setDataRate(18000000);
    mode.setCodeRate(CODE_RATE_3_4);
    mode.setConstellationSize(16);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType Ieee80211Modulation::GetOfdmRate24MbpsBW10MHz()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_OFDM);
    mode.setBandwidth(10000000);
    mode.setDataRate(24000000);
    mode.setCodeRate(CODE_RATE_2_3);
    mode.setConstellationSize(64);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType Ieee80211Modulation::GetOfdmRate27MbpsBW10MHz()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_OFDM);
    mode.setBandwidth(10000000);
    mode.setDataRate(27000000);
    mode.setCodeRate(CODE_RATE_3_4);
    mode.setConstellationSize(64);
    mode.setIsMandatory(false);
    return mode;
}

/* 5 MHz channel rates */
ModulationType Ieee80211Modulation::GetOfdmRate1_5MbpsBW5MHz()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_OFDM);
    mode.setBandwidth(5000000);
    mode.setDataRate(1500000);
    mode.setCodeRate(CODE_RATE_1_2);
    mode.setConstellationSize(2);
    mode.setIsMandatory(true);
    return mode;
}

ModulationType Ieee80211Modulation::GetOfdmRate2_25MbpsBW5MHz()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_OFDM);
    mode.setBandwidth(5000000);
    mode.setDataRate(2250000);
    mode.setCodeRate(CODE_RATE_3_4);
    mode.setConstellationSize(2);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType Ieee80211Modulation::GetOfdmRate3MbpsBW5MHz()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_OFDM);
    mode.setBandwidth(5000000);
    mode.setDataRate(3000000);
    mode.setCodeRate(CODE_RATE_1_2);
    mode.setConstellationSize(4);
    mode.setIsMandatory(true);
    return mode;
}

ModulationType Ieee80211Modulation::GetOfdmRate4_5MbpsBW5MHz()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_OFDM);
    mode.setBandwidth(5000000);
    mode.setDataRate(4500000);
    mode.setCodeRate(CODE_RATE_3_4);
    mode.setConstellationSize(4);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType Ieee80211Modulation::GetOfdmRate6MbpsBW5MHz()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_OFDM);
    mode.setBandwidth(5000000);
    mode.setDataRate(6000000);
    mode.setCodeRate(CODE_RATE_1_2);
    mode.setConstellationSize(16);
    mode.setIsMandatory(true);
    return mode;
}

ModulationType Ieee80211Modulation::GetOfdmRate9MbpsBW5MHz()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_OFDM);
    mode.setBandwidth(5000000);
    mode.setDataRate(9000000);
    mode.setCodeRate(CODE_RATE_3_4);
    mode.setConstellationSize(16);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType Ieee80211Modulation::GetOfdmRate12MbpsBW5MHz()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_OFDM);
    mode.setBandwidth(5000000);
    mode.setDataRate(12000000);
    mode.setCodeRate(CODE_RATE_2_3);
    mode.setConstellationSize(64);
    mode.setIsMandatory(false);
    return mode;
}

ModulationType Ieee80211Modulation::GetOfdmRate13_5MbpsBW5MHz()
{
    ModulationType mode;
    mode.setModulationClass(MOD_CLASS_OFDM);
    mode.setBandwidth(5000000);
    mode.setDataRate(13500000);
    mode.setCodeRate(CODE_RATE_3_4);
    mode.setConstellationSize(64);
    mode.setIsMandatory(false);
    return mode;
}

simtime_t Ieee80211Modulation::getPlcpHeaderDuration(ModulationType payloadMode, Ieee80211PreambleMode preamble)
{
    switch (payloadMode.getModulationClass()) {
        case MOD_CLASS_OFDM: {
            switch (payloadMode.getBandwidth()) {
                case 20000000:
                default:
                    // IEEE Std 802.11-2007, section 17.3.3 and figure 17-4
                    // also section 17.3.2.3, table 17-4
                    // We return the duration of the SIGNAL field only, since the
                    // SERVICE field (which strictly speaking belongs to the PLCP
                    // header, see section 17.3.2 and figure 17-1) is sent using the
                    // payload mode.
                    return 4.0 / 1000000.0;

                case 10000000:
                    // IEEE Std 802.11-2007, section 17.3.2.3, table 17-4
                    return 8 / 1000000.0;

                case 5000000:
                    // IEEE Std 802.11-2007, section 17.3.2.3, table 17-4
                    return 16.0 / 1000000.0;
            }
            break;
        }

        case MOD_CLASS_ERP_OFDM:
            return 16.0 / 1000000.0;

        case MOD_CLASS_DSSS:
            if (preamble == IEEE80211_PREAMBLE_SHORT) {
                // IEEE Std 802.11-2007, section 18.2.2.2 and figure 18-2
                return 24.0 / 1000000.0;
            }
            else {    // IEEE80211_PREAMBLE_LONG
                      // IEEE Std 802.11-2007, sections 18.2.2.1 and figure 18-1
                return 48.0 / 1000000.0;
            }

        default:
            throw cRuntimeError("unsupported modulation class");
            return 0;
    }
}

simtime_t Ieee80211Modulation::getPlcpPreambleDuration(ModulationType payloadMode, Ieee80211PreambleMode preamble)
{
    switch (payloadMode.getModulationClass()) {
        case MOD_CLASS_OFDM: {
            switch (payloadMode.getBandwidth()) {
                case 20000000:
                default:
                    // IEEE Std 802.11-2007, section 17.3.3,  figure 17-4
                    // also section 17.3.2.3, table 17-4
                    return 16.0 / 1000000.0;

                case 10000000:
                    // IEEE Std 802.11-2007, section 17.3.3, table 17-4
                    // also section 17.3.2.3, table 17-4
                    return 32.0 / 1000000.0;

                case 5000000:
                    // IEEE Std 802.11-2007, section 17.3.3
                    // also section 17.3.2.3, table 17-4
                    return 64.0 / 1000000.0;
            }
            break;
        }

        case MOD_CLASS_ERP_OFDM:
            return 4.0 / 1000000.0;

        case MOD_CLASS_DSSS:
            if (preamble == IEEE80211_PREAMBLE_SHORT) {
                // IEEE Std 802.11-2007, section 18.2.2.2 and figure 18-2
                return 72.0 / 1000000.0;
            }
            else {    // IEEE80211_PREAMBLE_LONG
                      // IEEE Std 802.11-2007, sections 18.2.2.1 and figure 18-1
                return 144.0 / 1000000.0;
            }

        default:
            throw cRuntimeError("unsupported modulation class");
            return 0;
    }
}

//
// Compute the Payload duration in function of the modulation type
//
simtime_t Ieee80211Modulation::getPayloadDuration(uint64_t size, ModulationType payloadMode)
{
    simtime_t val;
    switch (payloadMode.getModulationClass()) {
        case MOD_CLASS_OFDM:
        case MOD_CLASS_ERP_OFDM: {
            // IEEE Std 802.11-2007, section 17.3.2.3, table 17-4
            // corresponds to T_{SYM} in the table
            uint32_t symbolDurationUs;
            switch (payloadMode.getBandwidth()) {
                case 20000000:
                default:
                    symbolDurationUs = 4;
                    break;

                case 10000000:
                    symbolDurationUs = 8;
                    break;

                case 5000000:
                    symbolDurationUs = 16;
                    break;
            }
            // IEEE Std 802.11-2007, section 17.3.2.2, table 17-3
            // corresponds to N_{DBPS} in the table
            double numDataBitsPerSymbol = payloadMode.getDataRate() * symbolDurationUs / 1e6;
            // IEEE Std 802.11-2007, section 17.3.5.3, equation (17-11)
            uint32_t numSymbols = lrint(ceil((16 + size + 6.0) / numDataBitsPerSymbol));

            // Add signal extension for ERP PHY
            double aux;
            if (payloadMode.getModulationClass() == MOD_CLASS_ERP_OFDM)
                aux = numSymbols * symbolDurationUs + 6;
            else
                aux = numSymbols * symbolDurationUs;
            val = (aux / 1000000.0);
            return val;
        }

        case MOD_CLASS_DSSS:
            // IEEE Std 802.11-2007, section 18.2.3.5
            double aux;
            aux = lrint(ceil((size) / (payloadMode.getDataRate() / 1.0e6)));
            val = (aux / 1000000.0);
            return val;
            break;

        default:
            throw cRuntimeError("unsupported modulation class");
            return 0;
    }
}

//
// Return the physical header duration, useful for the mac
//
simtime_t Ieee80211Modulation::getPreambleAndHeader(ModulationType payloadMode, Ieee80211PreambleMode preamble)
{
    return getPlcpPreambleDuration(payloadMode, preamble) + getPlcpHeaderDuration(payloadMode, preamble);
}

simtime_t Ieee80211Modulation::calculateTxDuration(uint64_t size, ModulationType payloadMode, Ieee80211PreambleMode preamble)
{
    simtime_t duration = getPlcpPreambleDuration(payloadMode, preamble)
        + getPlcpHeaderDuration(payloadMode, preamble)
        + getPayloadDuration(size, payloadMode);
    return duration;
}

ModulationType Ieee80211Modulation::getPlcpHeaderMode(ModulationType payloadMode, Ieee80211PreambleMode preamble)
{
    switch (payloadMode.getModulationClass()) {
        case MOD_CLASS_OFDM: {
            switch (payloadMode.getBandwidth()) {
                case 5000000:
                    return Ieee80211Modulation::GetOfdmRate1_5MbpsBW5MHz();

                case 10000000:
                    return Ieee80211Modulation::GetOfdmRate3MbpsBW10MHz();

                default:
                    // IEEE Std 802.11-2007, 17.3.2
                    // actually this is only the first part of the PlcpHeader,
                    // because the last 16 bits of the PlcpHeader are using the
                    // same mode of the payload
                    return Ieee80211Modulation::GetOfdmRate6Mbps();
            }
            break;
        }

        case MOD_CLASS_ERP_OFDM:
            return Ieee80211Modulation::GetErpOfdmRate6Mbps();

        case MOD_CLASS_DSSS:
            if (preamble == IEEE80211_PREAMBLE_LONG) {
                // IEEE Std 802.11-2007, sections 15.2.3 and 18.2.2.1
                return Ieee80211Modulation::GetDsssRate1Mbps();
            }
            else {    // IEEE80211_PREAMBLE_SHORT
                      // IEEE Std 802.11-2007, section 18.2.2.2
                return Ieee80211Modulation::GetDsssRate2Mbps();
            }

        default:
            throw cRuntimeError("unsupported modulation class");
            return ModulationType();
    }
}

simtime_t Ieee80211Modulation::getSlotDuration(ModulationType modType, Ieee80211PreambleMode preamble)
{
    switch (modType.getModulationClass()) {
        case MOD_CLASS_OFDM: {
            switch (modType.getBandwidth()) {
                case 5000000:
                    return 21.0 / 1000000.0;

                case 10000000:
                    return 13.0 / 1000000.0;

                default:
                    // IEEE Std 802.11-2007, 17.3.2
                    // actually this is only the first part of the PlcpHeader,
                    // because the last 16 bits of the PlcpHeader are using the
                    // same mode of the payload
                    return 9.0 / 1000000.0;
            }
            break;
        }

        case MOD_CLASS_ERP_OFDM:
            if (preamble == IEEE80211_PREAMBLE_LONG) {
                // IEEE Std 802.11-2007, sections 15.2.3 and 18.2.2.1
                return 20.0 / 1000000.0;
            }
            else {    // IEEE80211_PREAMBLE_SHORT
                      // IEEE Std 802.11-2007, section 18.2.2.2
                return 9.0 / 1000000.0;
            }

        case MOD_CLASS_DSSS:
            return 20.0 / 1000000.0;

        default:
            throw cRuntimeError("unsupported modulation class");
            return SIMTIME_ZERO;
    }
}

simtime_t Ieee80211Modulation::getSifsTime(ModulationType modType, Ieee80211PreambleMode preamble)
{
    switch (modType.getModulationClass()) {
        case MOD_CLASS_OFDM: {
            switch (modType.getBandwidth()) {
                case 5000000:
                    return 64.0 / 1000000.0;

                case 10000000:
                    return 32.0 / 1000000.0;

                default:
                    // IEEE Std 802.11-2007, 17.3.2
                    // actually this is only the first part of the PlcpHeader,
                    // because the last 16 bits of the PlcpHeader are using the
                    // same mode of the payload
                    return 16.0 / 1000000;
            }
            break;
        }

        case MOD_CLASS_ERP_OFDM:
            // IEEE Std 802.11-2007, sections 15.2.3 and 18.2.2.1
            return 10.0 / 1000000.0;

        case MOD_CLASS_DSSS:
            return 10.0 / 1000000.0;

        default:
            throw cRuntimeError("unsupported modulation class");
            return SIMTIME_ZERO;
    }
}

simtime_t Ieee80211Modulation::get_aPHY_RX_START_Delay(ModulationType modType, Ieee80211PreambleMode preamble)
{
    switch (modType.getModulationClass()) {
        case MOD_CLASS_OFDM: {
            switch (modType.getBandwidth()) {
                case 5000000:
                    return 97.0 / 1000000.0;

                case 10000000:
                    return 49.0 / 1000000.0;

                default:
                    // IEEE Std 802.11-2007, 17.3.2
                    // actually this is only the first part of the PlcpHeader,
                    // because the last 16 bits of the PlcpHeader are using the
                    // same mode of the payload
                    return 25.0 / 1000000.0;
            }
        }

        case MOD_CLASS_ERP_OFDM:
            // IEEE Std 802.11-2007, section 18.2.2.2
            return 24.0 / 1000000.0;

        case MOD_CLASS_DSSS:
            if (preamble == IEEE80211_PREAMBLE_LONG) {
                // IEEE Std 802.11-2007, sections 15.2.3 and 18.2.2.1
                return 192.0 / 1000000.0;
            }
            else {    // IEEE80211_PREAMBLE_SHORT
                      // IEEE Std 802.11-2007, section 18.2.2.2
                return 96.0 / 1000000.0;
            }

        default:
            throw cRuntimeError("unsupported modulation class");
            return SIMTIME_ZERO;
    }
}

} // namespace physicallayer

} // namespace inet

