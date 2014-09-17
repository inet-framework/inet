//
// Copyright (C) 2011 Juan Luis Garrote Molinero
// Copyright (C) 2013 Zsolt Prontvai
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

#include "inet/linklayer/ieee8021d/rstp/RSTP.h"

#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"

namespace inet {

Define_Module(RSTP);

RSTP::RSTP()
{
    helloTimer = NULL;
    upgradeTimer = NULL;
}

RSTP::~RSTP()
{
    cancelAndDelete(helloTimer);
    cancelAndDelete(upgradeTimer);
}

void RSTP::initialize(int stage)
{
    STPBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        autoEdge = par("autoEdge");
        tcWhileTime = par("tcWhileTime");
        migrateTime = par("migrateTime");
        helloTimer = new cMessage("itshellotime", SELF_HELLOTIME);
        upgradeTimer = new cMessage("upgrade", SELF_UPGRADE);
    }
    else if (stage == INITSTAGE_LINK_LAYER_2) {    // "auto" MAC addresses assignment takes place in stage 0
        updateDisplay();
    }
}

void RSTP::scheduleNextUpgrade()
{
    cancelEvent(upgradeTimer);
    Ieee8021dInterfaceData *nextInterfaceData = NULL;
    for (unsigned int i = 0; i < numPorts; i++) {
        if (getPortInterfaceEntry(i)->hasCarrier()) {
            Ieee8021dInterfaceData *iPort = getPortInterfaceData(i);
            if (iPort->getRole() == Ieee8021dInterfaceData::NOTASSIGNED) {
                if (nextInterfaceData == NULL)
                    nextInterfaceData = iPort;
                else if (iPort->getNextUpgrade() < nextInterfaceData->getNextUpgrade())
                    nextInterfaceData = iPort;
            }
            else if (iPort->getRole() == Ieee8021dInterfaceData::DESIGNATED) {
                if (iPort->getState() == Ieee8021dInterfaceData::DISCARDING) {
                    if (nextInterfaceData == NULL)
                        nextInterfaceData = iPort;
                    else if (iPort->getNextUpgrade() < nextInterfaceData->getNextUpgrade())
                        nextInterfaceData = iPort;
                }
                else if (iPort->getState() == Ieee8021dInterfaceData::LEARNING) {
                    if (nextInterfaceData == NULL)
                        nextInterfaceData = iPort;
                    else if (iPort->getNextUpgrade() < nextInterfaceData->getNextUpgrade())
                        nextInterfaceData = iPort;
                }
            }
        }
    }
    if (nextInterfaceData != NULL)
        scheduleAt(nextInterfaceData->getNextUpgrade(), upgradeTimer);
}

void RSTP::handleMessage(cMessage *msg)
{
    // it can receive BPDU or self messages
    if (!isOperational) {
        if (msg->isSelfMessage())
            throw cRuntimeError("Model error: self msg '%s' received when isOperational is false", msg->getName());
        EV << "Message '" << msg << "' arrived when module status is down, dropped\n";
        delete msg;
        return;
    }

    if (msg->isSelfMessage()) {
        switch (msg->getKind()) {
            case SELF_HELLOTIME:
                handleHelloTime(msg);
                break;

            case SELF_UPGRADE:
                // designated ports state upgrading (discarding-->learning, learning-->forwarding)
                handleUpgrade(msg);
                break;

            default:
                error("Unknown self message");
                break;
        }
    }
    else {
        handleIncomingFrame(check_and_cast<BPDU *>(msg));    // handling BPDU
    }
}

void RSTP::handleUpgrade(cMessage *msg)
{
    for (unsigned int i = 0; i < numPorts; i++) {
        Ieee8021dInterfaceData *iPort = getPortInterfaceData(i);
        if (getPortInterfaceEntry(i)->hasCarrier() && iPort->getNextUpgrade() == simTime()) {
            if (iPort->getRole() == Ieee8021dInterfaceData::NOTASSIGNED) {
                EV_DETAIL << "MigrateTime. Setting port " << i << "to designated." << endl;
                iPort->setRole(Ieee8021dInterfaceData::DESIGNATED);
                iPort->setState(Ieee8021dInterfaceData::DISCARDING);    // contest to become forwarding.
                iPort->setNextUpgrade(simTime() + forwardDelay);
            }
            else if (iPort->getRole() == Ieee8021dInterfaceData::DESIGNATED) {
                if (iPort->getState() == Ieee8021dInterfaceData::DISCARDING) {
                    EV_INFO << "UpgradeTime. Setting port " << i << " state to learning." << endl;
                    iPort->setState(Ieee8021dInterfaceData::LEARNING);
                    iPort->setNextUpgrade(simTime() + forwardDelay);
                }
                else if (iPort->getState() == Ieee8021dInterfaceData::LEARNING) {
                    EV_INFO << "UpgradeTime. Setting port " << i << " state to forwarding." << endl;
                    iPort->setState(Ieee8021dInterfaceData::FORWARDING);
                    flushOtherPorts(i);
                }
            }
        }
    }
    updateDisplay();
    scheduleNextUpgrade();
}

