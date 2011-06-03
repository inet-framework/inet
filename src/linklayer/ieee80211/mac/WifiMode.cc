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
WifyModulationType::GetDsssRate1Mbps()
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
WifyModulationType::GetDsssRate2Mbps()
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
WifyModulationType::GetDsssRate5_5Mbps()
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
WifyModulationType::GetDsssRate11Mbps()
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
WifyModulationType::GetErpOfdmRate6Mbps()
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
WifyModulationType::GetErpOfdmRate9Mbps()
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
WifyModulationType::GetErpOfdmRate12Mbps()
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
WifyModulationType::GetErpOfdmRate18Mbps()
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
WifyModulationType::GetErpOfdmRate24Mbps()
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
WifyModulationType::GetErpOfdmRate36Mbps()
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
WifyModulationType::GetErpOfdmRate48Mbps()
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
WifyModulationType::GetErpOfdmRate54Mbps()
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
WifyModulationType::GetOfdmRate6Mbps()
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
WifyModulationType::GetOfdmRate9Mbps()
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
WifyModulationType::GetOfdmRate12Mbps()
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
WifyModulationType::GetOfdmRate18Mbps()
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
WifyModulationType::GetOfdmRate24Mbps()
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
WifyModulationType::GetOfdmRate36Mbps()
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
WifyModulationType::GetOfdmRate48Mbps()
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
WifyModulationType::GetOfdmRate54Mbps()
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
WifyModulationType::GetOfdmRate3MbpsBW10MHz()
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
WifyModulationType::GetOfdmRate4_5MbpsBW10MHz()
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
WifyModulationType::GetOfdmRate6MbpsBW10MHz()
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
WifyModulationType::GetOfdmRate9MbpsBW10MHz()
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
WifyModulationType::GetOfdmRate12MbpsBW10MHz()
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
WifyModulationType::GetOfdmRate18MbpsBW10MHz()
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
WifyModulationType::GetOfdmRate24MbpsBW10MHz()
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
WifyModulationType::GetOfdmRate27MbpsBW10MHz()
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
WifyModulationType::GetOfdmRate1_5MbpsBW5MHz()
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
WifyModulationType::GetOfdmRate2_25MbpsBW5MHz()
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
WifyModulationType::GetOfdmRate3MbpsBW5MHz()
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
WifyModulationType::GetOfdmRate4_5MbpsBW5MHz()
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
WifyModulationType::GetOfdmRate6MbpsBW5MHz()
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
WifyModulationType::GetOfdmRate9MbpsBW5MHz()
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
WifyModulationType::GetOfdmRate12MbpsBW5MHz()
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
WifyModulationType::GetOfdmRate13_5MbpsBW5MHz()
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

ModulationType
WifyModulationType:: getMode80211a(double bitrate)
{
   if (bitrate == BITRATES_80211a[7])
       return GetOfdmRate54Mbps();
   else if (bitrate == BITRATES_80211a[6])
       return GetOfdmRate48Mbps();
   else if (bitrate == BITRATES_80211a[5])
       return GetOfdmRate36Mbps();
   else if (bitrate == BITRATES_80211a[4])
       return GetOfdmRate24Mbps();
   else if (bitrate == BITRATES_80211a[3])
       return GetOfdmRate18Mbps();
   else if (bitrate == BITRATES_80211a[2])
       return GetOfdmRate12Mbps();
   else if (bitrate == BITRATES_80211a[1])
       return GetOfdmRate9Mbps();
   else if (bitrate == BITRATES_80211a[0])
       return GetOfdmRate6Mbps();
   else
       opp_error("mode not valid");
   return ModulationType();
}

ModulationType
WifyModulationType:: getMode80211g(double bitrate)
{
   if (bitrate == BITRATES_80211g[11])
       return GetErpOfdmRate54Mbps();
   else if (bitrate == BITRATES_80211g[10])
       return GetErpOfdmRate48Mbps();
   else if (bitrate == BITRATES_80211g[9])
       return GetErpOfdmRate36Mbps();
   else if (bitrate == BITRATES_80211g[8])
       return GetErpOfdmRate24Mbps();
   else if (bitrate == BITRATES_80211g[7])
       return GetErpOfdmRate18Mbps();
   else if (bitrate == BITRATES_80211g[6])
       return GetErpOfdmRate12Mbps();
   else if (bitrate == BITRATES_80211g[5])
       return GetDsssRate11Mbps();
   else if (bitrate == BITRATES_80211g[4])
       return GetErpOfdmRate9Mbps();
   else if (bitrate == BITRATES_80211g[3])
       return GetErpOfdmRate6Mbps();
   else if (bitrate == BITRATES_80211g[2])
       return GetDsssRate5_5Mbps();
   else if (bitrate == BITRATES_80211g[1])
       return GetDsssRate2Mbps();
   else if (bitrate == BITRATES_80211g[0])
       return GetDsssRate1Mbps();
   else
       opp_error("mode not valid");
   return ModulationType();
}


