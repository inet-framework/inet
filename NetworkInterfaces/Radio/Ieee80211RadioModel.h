//
// Copyright (C) Andras Varga, Levente Meszaros
// Based on the Mobility Framework's SnrEval by Marc Loebbers
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

#ifndef IEEE80211RADIOMODEL_H
#define IEEE80211RADIOMODEL_H

#include "IRadioModel.h"

/**
 * Radio model for IEEE 802.11.
 */
class INET_API Ieee80211RadioModel : public IRadioModel
{
  protected:
    double snirThreshold;

  protected:
    virtual void initializeFrom(cModule *radioModule);

    virtual double calcDuration(AirFrame *airframe);

    virtual bool isReceivedCorrectly(AirFrame *airframe, const SnrList& receivedList);

    // utility
    virtual bool packetOk(double snirMin, int lengthMPDU, double bitrate);
    // utility
    virtual double dB2fraction(double dB);
};

#endif