void RSTP::handleHelloTime(cMessage *msg)
{
    EV_DETAIL << "Hello time." << endl;
    for (unsigned int i = 0; i < numPorts; i++) {
        // sends hello through all active (learning, forwarding or not assigned) ports
        // increments LostBPDU just from ROOT, ALTERNATE and BACKUP
        Ieee8021dInterfaceData *iPort = getPortInterfaceData(i);
        if (!iPort->isEdge()
            && (iPort->getRole() == Ieee8021dInterfaceData::ROOT
                || iPort->getRole() == Ieee8021dInterfaceData::ALTERNATE
                || iPort->getRole() == Ieee8021dInterfaceData::BACKUP))
        {
            iPort->setLostBPDU(iPort->getLostBPDU() + 1);
            if (iPort->getLostBPDU() > 3) {    // 3 HelloTime without the best BPDU.
                EV_DETAIL << "3 HelloTime without the best BPDU" << endl;
                // starts contest
                if (iPort->getRole() == Ieee8021dInterfaceData::ROOT) {
                    // looking for the best ALTERNATE port
                    int candidate = getBestAlternate();
                    if (candidate != -1) {
                        // if an alternate gate has been found, switch to alternate
                        EV_DETAIL << "It was the root port. Alternate gate has been found. Setting port " << candidate << " to root. Setting current root port (port" << i << ") to designated." << endl;
                        // ALTERNATE->ROOT. DISCARDING->FORWARDING (immediately)
                        // old root gate goes to DESIGNATED and DISCARDING
                        // a new contest should be done to determine the new root path from this LAN
                        // updating root vector.
                        Ieee8021dInterfaceData *candidatePort = getPortInterfaceData(candidate);
                        iPort->setRole(Ieee8021dInterfaceData::DESIGNATED);
                        iPort->setState(Ieee8021dInterfaceData::DISCARDING);    // if there is not a better BPDU, that will become FORWARDING
                        iPort->setNextUpgrade(simTime() + forwardDelay);
                        scheduleNextUpgrade();
                        initInterfacedata(i);    // reset, then a new BPDU will be allowed to upgrade the best received info for this port
                        candidatePort->setRole(Ieee8021dInterfaceData::ROOT);
                        candidatePort->setState(Ieee8021dInterfaceData::FORWARDING);
                        candidatePort->setLostBPDU(0);
                        flushOtherPorts(candidate);
                        macTable->copyTable(i, candidate);    // copy cache from old to new root
                    }
                    else {
                        // alternate not found, selects a new root
                        EV_DETAIL << "It was the root port. Alternate not found. Starts from beginning." << endl;
                        // initializing ports, start from the beginning
                        initPorts();
                    }
                }
                else if (iPort->getRole() == Ieee8021dInterfaceData::ALTERNATE
                         || iPort->getRole() == Ieee8021dInterfaceData::BACKUP)
                {
                    EV_DETAIL << "Setting port " << i << " to designated." << endl;
                    // it should take care of this LAN, switching to designated
                    iPort->setRole(Ieee8021dInterfaceData::DESIGNATED);
                    iPort->setState(Ieee8021dInterfaceData::DISCARDING);    // a new content will start in case of another switch were in alternate
                    iPort->setNextUpgrade(simTime() + forwardDelay);
                    scheduleNextUpgrade();
                    // if there is no problem, this will become forwarding in a few seconds
                    initInterfacedata(i);
                }
                iPort->setLostBPDU(0);    // reseting lost bpdu counter after a change.
            }
        }
    }
    sendBPDUs();    // generating and sending new BPDUs
    sendTCNtoRoot();
    updateDisplay();
    scheduleAt(simTime() + helloTime, msg);    // programming next hello time
}

void RSTP::checkTC(BPDU *frame, int arrival)
{
    Ieee8021dInterfaceData *port = getPortInterfaceData(arrival);
    if ((frame->getTcFlag() == true) && (port->getState() == Ieee8021dInterfaceData::FORWARDING)) {
        EV_DETAIL << "TCN received" << endl;
        findContainingNode(this)->bubble("TCN received");
        for (unsigned int i = 0; i < numPorts; i++) {
            if ((int)i != arrival) {
                Ieee8021dInterfaceData *port2 = getPortInterfaceData(i);
                // flushing other ports
                // TCN over other ports
                macTable->flush(i);
                port2->setTCWhile(simTime() + tcWhileTime);
            }
        }
    }
}

