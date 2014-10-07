//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_IEEE80211RADIO_H
#define __INET_IEEE80211RADIO_H

#include "inet/physicallayer/common/Radio.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211Radio : public Radio
{
  public:
    /**
     * This signal is emitted every time the radio channel changes.
     * The signal value is the new radio channel.
     */
    static simsignal_t radioChannelChangedSignal;

  protected:
    int channelNumber;

  protected:
    void initialize(int stage);

    void handleUpperCommand(cMessage *message);

  public:
    Ieee80211Radio();

    virtual void setBitrate(bps newBitrate);
    virtual void setChannelNumber(int newChannelNumber);
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IEEE80211RADIO_H

