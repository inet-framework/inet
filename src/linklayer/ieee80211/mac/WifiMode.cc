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
#include "WifiMode.h"
#include "Ieee80211DataRate.h"


ModulationType
WifiModulationType::GetDsssRate1Mbps()
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

ModulationType
WifiModulationType::GetDsssRate2Mbps()
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
ModulationType
WifiModulationType::GetDsssRate5_5Mbps()
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

ModulationType
WifiModulationType::GetDsssRate11Mbps()
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
ModulationType
WifiModulationType::GetErpOfdmRate6Mbps()
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

ModulationType
WifiModulationType::GetErpOfdmRate9Mbps()
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

ModulationType
WifiModulationType::GetErpOfdmRate12Mbps()
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

ModulationType
WifiModulationType::GetErpOfdmRate18Mbps()
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

ModulationType
WifiModulationType::GetErpOfdmRate24Mbps()
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

ModulationType
WifiModulationType::GetErpOfdmRate36Mbps()
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

ModulationType
WifiModulationType::GetErpOfdmRate48Mbps()
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

ModulationType
WifiModulationType::GetErpOfdmRate54Mbps()
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
ModulationType
WifiModulationType::GetOfdmRate6Mbps()
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

ModulationType
WifiModulationType::GetOfdmRate9Mbps()
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

ModulationType
WifiModulationType::GetOfdmRate12Mbps()
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

ModulationType
WifiModulationType::GetOfdmRate18Mbps()
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

ModulationType
WifiModulationType::GetOfdmRate24Mbps()
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

ModulationType
WifiModulationType::GetOfdmRate36Mbps()
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

ModulationType
WifiModulationType::GetOfdmRate48Mbps()
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

ModulationType
WifiModulationType::GetOfdmRate54Mbps()
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
ModulationType
WifiModulationType::GetOfdmRate3MbpsBW10MHz()
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

ModulationType
WifiModulationType::GetOfdmRate4_5MbpsBW10MHz()
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

ModulationType
WifiModulationType::GetOfdmRate6MbpsBW10MHz()
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

ModulationType
WifiModulationType::GetOfdmRate9MbpsBW10MHz()
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

ModulationType
WifiModulationType::GetOfdmRate12MbpsBW10MHz()
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

ModulationType
WifiModulationType::GetOfdmRate18MbpsBW10MHz()
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

ModulationType
WifiModulationType::GetOfdmRate24MbpsBW10MHz()
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

ModulationType
WifiModulationType::GetOfdmRate27MbpsBW10MHz()
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
ModulationType
WifiModulationType::GetOfdmRate1_5MbpsBW5MHz()
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

ModulationType
WifiModulationType::GetOfdmRate2_25MbpsBW5MHz()
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

ModulationType
WifiModulationType::GetOfdmRate3MbpsBW5MHz()
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

ModulationType
WifiModulationType::GetOfdmRate4_5MbpsBW5MHz()
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

ModulationType
WifiModulationType::GetOfdmRate6MbpsBW5MHz()
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

ModulationType
WifiModulationType::GetOfdmRate9MbpsBW5MHz()
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

ModulationType
WifiModulationType::GetOfdmRate12MbpsBW5MHz()
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

ModulationType
WifiModulationType::GetOfdmRate13_5MbpsBW5MHz()
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


ModulationType WifiModulationType::getModulationType(char mode, double bitrate)
{
    int i = Ieee80211Descriptor::getIdx(mode, bitrate);
    return Ieee80211Descriptor::getDescriptor(i).modulationType;
}


simtime_t
WifiModulationType::getPlcpHeaderDuration(ModulationType payloadMode, WifiPreamble preamble)
{
  switch (payloadMode.getModulationClass())
  {
      case MOD_CLASS_OFDM:
      {
          switch (payloadMode.getBandwidth()) {
              case 20000000:
              default:
                  // IEEE Std 802.11-2007, section 17.3.3 and figure 17-4
                  // also section 17.3.2.3, table 17-4
                  // We return the duration of the SIGNAL field only, since the
                  // SERVICE field (which strictly speaking belongs to the PLCP
                  // header, see section 17.3.2 and figure 17-1) is sent using the
                  // payload mode.
                  return 4.0/1000000.0;
              case 10000000:
                  // IEEE Std 802.11-2007, section 17.3.2.3, table 17-4
                  return 8/1000000.0;
              case 5000000:
                  // IEEE Std 802.11-2007, section 17.3.2.3, table 17-4
                  return 16.0/1000000.0;
          }
          break;
      }
      case MOD_CLASS_ERP_OFDM:
          return 16.0/1000000.0;
      case MOD_CLASS_DSSS:
          if (preamble == WIFI_PREAMBLE_SHORT)
          {
              // IEEE Std 802.11-2007, section 18.2.2.2 and figure 18-2
              return 24.0/1000000.0;
          }
          else // WIFI_PREAMBLE_LONG
          {
              // IEEE Std 802.11-2007, sections 18.2.2.1 and figure 18-1
              return 48.0/1000000.0;
          }
      default:
          opp_error("unsupported modulation class");
      return 0;
    }
}