void RSTP::handleBackup(BPDU *frame, unsigned int arrivalPortNum)
{
    EV_DETAIL << "More than one port in the same LAN" << endl;
    Ieee8021dInterfaceData *port = getPortInterfaceData(arrivalPortNum);
    if ((frame->getPortPriority() < port->getPortPriority())
        || ((frame->getPortPriority() == port->getPortPriority()) && (frame->getPortNum() < arrivalPortNum)))
    {
        // flushing arrival port
        macTable->flush(arrivalPortNum);
        port->setRole(Ieee8021dInterfaceData::BACKUP);
        port->setState(Ieee8021dInterfaceData::DISCARDING);
        port->setLostBPDU(0);
        EV_DETAIL << "Setting port " << arrivalPortNum << "to backup" << endl;
    }
    else if (frame->getPortPriority() > port->getPortPriority()
             || (frame->getPortPriority() == port->getPortPriority() && frame->getPortNum() > arrivalPortNum))
    {
        Ieee8021dInterfaceData *port2 = getPortInterfaceData(frame->getPortNum());
        // flushing sender port
        macTable->flush(frame->getPortNum());    // portNum is sender port number, it is not arrival port
        port2->setRole(Ieee8021dInterfaceData::BACKUP);
        port2->setState(Ieee8021dInterfaceData::DISCARDING);
        port2->setLostBPDU(0);
        EV_DETAIL << "Setting port " << frame->getPortNum() << "to backup" << endl;
    }
    else {
        Ieee8021dInterfaceData *port2 = getPortInterfaceData(frame->getPortNum());
        // unavoidable loop, received its own message at the same port
        // switch to disabled
        EV_DETAIL << "Unavoidable loop. Received its own message at the same port. Setting port " << frame->getPortNum() << " to disabled." << endl;
        // flushing that port
        macTable->flush(frame->getPortNum());    // portNum is sender port number, it is not arrival port
        port2->setRole(Ieee8021dInterfaceData::DISABLED);
        port2->setState(Ieee8021dInterfaceData::DISCARDING);
    }
}

void RSTP::handleIncomingFrame(BPDU *frame)
{
    // incoming BPDU handling
    // checking message age
    Ieee802Ctrl *etherctrl = check_and_cast<Ieee802Ctrl *>(frame->removeControlInfo());
    int arrivalPortNum = etherctrl->getSwitchPort();
    MACAddress src = etherctrl->getSrc();
    delete etherctrl;
    EV_INFO << "BPDU received at port " << arrivalPortNum << "." << endl;
    if (frame->getMessageAge() < maxAge) {
        // checking TC
        checkTC(frame, arrivalPortNum);    // sets TCWhile if arrival port was FORWARDING
        // checking possible backup
        if (src.compareTo(bridgeAddress) == 0) // more than one port in the same LAN
            handleBackup(frame, arrivalPortNum);
        else
            processBPDU(frame, arrivalPortNum);
    }
    else
        EV_DETAIL << "Expired BPDU" << endl;
    delete frame;

    updateDisplay();
}

void RSTP::processBPDU(BPDU *frame, unsigned int arrivalPortNum)
{
    //three challenges.
    //
    //first:  vs best received BPDU for that port --------->case
    //second: vs root BPDU--------------------------------->case1
    //third:  vs BPDU that would be sent from this Bridge.->case2
    Ieee8021dInterfaceData *arrivalPort = getPortInterfaceData(arrivalPortNum);
    bool flood = false;
    if (compareInterfacedata(arrivalPortNum, frame, arrivalPort->getLinkCost()) > 0    //better root
        && frame->getRootAddress().compareTo(bridgeAddress) != 0) // root will not participate in a loop with its own address
        flood = processBetterSource(frame, arrivalPortNum);
    else if (frame->getBridgeAddress().compareTo(arrivalPort->getBridgeAddress()) == 0    // worse or similar, but the same source
             && frame->getRootAddress().compareTo(bridgeAddress) != 0) // root will not participate
        flood = processSameSource(frame, arrivalPortNum);
    if (flood) {
        sendBPDUs();    //expedited BPDU
        sendTCNtoRoot();
    }
}