ModulationType
WifyModulationType:: getMode80211b(double bitrate)
{
   if (bitrate == BITRATES_80211b[3])
       return GetDsssRate11Mbps();
   else if (bitrate == BITRATES_80211b[2])
       return GetDsssRate5_5Mbps();
   else if (bitrate == BITRATES_80211b[1])
       return GetDsssRate2Mbps();
   else if (bitrate == BITRATES_80211b[0])
       return GetDsssRate1Mbps();
   else
       opp_error("mode not valid");
   return ModulationType();
}


ModulationType
WifyModulationType::getMode80211p(double bitrate)
{
   if (bitrate == BITRATES_80211p[7])
       return GetOfdmRate27MbpsBW10MHz();
   else if (bitrate == BITRATES_80211p[6])
       return GetOfdmRate24MbpsBW10MHz();
   else if (bitrate == BITRATES_80211p[5])
       return GetOfdmRate18MbpsBW10MHz();
   else if (bitrate == BITRATES_80211p[4])
       return GetOfdmRate12MbpsBW10MHz();
   else if (bitrate == BITRATES_80211p[3])
       return GetOfdmRate9MbpsBW10MHz();
   else if (bitrate == BITRATES_80211p[2])
       return GetOfdmRate6MbpsBW10MHz();
   else if (bitrate == BITRATES_80211p[1])
       return GetOfdmRate4_5MbpsBW10MHz();
   else if (bitrate == BITRATES_80211p[0])
       return GetOfdmRate3MbpsBW10MHz();
   else
       opp_error("mode not valid");
   return ModulationType();
}

simtime_t
WifyModulationType::getPlcpHeaderDuration(ModulationType payloadMode, WifiPreamble preamble)
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
WifyModulationType::getPlcpPreambleDuration(ModulationType payloadMode, WifiPreamble preamble)
{
  switch (payloadMode.getModulationClass())
    {
    case MOD_CLASS_OFDM:
      {
        switch (payloadMode.getBandwidth()) {
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
WifyModulationType::getPayloadDuration(uint64_t size, ModulationType payloadMode)
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
WifyModulationType::getPreambleAndHeader(ModulationType payloadMode, WifiPreamble preamble)
{
    return (getPlcpPreambleDuration(payloadMode, preamble)+ getPlcpHeaderDuration(payloadMode, preamble));
}

simtime_t
WifyModulationType::calculateTxDuration(uint64_t size, ModulationType payloadMode, WifiPreamble preamble)
{
  simtime_t duration = getPlcpPreambleDuration(payloadMode, preamble)
                      + getPlcpHeaderDuration(payloadMode, preamble)
                      + getPayloadDuration(size, payloadMode);
  return duration;
}

ModulationType
WifyModulationType::getPlcpHeaderMode(ModulationType payloadMode, WifiPreamble preamble)
{
  switch (payloadMode.getModulationClass())
     {
     case MOD_CLASS_OFDM:
       {
         switch (payloadMode.getBandwidth()) {
         case 5000000:
           return WifyModulationType::GetOfdmRate1_5MbpsBW5MHz();
         case 10000000:
           return WifyModulationType::GetOfdmRate3MbpsBW10MHz();
         default:
           // IEEE Std 802.11-2007, 17.3.2
           // actually this is only the first part of the PlcpHeader,
           // because the last 16 bits of the PlcpHeader are using the
           // same mode of the payload
           return WifyModulationType::GetOfdmRate6Mbps();
         }
       }

     case MOD_CLASS_ERP_OFDM:
       return WifyModulationType::GetErpOfdmRate6Mbps();

     case MOD_CLASS_DSSS:
       if (preamble == WIFI_PREAMBLE_LONG)
         {
           // IEEE Std 802.11-2007, sections 15.2.3 and 18.2.2.1
           return WifyModulationType::GetDsssRate1Mbps();
         }
       else //  WIFI_PREAMBLE_SHORT
         {
           // IEEE Std 802.11-2007, section 18.2.2.2
           return WifyModulationType::GetDsssRate2Mbps();
         }

     default:
       opp_error("unsupported modulation class");
       return ModulationType();
     }
}

