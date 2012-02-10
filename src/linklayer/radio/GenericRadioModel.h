//
// Copyright (C) 2006 Andras Varga
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef GENERICRADIOMODEL_H
#define GENERICRADIOMODEL_H

#include "IRadioModel.h"
#include "IModulation.h"

/**
 * Generic radio model. Frame duration is calculated from the bitrate
 * and the packet length plus a physical header length. Bit error rate
 * is calculated from the modulation scheme, signal bandwidth, bitrate
 * and the frame length.
 */
class INET_API GenericRadioModel : public IRadioModel
{
  protected:
    double snirThreshold;
    long headerLengthBits;
    double bandwidth;
    IModulation *modulation;

  public:
    GenericRadioModel();
    virtual ~GenericRadioModel();

    virtual void initializeFrom(cModule *radioModule);

    virtual double calculateDuration(AirFrame *airframe);

    virtual bool isReceivedCorrectly(AirFrame *airframe, const SnrList& receivedList);

    // used by the Airtime Link Metric computation
    virtual bool haveTestFrame() {return false;}
    virtual double calculateDurationTestFrame(AirFrame *airframe) {return 0;}
    virtual double getTestFrameError(double snirMin, double bitrate) {return 0;}
    virtual int getTestFrameSize() {return 0;}
  protected:
    // utility
    virtual bool isPacketOK(double snirMin, int length, double bitrate);
    // utility
    virtual double dB2fraction(double dB);
};

#endif