bool RSTP::processBetterSource(BPDU *frame, unsigned int arrivalPortNum)
{
    EV_DETAIL << "Better BDPU received than the current best for this port." << endl;
    // update that port rstp info
    updateInterfacedata(frame, arrivalPortNum);
    Ieee8021dInterfaceData *arrivalPort = getPortInterfaceData(arrivalPortNum);
    int r = getRootIndex();
    if (r == -1) {
        EV_DETAIL << "There was no root. Setting the arrival port to root." << endl;
        // there was no root
        arrivalPort->setRole(Ieee8021dInterfaceData::ROOT);
        arrivalPort->setState(Ieee8021dInterfaceData::FORWARDING);
        arrivalPort->setLostBPDU(0);
        flushOtherPorts(arrivalPortNum);
        return true;
    }
    else {
        Ieee8021dInterfaceData *rootPort = getPortInterfaceData(r);
        // there was a Root -> challenge 2 (compare with the root)
        int case2 = compareInterfacedata(r, frame, arrivalPort->getLinkCost());    // comparing with root port's BPDU
        int case3 = 0;

        switch (case2) {
            case SIMILAR:    // double link to the same port of the root source -> Tie breaking (better local port first)
                EV_DETAIL << "Double link to the same port of the root source." << endl;
                if (rootPort->getPortPriority() < arrivalPort->getPortPriority()
                    || (rootPort->getPortPriority() == arrivalPort->getPortPriority() && (unsigned int)r < arrivalPortNum))
                {
                    // flushing that port
                    EV_DETAIL << "The current root has better local port. Setting the arrival port to alternate." << endl;
                    macTable->flush(arrivalPortNum);
                    arrivalPort->setRole(Ieee8021dInterfaceData::ALTERNATE);
                    arrivalPort->setState(Ieee8021dInterfaceData::DISCARDING);
                    arrivalPort->setLostBPDU(0);
                }
                else {
                    if (arrivalPort->getState() != Ieee8021dInterfaceData::FORWARDING)
                        flushOtherPorts(arrivalPortNum);
                    else
                        macTable->flush(r); // flushing r, needed in case arrival were previously FORWARDING
                    EV_DETAIL << "This has better local port. Setting the arrival port to root. Setting current root port (port " << r << ") to alternate." << endl;
                    rootPort->setRole(Ieee8021dInterfaceData::ALTERNATE);
                    rootPort->setState(Ieee8021dInterfaceData::DISCARDING);    // comes from root, preserve lostBPDU
                    arrivalPort->setRole(Ieee8021dInterfaceData::ROOT);
                    arrivalPort->setState(Ieee8021dInterfaceData::FORWARDING);
                    arrivalPort->setLostBPDU(0);
                    macTable->copyTable(r, arrivalPortNum);    // copy cache from old to new root
                    // the change does not deserve flooding
                }
                break;

            case BETTER_ROOT:    // new port rstp info is better than the root in another gate -> root change
                EV_DETAIL << "Better root received than the current root. Setting the arrival port to root." << endl;
                for (unsigned int i = 0; i < numPorts; i++) {
                    Ieee8021dInterfaceData *iPort = getPortInterfaceData(i);
                    if (!iPort->isEdge()) {    // avoiding clients reseting
                        if (arrivalPort->getState() != Ieee8021dInterfaceData::FORWARDING)
                            iPort->setTCWhile(simTime() + tcWhileTime);
                        macTable->flush(i);
                        if (i != (unsigned)arrivalPortNum) {
                            iPort->setRole(Ieee8021dInterfaceData::NOTASSIGNED);
                            iPort->setState(Ieee8021dInterfaceData::DISCARDING);
                            iPort->setNextUpgrade(simTime() + migrateTime);
                            scheduleNextUpgrade();
                            initInterfacedata(i);
                        }
                    }
                }
                arrivalPort->setRole(Ieee8021dInterfaceData::ROOT);
                arrivalPort->setState(Ieee8021dInterfaceData::FORWARDING);
                arrivalPort->setLostBPDU(0);

                return true;

            case BETTER_RPC:    // same that Root but better RPC
            case BETTER_SRC:    // same that Root RPC but better source
            case BETTER_PORT:    // same that root RPC and source but better port
                if (arrivalPort->getState() != Ieee8021dInterfaceData::FORWARDING) {
                    EV_DETAIL << "Better route to the current root. Setting the arrival port to root." << endl;
                    flushOtherPorts(arrivalPortNum);
                }
                arrivalPort->setRole(Ieee8021dInterfaceData::ROOT);
                arrivalPort->setState(Ieee8021dInterfaceData::FORWARDING);
                arrivalPort->setLostBPDU(0);
                rootPort->setRole(Ieee8021dInterfaceData::ALTERNATE);    // temporary, just one port can be root at contest time
                macTable->copyTable(r, arrivalPortNum);    // copy cache from old to new root
                case3 = contestInterfacedata(r);
                if (case3 >= 0) {
                    EV_DETAIL << "Setting current root port (port " << r << ") to alternate." << endl;
                    rootPort->setRole(Ieee8021dInterfaceData::ALTERNATE);
                    rootPort->setState(Ieee8021dInterfaceData::DISCARDING);
                    // not lostBPDU reset
                    // flushing r
                    macTable->flush(r);
                }
                else {
                    EV_DETAIL << "Setting current root port (port " << r << ") to designated." << endl;
                    rootPort->setRole(Ieee8021dInterfaceData::DESIGNATED);
                    rootPort->setState(Ieee8021dInterfaceData::DISCARDING);
                    rootPort->setNextUpgrade(simTime() + forwardDelay);
                    scheduleNextUpgrade();
                }
                return true;

            case WORSE_ROOT:
                EV_DETAIL << "Worse BDPU received than the current root. Sending BPDU to show him a better root as soon as possible." << endl;
                sendBPDU(arrivalPortNum);    // BPDU to show him a better root as soon as possible
                break;

            case WORSE_RPC:    // same Root but worse RPC
            case WORSE_SRC:    // same Root RPC but worse source
            case WORSE_PORT:    // same Root RPC and source but worse port
                case3 = contestInterfacedata(frame, arrivalPortNum);    // case 0 not possible
                if (case3 < 0) {
                    EV_DETAIL << "Worse route to the current root. Setting the arrival port to designated." << endl;
                    arrivalPort->setRole(Ieee8021dInterfaceData::DESIGNATED);
                    arrivalPort->setState(Ieee8021dInterfaceData::DISCARDING);
                    arrivalPort->setNextUpgrade(simTime() + forwardDelay);
                    scheduleNextUpgrade();
                    sendBPDU(arrivalPortNum);    // BPDU to show him a better root as soon as possible
                }
                else {
                    EV_DETAIL << "Worse route to the current root. Setting the arrival port to alternate." << endl;
                    // flush arrival
                    macTable->flush(arrivalPortNum);
                    arrivalPort->setRole(Ieee8021dInterfaceData::ALTERNATE);
                    arrivalPort->setState(Ieee8021dInterfaceData::DISCARDING);
                    arrivalPort->setLostBPDU(0);
                }
                break;
        }
    }
    return false;
}

