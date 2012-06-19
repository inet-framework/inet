//
// Copyright (C) 2006 Andras Varga, Levente Meszaros
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
#include "BerParseFile.h"
#include "IErrorModel.h"
#include "WifiPreambleType.h"

/**
 * Radio model for IEEE 802.11. The implementation is largely based on the
 * Mobility Framework's SnrEval80211 and Decider80211 modules.
 * See the NED file for more info.
 */




class INET_API Ieee80211RadioModel : public IRadioModel
{
  protected:
    double snirThreshold;
    cOutVector snirVector;
    int i;
    BerParseFile *parseTable;
    bool fileBer;

    char phyOpMode;

    double PHY_HEADER_LENGTH;
    IErrorModel * errorModel;
    WifiPreamble wifiPreamble;
    bool  autoHeaderSize;

    unsigned int btSize; //
    bool useTestFrame;

  public:
    virtual void initializeFrom(cModule *radioModule);

    virtual double calculateDuration(AirFrame *airframe);

    virtual bool isReceivedCorrectly(AirFrame *airframe, const SnrList& receivedList);
    ~Ieee80211RadioModel();

    // used by the Airtime Link Metric computation
    virtual bool haveTestFrame() {return useTestFrame;}
    virtual double calculateDurationTestFrame(AirFrame *airframe);
    virtual double getTestFrameError(double snirMin, double bitrate);
    virtual int getTestFrameSize()
                {
                     if (useTestFrame) return btSize;
                     else return -1;
                }
  protected:
    // utility
    virtual bool isPacketOK(double snirMin, int lengthMPDU, double bitrate);
    // utility
    virtual double dB2fraction(double dB);

};

#endif

