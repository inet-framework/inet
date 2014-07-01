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

#ifndef __INET_IEEE80211SCALARTRANSMITTER_H
#define __INET_IEEE80211SCALARTRANSMITTER_H

#include "ScalarTransmitter.h"
#include "WifiPreambleType.h"

namespace inet {

namespace physicallayer
{

class INET_API Ieee80211ScalarTransmitter : public ScalarTransmitter
{
    protected:
        char opMode;
        WifiPreamble preambleMode;

    protected:
        virtual void initialize(int stage);

    public:
        Ieee80211ScalarTransmitter() :
            ScalarTransmitter(),
            opMode('\0'),
            preambleMode((WifiPreamble)-1)
        {}

        virtual const ITransmission *createTransmission(const IRadio *radio, const cPacket *packet, simtime_t startTime) const;
};

}

}


#endif