bool RSTP::processSameSource(BPDU *frame, unsigned int arrivalPortNum)
{
    EV_DETAIL << "BDPU received from the same source than the current best for this port" << endl;
    Ieee8021dInterfaceData *arrivalPort = getPortInterfaceData(arrivalPortNum);
    int case0 = compareInterfacedata(arrivalPortNum, frame, arrivalPort->getLinkCost());
    // source has updated BPDU information
    switch (case0) {
        case SIMILAR:
            arrivalPort->setLostBPDU(0);    // same BPDU, not updated
            break;

        case WORSE_ROOT:
            EV_DETAIL << "Worse root received than the current best for this port." << endl;
            if (arrivalPort->getRole() == Ieee8021dInterfaceData::ROOT) {
                int alternative = getBestAlternate();    // searching for alternate
                if (alternative >= 0) {
                    EV_DETAIL << "This port was the root, but there is a better alternative. Setting the arrival port to designated and port " << alternative << "to root." << endl;
                    Ieee8021dInterfaceData *alternativePort = getPortInterfaceData(alternative);
                    arrivalPort->setRole(Ieee8021dInterfaceData::DESIGNATED);
                    arrivalPort->setState(Ieee8021dInterfaceData::DISCARDING);
                    arrivalPort->setNextUpgrade(simTime() + forwardDelay);
                    scheduleNextUpgrade();
                    macTable->copyTable(arrivalPortNum, alternative);    // copy cache from old to new root
                    flushOtherPorts(alternative);
                    alternativePort->setRole(Ieee8021dInterfaceData::ROOT);
                    alternativePort->setState(Ieee8021dInterfaceData::FORWARDING);    // comes from alternate, preserves lostBPDU
                    updateInterfacedata(frame, arrivalPortNum);
                    sendBPDU(arrivalPortNum);    // show him a better Root as soon as possible
                }
                else {
                    EV_DETAIL << "This port was the root and there no alternative. Initialize all ports" << endl;
                    int case2 = 0;
                    initPorts();    // allowing other ports to contest again
                    // flushing all ports
                    for (unsigned int j = 0; j < numPorts; j++)
                        macTable->flush(j);
                    case2 = compareInterfacedata(arrivalPortNum, frame, arrivalPort->getLinkCost());
                    if (case2 > 0) {
                        EV_DETAIL << "This switch is not better, keep arrival port as a ROOT" << endl;
                        updateInterfacedata(frame, arrivalPortNum);    // if this module is not better, keep it as a ROOT
                        arrivalPort->setRole(Ieee8021dInterfaceData::ROOT);
                        arrivalPort->setState(Ieee8021dInterfaceData::FORWARDING);
                    }
                    // propagating new information
                    return true;
                }
            }
            else if (arrivalPort->getRole() == Ieee8021dInterfaceData::ALTERNATE) {
                EV_DETAIL << "This port was an alternate, setting to designated" << endl;
                arrivalPort->setRole(Ieee8021dInterfaceData::DESIGNATED);
                arrivalPort->setState(Ieee8021dInterfaceData::DISCARDING);
                arrivalPort->setNextUpgrade(simTime() + forwardDelay);
                scheduleNextUpgrade();
                updateInterfacedata(frame, arrivalPortNum);
                sendBPDU(arrivalPortNum);    //Show him a better Root as soon as possible
            }
            break;

        case WORSE_RPC:
        case WORSE_SRC:
        case WORSE_PORT:
            EV_DETAIL << "Worse route to the current root than the current best for this port." << endl;
            if (arrivalPort->getRole() == Ieee8021dInterfaceData::ROOT) {
                arrivalPort->setLostBPDU(0);
                int alternative = getBestAlternate();    // searching for alternate
                if (alternative >= 0) {
                    Ieee8021dInterfaceData *alternativePort = getPortInterfaceData(alternative);
                    int case2 = 0;
                    case2 = compareInterfacedata(alternative, frame, arrivalPort->getLinkCost());
                    if (case2 < 0) {    // if alternate is better, change
                        alternativePort->setRole(Ieee8021dInterfaceData::ROOT);
                        alternativePort->setState(Ieee8021dInterfaceData::FORWARDING);
                        arrivalPort->setRole(Ieee8021dInterfaceData::DESIGNATED);    // temporary, just one port can be root at contest time
                        int case3 = 0;
                        case3 = contestInterfacedata(frame, arrivalPortNum);
                        if (case3 < 0) {
                            EV_DETAIL << "This port was the root, but there is a better alternative. Setting the arrival port to designated and port " << alternative << "to root." << endl;
                            arrivalPort->setRole(Ieee8021dInterfaceData::DESIGNATED);
                            arrivalPort->setState(Ieee8021dInterfaceData::DISCARDING);
                            arrivalPort->setNextUpgrade(simTime() + forwardDelay);
                            scheduleNextUpgrade();
                        }
                        else {
                            EV_DETAIL << "This port was the root, but there is a better alternative. Setting the arrival port to alternate and port " << alternative << "to root." << endl;
                            arrivalPort->setRole(Ieee8021dInterfaceData::ALTERNATE);
                            arrivalPort->setState(Ieee8021dInterfaceData::DISCARDING);
                        }
                        flushOtherPorts(alternative);
                        macTable->copyTable(arrivalPortNum, alternative);    // copy cache from old to new root
                    }
                }
                updateInterfacedata(frame, arrivalPortNum);
                // propagating new information
                return true;
                // if alternate is worse than root, or there is not alternate, keep old root as root
            }
            else if (arrivalPort->getRole() == Ieee8021dInterfaceData::ALTERNATE) {
                int case2 = 0;
                case2 = contestInterfacedata(frame, arrivalPortNum);
                if (case2 < 0) {
                    EV_DETAIL << "This port was an alternate, setting to designated" << endl;
                    arrivalPort->setRole(Ieee8021dInterfaceData::DESIGNATED);    // if the frame is worse than this module generated frame, switch to Designated/Discarding
                    arrivalPort->setState(Ieee8021dInterfaceData::DISCARDING);
                    arrivalPort->setNextUpgrade(simTime() + forwardDelay);
                    scheduleNextUpgrade();
                    sendBPDU(arrivalPortNum);    // show him a better BPDU as soon as possible
                }
                else {
                    arrivalPort->setLostBPDU(0);    // if it is better than this module generated frame, keep it as alternate
                    // this does not deserve expedited BPDU
                }
            }
            updateInterfacedata(frame, arrivalPortNum);
            break;
    }
    return false;
}

