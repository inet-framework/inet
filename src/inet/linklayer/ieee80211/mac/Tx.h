//
// Copyright (C) 2016 OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_TX_H
#define __INET_TX_H

#include "inet/linklayer/ieee80211/mac/contract/ITx.h"

namespace inet {
namespace ieee80211 {

class Ieee80211Mac;
class IRx;
class IStatistics;

/**
 * The default implementation of ITx.
 */
class INET_API Tx : public cSimpleModule, public ITx
{
    protected:
        MACAddress address = MACAddress::UNSPECIFIED_ADDRESS;
        ITx::ICallback *txCallback = nullptr;
        Ieee80211Mac *mac = nullptr;
        IRx *rx = nullptr;
        IStatistics *statistics = nullptr;
        Ieee80211Frame *frame = nullptr;
        cMessage *endIfsTimer = nullptr;
        simtime_t durationField = -1;
        bool transmitting = false;

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;
        virtual void handleMessage(cMessage *msg) override;
        virtual void refreshDisplay() const override;

    public:
        Tx() {}
        ~Tx();

        virtual void transmitFrame(Ieee80211Frame *frame, ITx::ICallback *txCallback) override;
        virtual void transmitFrame(Ieee80211Frame *frame, simtime_t ifs, ITx::ICallback *txCallback) override;
        virtual void radioTransmissionFinished() override;
};

} // namespace ieee80211
} // namespace inet

#endif
