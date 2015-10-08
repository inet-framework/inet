//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __MAC_IEEE80211MACIMMEDIATETX_H_
#define __MAC_IEEE80211MACIMMEDIATETX_H_

#include "Ieee80211MacPlugin.h"
#include "IIeee80211MacImmediateTx.h"

namespace inet {

namespace ieee80211 {

class Ieee80211MacImmediateTx : public Ieee80211MacPlugin, public IIeee80211MacImmediateTx
{
    protected:
        Ieee80211Frame *frame = nullptr;
        cMessage *endIfsTimer = nullptr;
        bool transmitting = false;
        ICallback *completionCallback = nullptr;

    protected:
        virtual void handleMessage(cMessage *msg);

    public:
        Ieee80211MacImmediateTx(Ieee80211NewMac *mac);
        ~Ieee80211MacImmediateTx();

        virtual void transmitImmediateFrame(Ieee80211Frame *frame, simtime_t ifs, ICallback *completionCallback) override;
        virtual void radioTransmissionFinished() override;
};

}

} //namespace

#endif