void RSTP::sendTCNtoRoot()
{
    // if TCWhile is not expired, sends BPDU with TC flag to the root
    this->bubble("SendTCNtoRoot");
    EV_DETAIL << "SendTCNtoRoot" << endl;
    int r = getRootIndex();
    if ((r >= 0) && ((unsigned int)r < numPorts)) {
        Ieee8021dInterfaceData *rootPort = getPortInterfaceData(r);
        if (rootPort->getRole() != Ieee8021dInterfaceData::DISABLED) {
            if (simTime() < rootPort->getTCWhile()) {
                BPDU *frame = new BPDU();
                Ieee802Ctrl *etherctrl = new Ieee802Ctrl();

                frame->setRootPriority(rootPort->getRootPriority());
                frame->setRootAddress(rootPort->getRootAddress());
                frame->setMessageAge(rootPort->getAge());
                frame->setRootPathCost(rootPort->getRootPathCost());
                frame->setBridgePriority(bridgePriority);
                frame->setTcaFlag(false);
                frame->setPortNum(r);
                frame->setBridgeAddress(bridgeAddress);
                frame->setTcFlag(true);
                frame->setName("BPDU");
                frame->setMaxAge(maxAge);
                frame->setHelloTime(helloTime);
                frame->setForwardDelay(forwardDelay);
                if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
                    frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);
                etherctrl->setSrc(bridgeAddress);
                etherctrl->setDest(MACAddress::STP_MULTICAST_ADDRESS);
                etherctrl->setSwitchPort(r);
                frame->setControlInfo(etherctrl);
                send(frame, "relayOut");
            }
        }
    }
}

void RSTP::sendBPDUs()
{
    // send BPDUs through all ports, if they are required
    for (unsigned int i = 0; i < numPorts; i++) {
        Ieee8021dInterfaceData *iPort = getPortInterfaceData(i);
        if ((iPort->getRole() != Ieee8021dInterfaceData::ROOT)
            && (iPort->getRole() != Ieee8021dInterfaceData::ALTERNATE)
            && (iPort->getRole() != Ieee8021dInterfaceData::DISABLED) && (!iPort->isEdge()))
        {
            sendBPDU(i);
        }
    }
}

void RSTP::sendBPDU(int port)
{
    // send a BPDU throuth port
    Ieee8021dInterfaceData *iport = getPortInterfaceData(port);
    int r = getRootIndex();
    Ieee8021dInterfaceData *rootPort;
    if (r != -1)
        rootPort = getPortInterfaceData(r);
    if (iport->getRole() != Ieee8021dInterfaceData::DISABLED) {
        BPDU *frame = new BPDU();
        Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
        if (r != -1) {
            frame->setRootPriority(rootPort->getRootPriority());
            frame->setRootAddress(rootPort->getRootAddress());
            frame->setMessageAge(rootPort->getAge());
            frame->setRootPathCost(rootPort->getRootPathCost());
        }
        else {
            frame->setRootPriority(bridgePriority);
            frame->setRootAddress(bridgeAddress);
            frame->setMessageAge(0);
            frame->setRootPathCost(0);
        }
        frame->setBridgePriority(bridgePriority);
        frame->setTcaFlag(false);
        frame->setPortNum(port);
        frame->setBridgeAddress(bridgeAddress);
        if (simTime() < iport->getTCWhile())
            frame->setTcFlag(true);
        else
            frame->setTcFlag(false);
        frame->setName("BPDU");
        frame->setMaxAge(maxAge);
        frame->setHelloTime(helloTime);
        frame->setForwardDelay(forwardDelay);
        if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
            frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);
        etherctrl->setSrc(bridgeAddress);
        etherctrl->setDest(MACAddress::STP_MULTICAST_ADDRESS);
        etherctrl->setSwitchPort(port);
        frame->setControlInfo(etherctrl);
        send(frame, "relayOut");
    }
}

