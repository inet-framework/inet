/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006,2007 INRIA
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
#ifndef __MODULATION_TYPE_H
#define __MODULATION_TYPE_H

//#include <stdint.h>
#include <string>
#include <vector>
#include <ostream>


enum ModulationClass {
  /** Modulation class unknown or unspecified. A WifiMode with this
  WifiModulationClass has not been properly initialised. */
  MOD_CLASS_UNKNOWN = 0,
  /** Infrared (IR) (Clause 16) */
  MOD_CLASS_IR,
  /** Frequency-hopping spread spectrum (FHSS) PHY (Clause 14) */
  MOD_CLASS_FHSS,
  /** DSSS PHY (Clause 15) and HR/DSSS PHY (Clause 18) */
  MOD_CLASS_DSSS,
  /** ERP-PBCC PHY (19.6) */
  MOD_CLASS_ERP_PBCC,
  /** DSSS-OFDM PHY (19.7) */
  MOD_CLASS_DSSS_OFDM,
  /** ERP-OFDM PHY (19.5) */
  MOD_CLASS_ERP_OFDM,
  /** OFDM PHY (Clause 17) */
  MOD_CLASS_OFDM,
  /** HT PHY (Clause 20) */
  MOD_CLASS_HT
};


/**
 * This enumeration defines the various convolutional coding rates
 * used for the OFDM transmission modes in the IEEE 802.11
 * standard. DSSS (for example) rates which do not have an explicit
 * coding stage in their generation should have this parameter set to
 * WIFI_CODE_RATE_UNDEFINED.
 */
enum CodeRate {
  /** No explicit coding (e.g., DSSS rates) */
  CODE_RATE_UNDEFINED,
  /** Rate 3/4 */
  CODE_RATE_3_4,
  /** Rate 2/3 */
  CODE_RATE_2_3,
  /** Rate 1/2 */
  CODE_RATE_1_2
};

/**
 * \brief represent a single transmission mode
 *
 * A WifiMode is implemented by a single integer which is used
 * to lookup in a global array the characteristics of the
 * associated transmission mode. It is thus extremely cheap to
 * keep a WifiMode variable around.
 */
class ModulationType
{

 public:
  /**
   * \returns the number of Hz used by this signal
   */
  uint32_t getBandwidth(void) const {return bandwidth;}
  void setBandwidth(uint32_t p) {bandwidth = p;}
  /**
   * \returns the physical bit rate of this signal.
   *
   * If a transmission mode uses 1/2 FEC, and if its
   * data rate is 3Mbs, the phy rate is 6Mbs
   */
  /// MANDATORY it is necessary set the dataRate before the codeRate
   void setCodeRate(enum CodeRate cRate)
   {
    codeRate = cRate;
    switch (cRate)
    {
      case CODE_RATE_3_4:
      phyRate = dataRate * 4 / 3;
      break;
    case CODE_RATE_2_3:
      phyRate = dataRate * 3 / 2;
      break;
    case CODE_RATE_1_2:
      phyRate = dataRate * 2 / 1;
      break;
    case CODE_RATE_UNDEFINED:
    default:
      phyRate = dataRate;
      break;
    }
   };
   uint32_t getPhyRate(void) const {return phyRate;}
  /**
   * \returns the data bit rate of this signal.
   */
  uint32_t getDataRate(void) const {return dataRate;}
  void setDataRate(uint32_t p) {dataRate = p;}
  /**
   * \returns the coding rate of this transmission mode
   */
  enum CodeRate getCodeRate(void) const {return codeRate;}

  /**
   * \returns the size of the modulation constellation.
   */
  uint8_t getConstellationSize(void) const {return constellationSize;}
  void setConstellationSize(uint8_t p) {constellationSize = p;}

  /**
   * \returns true if this mode is a mandatory mode, false
   *          otherwise.
   */
  enum ModulationClass getModulationClass() const {return modulationClass;}
  void setModulationClass(enum ModulationClass p) {modulationClass = p;}

  void setIsMandatory(bool val){isMandatory = val;}
  bool getIsMandatory(){return isMandatory;}
  ModulationType()
  {
      isMandatory = false;
      bandwidth = 0;
      codeRate = CODE_RATE_UNDEFINED;
      dataRate = 0;
      phyRate = 0;
      constellationSize = 0;
      modulationClass = MOD_CLASS_UNKNOWN;
  }
private:
  bool isMandatory;
  uint32_t bandwidth;
  enum CodeRate codeRate;
  uint32_t dataRate;
  uint32_t phyRate;
  uint8_t constellationSize;
  enum ModulationClass modulationClass;
};

bool operator==(const ModulationType &a, const ModulationType &b);


#endif