simtime_t
WifiModulationType::getPlcpPreambleDuration(ModulationType payloadMode, WifiPreamble preamble)
{
    switch (payloadMode.getModulationClass())
    {
        case MOD_CLASS_OFDM:
        {
            switch (payloadMode.getBandwidth())
            {
                case 20000000:
                default:
                // IEEE Std 802.11-2007, section 17.3.3,  figure 17-4
                // also section 17.3.2.3, table 17-4
                    return 16.0/1000000.0;
                case 10000000:
                // IEEE Std 802.11-2007, section 17.3.3, table 17-4
                // also section 17.3.2.3, table 17-4
                    return 32.0/1000000.0;
                case 5000000:
                // IEEE Std 802.11-2007, section 17.3.3
                // also section 17.3.2.3, table 17-4
                    return 64.0/1000000.0;
            }
            break;
        }
        case MOD_CLASS_ERP_OFDM:
            return 4.0/1000000.0;
        case MOD_CLASS_DSSS:
            if (preamble == WIFI_PREAMBLE_SHORT)
            {
                // IEEE Std 802.11-2007, section 18.2.2.2 and figure 18-2
                return 72.0/1000000.0;
            }
            else // WIFI_PREAMBLE_LONG
            {
                // IEEE Std 802.11-2007, sections 18.2.2.1 and figure 18-1
                return 144.0/1000000.0;
            }
        default:
            opp_error("unsupported modulation class");
        return 0;
    }
}
//
// Compute the Payload duration in function of the modulation type
//
simtime_t
WifiModulationType::getPayloadDuration(uint64_t size, ModulationType payloadMode)
{
    simtime_t val;
    switch (payloadMode.getModulationClass())
    {
        case MOD_CLASS_OFDM:
        case MOD_CLASS_ERP_OFDM:
        {
            // IEEE Std 802.11-2007, section 17.3.2.3, table 17-4
            // corresponds to T_{SYM} in the table
            uint32_t symbolDurationUs;
            switch (payloadMode.getBandwidth())
            {
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
            double numDataBitsPerSymbol = payloadMode.getDataRate()  * symbolDurationUs / 1e6;
            // IEEE Std 802.11-2007, section 17.3.5.3, equation (17-11)
            uint32_t numSymbols = lrint(ceil((16 + size + 6.0)/numDataBitsPerSymbol));

            // Add signal extension for ERP PHY
            double aux;
            if (payloadMode.getModulationClass() == MOD_CLASS_ERP_OFDM)
                aux = numSymbols*symbolDurationUs + 6;
            else
                aux = numSymbols*symbolDurationUs;
            val = (aux/1000000);
            return val;
        }
        case MOD_CLASS_DSSS:
            // IEEE Std 802.11-2007, section 18.2.3.5
            double aux;
            aux = lrint(ceil((size) / (payloadMode.getDataRate() / 1.0e6)));
            val = (aux/1000000);
            return val;
            break;
        default:
            opp_error("unsupported modulation class");
            return 0;
    }
}

//
// Return the physical header duration, useful for the mac
//
simtime_t
WifiModulationType::getPreambleAndHeader(ModulationType payloadMode, WifiPreamble preamble)
{
    return (getPlcpPreambleDuration(payloadMode, preamble)+ getPlcpHeaderDuration(payloadMode, preamble));
}

simtime_t
WifiModulationType::calculateTxDuration(uint64_t size, ModulationType payloadMode, WifiPreamble preamble)
{
    simtime_t duration = getPlcpPreambleDuration(payloadMode, preamble)
                       + getPlcpHeaderDuration(payloadMode, preamble)
                       + getPayloadDuration(size, payloadMode);
    return duration;
}

ModulationType
WifiModulationType::getPlcpHeaderMode(ModulationType payloadMode, WifiPreamble preamble)
{
    switch (payloadMode.getModulationClass())
    {
        case MOD_CLASS_OFDM:
        {
            switch (payloadMode.getBandwidth())
            {
                case 5000000:
                    return WifiModulationType::GetOfdmRate1_5MbpsBW5MHz();
                case 10000000:
                    return WifiModulationType::GetOfdmRate3MbpsBW10MHz();
                default:
                // IEEE Std 802.11-2007, 17.3.2
                // actually this is only the first part of the PlcpHeader,
                // because the last 16 bits of the PlcpHeader are using the
                // same mode of the payload
                    return WifiModulationType::GetOfdmRate6Mbps();
            }
            break;
        }
        case MOD_CLASS_ERP_OFDM:
            return WifiModulationType::GetErpOfdmRate6Mbps();
        case MOD_CLASS_DSSS:
            if (preamble == WIFI_PREAMBLE_LONG)
            {
                // IEEE Std 802.11-2007, sections 15.2.3 and 18.2.2.1
                return WifiModulationType::GetDsssRate1Mbps();
            }
            else //  WIFI_PREAMBLE_SHORT
            {
                // IEEE Std 802.11-2007, section 18.2.2.2
                return WifiModulationType::GetDsssRate2Mbps();
            }
       default:
           opp_error("unsupported modulation class");
           return ModulationType();
     }
}


simtime_t
WifiModulationType::getSlotDuration(ModulationType modType, WifiPreamble preamble)
{
    switch (modType.getModulationClass())
    {
        case MOD_CLASS_OFDM:
        {
            switch (modType.getBandwidth())
            {
                case 5000000:
                    return (21/1000000);
                case 10000000:
                    return (13/1000000);
                default:
                    // IEEE Std 802.11-2007, 17.3.2
                    // actually this is only the first part of the PlcpHeader,
                    // because the last 16 bits of the PlcpHeader are using the
                    // same mode of the payload
                    return (9/1000000);
            }
            break;
        }
        case MOD_CLASS_ERP_OFDM:
            if (preamble == WIFI_PREAMBLE_LONG)
            {
                // IEEE Std 802.11-2007, sections 15.2.3 and 18.2.2.1
                return (20/1000000);
            }
            else //  WIFI_PREAMBLE_SHORT
            {
                // IEEE Std 802.11-2007, section 18.2.2.2
                return (9/1000000);
            }
        case MOD_CLASS_DSSS:
            return (20/1000000);
        default:
            opp_error("unsupported modulation class");
            return 0;
    }
}

simtime_t
WifiModulationType::getSifsTime(ModulationType modType, WifiPreamble preamble)
{
    switch (modType.getModulationClass())
    {
        case MOD_CLASS_OFDM:
        {
            switch (modType.getBandwidth())
            {
                case 5000000:
                    return (64/1000000);
                case 10000000:
                    return (32/1000000);
                default:
                    // IEEE Std 802.11-2007, 17.3.2
                    // actually this is only the first part of the PlcpHeader,
                    // because the last 16 bits of the PlcpHeader are using the
                    // same mode of the payload
                    return (16/1000000);
            }
            break;
        }
        case MOD_CLASS_ERP_OFDM:
            // IEEE Std 802.11-2007, sections 15.2.3 and 18.2.2.1
            return (10/1000000);
        case MOD_CLASS_DSSS:
            return (10/1000000);
        default:
            opp_error("unsupported modulation class");
        return 0;
    }
}

simtime_t
WifiModulationType::get_aPHY_RX_START_Delay(ModulationType modType, WifiPreamble preamble)
{
    switch (modType.getModulationClass())
    {
        case MOD_CLASS_OFDM:
        {
            switch (modType.getBandwidth())
            {
                case 5000000:
                    return (97/1000000);
                case 10000000:
                    return (49/1000000);
                default:
                    // IEEE Std 802.11-2007, 17.3.2
                    // actually this is only the first part of the PlcpHeader,
                    // because the last 16 bits of the PlcpHeader are using the
                    // same mode of the payload
                    return (25/1000000);
            }
        }
        case MOD_CLASS_ERP_OFDM:
            // IEEE Std 802.11-2007, section 18.2.2.2
            return (24/1000000);
        case MOD_CLASS_DSSS:
            if (preamble == WIFI_PREAMBLE_LONG)
            {
                // IEEE Std 802.11-2007, sections 15.2.3 and 18.2.2.1
                return (192/1000000);
            }
            else //  WIFI_PREAMBLE_SHORT
            {
                // IEEE Std 802.11-2007, section 18.2.2.2
                return (96/1000000);
            }
         default:
             opp_error("unsupported modulation class");
         return 0;
    }
}

