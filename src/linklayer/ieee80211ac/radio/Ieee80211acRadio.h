//
// Copyright (C) 2014 Andrea Tino
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

//
// Ieee80211acRadio.h
//

#ifndef IEEE80211ACRADIO_H_
#define IEEE80211ACRADIO_H_

#include "INETDefs.h"
#include "AirFrame_m.h"
#include "radio/Radio.h"

/**
 * Class for radio layer of IEEE 802.11ac protocol.
 *
 * @author Andrea Tino
 */
class INET_API Ieee80211acRadio : public Radio {
public:
  typedef double Decibel;
  typedef double Frequency;
  typedef double Ber;
  typedef double Energy;
  typedef double Power;
  typedef double SNR;

  /**
   * @brief Transmitter status.
   */
  enum TxStatus {
  };

  /**
   * @brief Receiver status.
   */
  enum RxStatus {
  };

protected:
  typedef Radio::SensitivityList SensitivityList;

public:
  /**
   * @brief Creates a new instance.
   */
  Ieee80211acRadio();

  virtual ~Ieee80211acRadio();

protected:
  /**
   * @brief OMNeT++ special function.
   */
  virtual void initialize();

  /**
   * @brief OMNeT++ special function.
   */
  virtual void finish();

  /**
   * @brief OMNeT++ special function.
   */
  virtual void handleMessage(cMessage* msg);

protected:
  virtual void handleupperMsg(AirFrame* airframe);

  virtual void handleSelfMsg(cMessage* msg);

  virtual void handleCommand(int msgkind, cObject* ctrl);

  virtual void handleLowerMsgStart(AirFrame* airframe);

  virtual void handleLowerMsgEnd(AirFrame* airframe);

  virtual void bufferMsg(AirFrame* airframe);

  virtual AirFrame* unbufferMsg(cMessage* msg);

  virtual void sendUp(AirFrame* airframe);

  virtual void sendDown(AirFrame* airframe);

  virtual AirFrame* encapsulatePacket(cPacket* pkt);

  virtual bool processAirFrame(AirFrame* airframe);

}; /* Ieee80211acRadio */

#endif

