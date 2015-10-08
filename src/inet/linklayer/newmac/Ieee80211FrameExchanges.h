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

#ifndef IEEE80211FRAMEEXCHANGES_H_
#define IEEE80211FRAMEEXCHANGES_H_

#include "Ieee80211MacPlugin.h"
#include "Ieee80211FrameExchange.h"

namespace inet {
namespace ieee80211 {

class Ieee80211NewMac;
class IIeee80211MacContext;
class Ieee80211DataOrMgmtFrame;

class Ieee80211SendDataWithAckFrameExchange : public Ieee80211FSMBasedFrameExchange
{
    protected:
        Ieee80211DataOrMgmtFrame *frame;
        int retryCount = 0;
        cMessage *ackTimer = nullptr;

        enum State { INIT, TRANSMITDATA, WAITACK, SUCCESS, FAILURE };
        State state = INIT;

    protected:
        void handleWithFSM(EventType event, cMessage *frameOrTimer);

        void transmitDataFrame();
        void retryDataFrame();
        void scheduleAckTimeout();
        void processFrame(Ieee80211Frame *receivedFrame);
        bool isAck(Ieee80211Frame *frame);

    public:
        Ieee80211SendDataWithAckFrameExchange(Ieee80211NewMac *mac, IIeee80211MacContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *frame);
        ~Ieee80211SendDataWithAckFrameExchange();
};

class Ieee80211SendDataWithRtsCtsFrameExchange : public Ieee80211StepBasedFrameExchange
{
    protected:
        Ieee80211DataOrMgmtFrame *dataFrame = nullptr;
        int retryCount = 0;
    protected:
        virtual void doStep(int step);
        virtual bool processReply(int step, Ieee80211Frame *frame);
        virtual void processTimeout(int step);
    public:
        Ieee80211SendDataWithRtsCtsFrameExchange(Ieee80211NewMac *mac, IIeee80211MacContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *dataFrame);
};

}
} /* namespace inet */

#endif