void RSTP::printState()
{
    //  prints current database info
    EV_DETAIL << "Switch " << findContainingNode(this)->getFullName() << " state:" << endl;
    int rootIndex = getRootIndex();
    EV_DETAIL << "  Priority: " << bridgePriority << endl;
    EV_DETAIL << "  Local MAC: " << bridgeAddress << endl;
    if (rootIndex >= 0) {
        Ieee8021dInterfaceData *rootPort = getPortInterfaceData(rootIndex);
        EV_DETAIL << "  Root Priority: " << rootPort->getRootPriority() << endl;
        EV_DETAIL << "  Root Address: " << rootPort->getRootAddress().str() << endl;
        EV_DETAIL << "  Cost: " << rootPort->getRootPathCost() << endl;
        EV_DETAIL << "  Age:  " << rootPort->getAge() << endl;
        EV_DETAIL << "  Bridge Priority: " << rootPort->getBridgePriority() << endl;
        EV_DETAIL << "  Bridge Address: " << rootPort->getBridgeAddress().str() << endl;
        EV_DETAIL << "  Src TxGate Priority: " << rootPort->getPortPriority() << endl;
        EV_DETAIL << "  Src TxGate: " << rootPort->getPortNum() << endl;
    }
    EV_DETAIL << "Port State/Role:" << endl;
    for (unsigned int i = 0; i < numPorts; i++) {
        Ieee8021dInterfaceData *iPort = getPortInterfaceData(i);
        EV_DETAIL << "  " << i << ": " << iPort->getStateName() << "/" << iPort->getRoleName() << (iPort->isEdge() ? " (Client)" : "") << endl;
    }
    EV_DETAIL << "Per-port best sources, Root/Src:" << endl;
    for (unsigned int i = 0; i < numPorts; i++) {
        Ieee8021dInterfaceData *iPort = getPortInterfaceData(i);
        EV_DETAIL << "  " << i << ": " << iPort->getRootAddress().str() << "/" << iPort->getBridgeAddress().str() << endl;
    }
    EV_DETAIL << endl;
}

void RSTP::initInterfacedata(unsigned int portNum)
{
    Ieee8021dInterfaceData *ifd = getPortInterfaceData(portNum);
    ifd->setRootPriority(bridgePriority);
    ifd->setRootAddress(bridgeAddress);
    ifd->setRootPathCost(0);
    ifd->setAge(0);
    ifd->setBridgePriority(bridgePriority);
    ifd->setBridgeAddress(bridgeAddress);
    ifd->setPortPriority(-1);
    ifd->setPortNum(-1);
    ifd->setLostBPDU(0);
}

void RSTP::initPorts()
{
    for (unsigned int j = 0; j < numPorts; j++) {
        Ieee8021dInterfaceData *jPort = getPortInterfaceData(j);
        if (!jPort->isEdge()) {
            jPort->setRole(Ieee8021dInterfaceData::NOTASSIGNED);
            jPort->setState(Ieee8021dInterfaceData::DISCARDING);
            jPort->setNextUpgrade(simTime() + migrateTime);
        }
        else {
            jPort->setRole(Ieee8021dInterfaceData::DESIGNATED);
            jPort->setState(Ieee8021dInterfaceData::FORWARDING);
        }
        initInterfacedata(j);
        macTable->flush(j);
    }
    scheduleNextUpgrade();
}

void RSTP::updateInterfacedata(BPDU *frame, unsigned int portNum)
{
    Ieee8021dInterfaceData *ifd = getPortInterfaceData(portNum);
    ifd->setRootPriority(frame->getRootPriority());
    ifd->setRootAddress(frame->getRootAddress());
    ifd->setRootPathCost(frame->getRootPathCost() + ifd->getLinkCost());
    ifd->setAge(frame->getMessageAge() + 1);
    ifd->setBridgePriority(frame->getBridgePriority());
    ifd->setBridgeAddress(frame->getBridgeAddress());
    ifd->setPortPriority(frame->getPortPriority());
    ifd->setPortNum(frame->getPortNum());
    ifd->setLostBPDU(0);
}

RSTP::CompareResult RSTP::contestInterfacedata(unsigned int portNum)
{
    int r = getRootIndex();
    Ieee8021dInterfaceData *rootPort = getPortInterfaceData(r);
    Ieee8021dInterfaceData *ifd = getPortInterfaceData(portNum);

    return compareRSTPData(rootPort->getRootPriority(), ifd->getRootPriority(),
            rootPort->getRootAddress(), ifd->getRootAddress(),
            rootPort->getRootPathCost() + ifd->getLinkCost(), ifd->getRootPathCost(),
            bridgePriority, ifd->getBridgePriority(),
            bridgeAddress, ifd->getBridgeAddress(),
            ifd->getPortPriority(), ifd->getPortPriority(),
            portNum, ifd->getPortNum());
}

RSTP::CompareResult RSTP::contestInterfacedata(BPDU *msg, unsigned int portNum)
{
    int r = getRootIndex();
    Ieee8021dInterfaceData *rootPort = getPortInterfaceData(r);
    Ieee8021dInterfaceData *ifd = getPortInterfaceData(portNum);

    return compareRSTPData(rootPort->getRootPriority(), msg->getRootPriority(),
            rootPort->getRootAddress(), msg->getRootAddress(),
            rootPort->getRootPathCost(), msg->getRootPathCost(),
            bridgePriority, msg->getBridgePriority(),
            bridgeAddress, msg->getBridgeAddress(),
            ifd->getPortPriority(), msg->getPortPriority(),
            portNum, msg->getPortNum());
}

