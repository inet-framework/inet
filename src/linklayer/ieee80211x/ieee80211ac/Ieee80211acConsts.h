//
// Copyright (C) 2014 Andrea Tino
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

#ifndef IEEE80211AC_CONSTS_H
#define IEEE80211AC_CONSTS_H

class Ieee80211acConsts {
public:
  // Types
  typedef unsigned int PktLength; // Number of bits

  /**
   *
   */
  static PktLength getLengthRTS() { return 160; }

  /**
   *
   */
  static PktLength getLengthCTS() { return 112; }

  /**
   *
   */
  static PktLength getLengthACK() { return 112; }

  /**
   *
   */
  static PktLength getLengthMGMT() { return 28*8; }

  /**
   *
   */
  static PktLength getLengthDATAHDR() { return 34*8; }

  /**
   *
   */
  static PktLength getHeaderBytesSNAP() { return 8; }

  /**
   * Time slot ST, short interframe space SIFS, distributed interframe
   * space DIFS, and extended interframe space EIFS.
   */
  simtime_t getST() { return 20E-6; }

  /**
   *
   */
  simtime_t getSIFS() { return 10E-6; }

  /**
   *
   */
  simtime_t getDIFS() { return 2*getST()+getSIFS(); }

  /**
   * 300 meters at the speed of light.
   */
  simtime_t getMaxPropagationDelay() { return 2E-6; }

  /**
   *
   */
  int getRetryLimit() { return 7; }

  /** 
   * Minimum size (initial size) of contention window.
   */
  int getMinimumContentionWindow() { return 31; }

  /** 
   * Maximum size of contention window.
   */
  int getMaximumContentionWindow() { return 1023; }

  /**
   *
   */
  int getHeaderNoPreamble() { return 48; }

  /**
   *
   */
  double getBitrateHeader() { return 1E+6; }

  /**
   *
   */
  double getBandwidth() { return 2E+6; }

}; /* Ieee80211acConsts */

#endif

