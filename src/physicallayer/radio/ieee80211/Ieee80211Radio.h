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

#ifndef __INET_IEEE80211RADIO_H_
#define __INET_IEEE80211RADIO_H_

#include "ScalarRadio.h"

namespace radio
{

class INET_API Ieee80211Radio : public ScalarRadio
{
    protected:
        int channelNumber;

    protected:
        void initialize(int stage);

        void handleUpperCommand(cMessage *message);

    public:
        Ieee80211Radio();
        Ieee80211Radio(RadioMode radioMode, const IAntenna *antenna, const ITransmitter *transmitter, const IReceiver *receiver, IRadioMedium *channel);

        virtual void setChannelNumber(int newChannelNumber);
};

}

#endif