RSTP::CompareResult RSTP::compareInterfacedata(unsigned int portNum, BPDU *msg, int linkCost)
{
    Ieee8021dInterfaceData *ifd = getPortInterfaceData(portNum);

    return compareRSTPData(ifd->getRootPriority(), msg->getRootPriority(),
            ifd->getRootAddress(), msg->getRootAddress(),
            ifd->getRootPathCost(), msg->getRootPathCost() + linkCost,
            ifd->getBridgePriority(), msg->getBridgePriority(),
            ifd->getBridgeAddress(), msg->getBridgeAddress(),
            ifd->getPortPriority(), msg->getPortPriority(),
            ifd->getPortNum(), msg->getPortNum());
}

RSTP::CompareResult RSTP::compareRSTPData(int rootPriority1, int rootPriority2,
        MACAddress rootAddress1, MACAddress rootAddress2,
        int rootPathCost1, int rootPathCost2,
        int bridgePriority1, int bridgePriority2,
        MACAddress bridgeAddress1, MACAddress bridgeAddress2,
        int portPriority1, int portPriority2,
        int portNum1, int portNum2)
{
    if (rootPriority1 != rootPriority2)
        return (rootPriority1 < rootPriority2) ? WORSE_ROOT : BETTER_ROOT;

    int c = rootAddress1.compareTo(rootAddress2);
    if (c != 0)
        return (c < 0) ? WORSE_ROOT : BETTER_ROOT;

    if (rootPathCost1 != rootPathCost2)
        return (rootPathCost1 < rootPathCost2) ? WORSE_RPC : BETTER_RPC;

    if (bridgePriority1 != bridgePriority2)
        return (bridgePriority1 < bridgePriority2) ? WORSE_SRC : BETTER_SRC;

    c = bridgeAddress1.compareTo(bridgeAddress2);
    if (c != 0)
        return (c < 0) ? WORSE_SRC : BETTER_SRC;

    if (portPriority1 != portPriority2)
        return (portPriority1 < portPriority2) ? WORSE_PORT : BETTER_PORT;

    if (portNum1 != portNum2)
        return (portNum1 < portNum2) ? WORSE_PORT : BETTER_PORT;

    return SIMILAR;
}

int RSTP::getBestAlternate()
{
    int candidate = -1;    // index of the best alternate found
    for (unsigned int j = 0; j < numPorts; j++) {
        Ieee8021dInterfaceData *jPort = getPortInterfaceData(j);
        if (jPort->getRole() == Ieee8021dInterfaceData::ALTERNATE) {    // just from alternates, others are not updated
            if (candidate < 0)
                candidate = j;
            else {
                Ieee8021dInterfaceData *candidatePort = getPortInterfaceData(candidate);
                if (compareRSTPData(jPort->getRootPriority(), candidatePort->getRootPriority(),
                            jPort->getRootAddress(), candidatePort->getRootAddress(),
                            jPort->getRootPathCost(), candidatePort->getRootPathCost(),
                            jPort->getBridgePriority(), candidatePort->getBridgePriority(),
                            jPort->getBridgeAddress(), candidatePort->getBridgeAddress(),
                            jPort->getPortPriority(), candidatePort->getPortPriority(),
                            jPort->getPortNum(), candidatePort->getPortNum()) < 0)
                {
                    // alternate better than the found one
                    candidate = j;    // new candidate
                }
            }
        }
    }
    return candidate;
}

void RSTP::flushOtherPorts(unsigned int portNum)
{
    for (unsigned int i = 0; i < numPorts; i++) {
        Ieee8021dInterfaceData *iPort = getPortInterfaceData(i);
        iPort->setTCWhile(simulation.getSimTime() + tcWhileTime);
        if (i != portNum)
            macTable->flush(i);
    }
}

//void RSTP::receiveChangeNotification(int signalID, const cObject *obj)
void RSTP::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    Enter_Method_Silent();

    if (signalID == NF_INTERFACE_STATE_CHANGED) {
        InterfaceEntry *changedIE = check_and_cast<const InterfaceEntryChangeDetails *>(obj)->getInterfaceEntry();
        for (unsigned int i = 0; i < numPorts; i++) {
            InterfaceEntry *gateIfEntry = getPortInterfaceEntry(i);
            if (gateIfEntry == changedIE) {
                if (gateIfEntry->hasCarrier()) {
                    Ieee8021dInterfaceData *iPort = getPortInterfaceData(i);
                    if (iPort->getRole() == Ieee8021dInterfaceData::NOTASSIGNED)
                        iPort->setNextUpgrade(simTime() + migrateTime);
                    else if (iPort->getRole() == Ieee8021dInterfaceData::DESIGNATED
                             && (iPort->getState() == Ieee8021dInterfaceData::DISCARDING || iPort->getState() == Ieee8021dInterfaceData::LEARNING))
                        iPort->setNextUpgrade(simTime() + forwardDelay);
                    scheduleNextUpgrade();
                }
            }
        }
    }
}

void RSTP::start()
{
    STPBase::start();
    initPorts();
    scheduleAt(simTime(), helloTimer);
}

void RSTP::stop()
{
    STPBase::stop();
    cancelEvent(helloTimer);
    cancelEvent(upgradeTimer);
}

} // namespace inet

