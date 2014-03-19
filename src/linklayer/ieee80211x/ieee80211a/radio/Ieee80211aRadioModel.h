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

//
// Ieee80211acRadioModel.h
//

#ifndef IEEE80211ACRADIO_H_
#define IEEE80211ACRADIO_H_

#include "INETDefs.h"
#include "AirFrame_m.h"

/**
 * Class for radio layer of IEEE 802.11ac protocol.
 *
 * @author Andrea Tino
 */
class INET_API Ieee80211acRadioModel : public IRadioModel
{
  public:
    /**
     * @brief Creates a new instance.
     */
    Ieee80211acRadioModel();

    /**
     * @brief Creates a new instance.
     */
    virtual ~Ieee80211acRadioModel();

    /**
     * @brief Creates a new instance.
     */
    virtual void initializeFrom(cModule *radioModule);

    /**
     * @brief Creates a new instance.
     */
    virtual double calculateDuration(AirFrame *);

    /**
     * @brief Creates a new instance.
     */
    virtual bool isReceivedCorrectly(AirFrame *airframe,
      const SnrList& receivedList);

    /**
     * @brief Creates a new instance.
     */
    virtual bool haveTestFrame();

    /**
     * @brief Creates a new instance.
     */
    virtual double calculateDurationTestFrame(AirFrame *airframe);

    /**
     * @brief Creates a new instance.
     */
    virtual double getTestFrameError(double snirMin, double bitrate);

    /**
     * @brief Creates a new instance.
     */
    virtual int getTestFrameSize();

}; /* Ieee80211acRadioModel */

#endif

