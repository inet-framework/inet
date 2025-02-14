//
// Copyright (C) 2015-2019 Felix weinrank
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

#include "TrafficgenSimple.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Message.h"

namespace inet {
Define_Module(TrafficgenSimple);
simsignal_t TrafficgenSimple::sentPktSignal = registerSignal("sentPkt");

TrafficgenSimple::TrafficgenSimple() {
    active = false;
    timerStartSending = new cMessage("timer start",TRAFFICGEN_TIMER_START_SENDING);
    timerStopSending = new cMessage("timer stop",TRAFFICGEN_TIMER_STOP_SENDING);
    timerSendPacket = new cMessage("timer send",TRAFFICGEN_TIMER_SEND_PACKET);
}

TrafficgenSimple::~TrafficgenSimple() {
    cancelAndDelete(timerStartSending);
    cancelAndDelete(timerStopSending);
    cancelAndDelete(timerSendPacket);
}

void TrafficgenSimple::initialize(int stage) {
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        id              = par("id");
        name            = par("name").str();
        priority        = par("priority");
        packetCount     = par("packetCount");
        startTime       = par("startTime");
        stopTime        = par("stopTime");
        reliable        = par("reliable");
        ordered         = par("ordered");
        sentPktCount    = 0;

    }

    // inform connected application about parameters
    // important for SCTP: packet count
    if (stage == INITSTAGE_LINK_LAYER) {
        sendInit();
    }

    if (stage == INITSTAGE_APPLICATION_LAYER) {
        // start sending timer
        scheduleAt(simTime() + par("startTime").doubleValue(), timerStartSending);
        setStatusString("waiting");

        // start timer to stop sending if stopTime is defined
        if (par("stopTime").doubleValue() > 0.0 && par("stopTime").doubleValue() > par("startTime").doubleValue()) {
            scheduleAt(simTime() + par("stopTime").doubleValue(), timerStopSending);
        }
    }

};

/*
 * Handle Messages
 */
void TrafficgenSimple::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        EVD << msg->getKind() << endl;

        switch (msg->getKind()) {
            case TRAFFICGEN_TIMER_START_SENDING:
                EVD << "start sending timer";
                active = true;
            case TRAFFICGEN_TIMER_SEND_PACKET:
                sendData();
                break;
            case TRAFFICGEN_TIMER_STOP_SENDING:
                EVD << "stop sending timer";
                setStatusString("finished");
                sendFinish();
                cancelEvent(timerSendPacket);
                break;
            default:
                throw cRuntimeError("Invalid kind %d in self message", (int) msg->getKind());
        }
    } else if(msg->arrivedOn("generatorIn")) {
        handleTrafficControlMessage(check_and_cast<TrafficgenControl*>(msg));
        delete msg;
    } else {
        throw cRuntimeError("Don't know how to handle this message: %d", (int) msg->getKind());
    }
};

/*
 * Send initial message to inform app
 */
void TrafficgenSimple::sendInit() {
    char packetname [256];
    snprintf(packetname, 256, "%s - %s", name.c_str(), "init");
    TrafficgenInfo* info = new TrafficgenInfo(packetname);
    info->setKind(TRAFFICGEN_MSG_INFO);
    info->setType(TRAFFICGEN_INFO_INIT);
    info->setId(id);
    info->setPriority(priority);
    send(info, "generatorOut");
}

/*
 * Send data to app
 */
void TrafficgenSimple::sendData() {
    // prepare data chunk
    char packetname [256];
    snprintf(packetname, 256, "%s - %s", name.c_str(), "data");
    int burstSize = par("packetBurstSize");

    for (int i = 0; i < burstSize; i++) {
        TrafficgenData *data = new TrafficgenData(packetname);
        data->setKind(TRAFFICGEN_MSG_DATA);
        data->setId(id);
        data->setByteLength(par("packetSize").intValue());
        data->setPriority(priority);
        data->setReliable(reliable);
        data->setOrdered(ordered);
        // send packet, increment stats counter, start new timer
        send(data, "generatorOut");
        sentPktCount++;
        emit(sentPktSignal,data);

        EVD << "Generator: " << name << " - sending data - size: " << data->getByteLength() << endl;
    }

    if (sentPktCount < packetCount || packetCount == -1) {
        setStatusString("sending");
        scheduleAt(simTime() + par("packetInterval").doubleValue(), timerSendPacket);
    } else {
        sendFinish();
        setStatusString("finished");
        EVD << "all messages sent!" << endl;
    }
}

/*
 * Send finish message
 */

void TrafficgenSimple::sendFinish() {
    if (!active) {
        return;
    }
    char packetname [256];
    snprintf(packetname, 256, "%s - %s", name.c_str(), "finish");
    TrafficgenInfo* info = new TrafficgenInfo(packetname);
    info->setKind(TRAFFICGEN_MSG_INFO);
    info->setType(TRAFFICGEN_INFO_FINISH);
    info->setId(id);
    info->setPriority(priority);
    send(info, "generatorOut");
    active = false;
}

/*
 * Handle indication from app
 */
void TrafficgenSimple::handleTrafficControlMessage(TrafficgenControl *ind) {
    EVD << "indication received" << endl;

    if(ind->getControlMessageType() == TRAFFICGEN_START_SENDING &&
            //par("packetInterval").doubleValue() == 0.0 &&
            timerSendPacket->isScheduled() == false &&
            active == true) {
        sendData();
        return;
    }

    if(ind->getControlMessageType() == TRAFFICGEN_STOP_SENDING  &&
            //par("packetInterval").doubleValue() == 0.0 &&
            active == true) {
        cancelEvent(timerSendPacket);
        return;
    }
}

void TrafficgenSimple::finish() {
    EV << "### STATS ###" << endl;
    EV << sentPktCount << " pkt sent" << endl;
    EV << "###" << endl;
};

void TrafficgenSimple::setStatusString(const char *s) {
    if (hasGUI()) {
        getDisplayString().setTagArg("t", 0, s);
    }
}

} // namespace inet
