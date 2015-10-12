//
// Copyright (C) 2015 Andras Varga
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
// Author: Andras Varga
//

#ifndef __INET_BASICIMMEDIATETX_H
#define __INET_BASICIMMEDIATETX_H

#include "IImmediateTx.h"

namespace inet {
namespace ieee80211 {

class IUpperMac;
class IMacRadioInterface;

class INET_API BasicImmediateTx : public cSimpleModule, public IImmediateTx
{
    protected:
        IMacRadioInterface *mac;
        IUpperMac *upperMac;
        Ieee80211Frame *frame = nullptr;
        cMessage *endIfsTimer = nullptr;
        bool transmitting = false;
        ITxCallback *completionCallback = nullptr;

    protected:
        virtual void initialize() override;
        virtual void handleMessage(cMessage *msg) override;
        virtual void updateDisplayString();

    public:
        BasicImmediateTx() {}
        ~BasicImmediateTx();

        virtual void transmitImmediateFrame(Ieee80211Frame *frame, simtime_t ifs, ITxCallback *completionCallback) override;
        virtual void radioTransmissionFinished() override;
};

} // namespace ieee80211
} // namespace inet

#endif

