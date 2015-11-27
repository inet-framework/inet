//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 3
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
// Authors: Tomas Prochazka (xproch21@stud.fit.vutbr.cz), Veronika Rybova, Vladimir Vesely (ivesely@fit.vutbr.cz)
//          Tamas Borbely (tomi@omnetpp.org)

#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/NotifierConsts.h"
#include "inet/routing/pim/modes/PIMSM.h"

namespace inet {
Define_Module(PIMSM);

simsignal_t PIMSM::sentRegisterPkSignal = registerSignal("sentRegisterPk");
simsignal_t PIMSM::rcvdRegisterPkSignal = registerSignal("rcvdRegisterPk");
simsignal_t PIMSM::sentRegisterStopPkSignal = registerSignal("sentRegisterStopPk");
simsignal_t PIMSM::rcvdRegisterStopPkSignal = registerSignal("rcvdRegisterStopPk");
simsignal_t PIMSM::sentJoinPrunePkSignal = registerSignal("sentJoinPrunePk");
simsignal_t PIMSM::rcvdJoinPrunePkSignal = registerSignal("rcvdJoinPrunePk");
simsignal_t PIMSM::sentAssertPkSignal = registerSignal("sentAssertPk");
simsignal_t PIMSM::rcvdAssertPkSignal = registerSignal("rcvdAssertPk");

std::ostream& operator<<(std::ostream& out, const PIMSM::SourceAndGroup& sourceGroup)
{
    out << "(" << (sourceGroup.source.isUnspecified() ? "*" : sourceGroup.source.str()) << ", "
        << (sourceGroup.group.isUnspecified() ? "*" : sourceGroup.group.str()) << ")";
    return out;
}

std::ostream& operator<<(std::ostream& out, const PIMSM::Route& route)
{
    out << "(" << (route.source.isUnspecified() ? "*" : route.source.str()) << ", "
        << (route.group.isUnspecified() ? "*" : route.group.str()) << "), ";
    out << "RP is " << route.rpAddr << ", ";

    out << "Incoming interface: ";
    if (route.upstreamInterface) {
        out << route.upstreamInterface->ie->getName() << ", ";
        out << "RPF neighbor: " << route.upstreamInterface->rpfNeighbor() << ", ";
    }
    else
        out << "nullptr, ";

    out << "Downstream interfaces: ";
    for (unsigned int i = 0; i < route.downstreamInterfaces.size(); ++i) {
        if (i > 0)
            out << ", ";
        out << route.downstreamInterfaces[i]->ie->getName() << " ";
        switch (route.downstreamInterfaces[i]->joinPruneState) {
            case PIMSM::DownstreamInterface::NO_INFO:
                out << "(NI)";
                break;

            case PIMSM::DownstreamInterface::JOIN:
                out << "(J)";
                break;

            case PIMSM::DownstreamInterface::PRUNE_PENDING:
                out << "(PP)";
                break;
        }
    }

    return out;
}

PIMSM::~PIMSM()
{
    for (auto & elem : gRoutes)
        delete elem.second;
    for (auto & elem : sgRoutes)
        delete elem.second;
    // XXX rt contains references to the deleted route entries
}

void PIMSM::initialize(int stage)
{
    PIMBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        const char *rp = par("RP");
        if (rp && *rp)
            rpAddr = IPv4Address(rp);

        joinPrunePeriod = par("joinPrunePeriod");
        defaultOverrideInterval = par("defaultOverrideInterval");
        defaultPropagationDelay = par("defaultPropagationDelay");
        keepAlivePeriod = par("keepAlivePeriod");
        rpKeepAlivePeriod = par("rpKeepAlivePeriod");
        registerSuppressionTime = par("registerSuppressionTime");
        registerProbeTime = par("registerProbeTime");
        assertTime = par("assertTime");
        assertOverrideInterval = par("assertOverrideInterval");

        WATCH_PTRMAP(gRoutes);
        WATCH_PTRMAP(sgRoutes);
    }
}

bool PIMSM::handleNodeStart(IDoneCallback *doneCallback)
{
    bool done = PIMBase::handleNodeStart(doneCallback);

    if (isEnabled) {
        if (rpAddr.isUnspecified())
            throw cRuntimeError("PIMSM: missing RP address parameter.");

        // subscribe for notifications
        cModule *host = findContainingNode(this);
        if (!host)
            throw cRuntimeError("PIMDM: containing node not found.");
        host->subscribe(NF_IPv4_NEW_MULTICAST, this);
        host->subscribe(NF_IPv4_MDATA_REGISTER, this);
        host->subscribe(NF_IPv4_DATA_ON_RPF, this);
        host->subscribe(NF_IPv4_DATA_ON_NONRPF, this);
        host->subscribe(NF_IPv4_MCAST_REGISTERED, this);
        host->subscribe(NF_IPv4_MCAST_UNREGISTERED, this);
        host->subscribe(NF_PIM_NEIGHBOR_ADDED, this);
        host->subscribe(NF_PIM_NEIGHBOR_DELETED, this);
        host->subscribe(NF_PIM_NEIGHBOR_CHANGED, this);
    }

    return done;
}

bool PIMSM::handleNodeShutdown(IDoneCallback *doneCallback)
{
    // TODO send PIM Hellos to neighbors with 0 HoldTime
    stopPIMRouting();
    return PIMBase::handleNodeShutdown(doneCallback);
}

void PIMSM::handleNodeCrash()
{
    stopPIMRouting();
    PIMBase::handleNodeCrash();
}

void PIMSM::stopPIMRouting()
{
    if (isEnabled) {
        cModule *host = findContainingNode(this);
        if (!host)
            throw cRuntimeError("PIMSM: containing node not found.");
        host->unsubscribe(NF_IPv4_NEW_MULTICAST, this);
        host->unsubscribe(NF_IPv4_MDATA_REGISTER, this);
        host->unsubscribe(NF_IPv4_DATA_ON_RPF, this);
        host->unsubscribe(NF_IPv4_DATA_ON_NONRPF, this);
        host->unsubscribe(NF_IPv4_MCAST_REGISTERED, this);
        host->unsubscribe(NF_IPv4_MCAST_UNREGISTERED, this);
        host->unsubscribe(NF_PIM_NEIGHBOR_ADDED, this);
        host->unsubscribe(NF_PIM_NEIGHBOR_DELETED, this);
        host->unsubscribe(NF_PIM_NEIGHBOR_CHANGED, this);
    }

    clearRoutes();
}

void PIMSM::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        switch (msg->getKind()) {
            case HelloTimer:
                processHelloTimer(msg);
                break;

            case JoinTimer:
                processJoinTimer(msg);
                break;

            case PrunePendingTimer:
                processPrunePendingTimer(msg);
                break;

            case ExpiryTimer:
                processExpiryTimer(msg);
                break;

            case KeepAliveTimer:
                processKeepAliveTimer(msg);
                break;

            case RegisterStopTimer:
                processRegisterStopTimer(msg);
                break;

            case AssertTimer:
                processAssertTimer(msg);
                break;

            default:
                throw cRuntimeError("PIMSM: unknown self message: %s (%s)", msg->getName(), msg->getClassName());
        }
    }
    else if (dynamic_cast<PIMPacket *>(msg)) {
        if (!isEnabled) {
            EV_DETAIL << "PIM-SM is disabled, dropping packet.\n";
            delete msg;
            return;
        }

        PIMPacket *pkt = check_and_cast<PIMPacket *>(msg);
        switch (pkt->getType()) {
            case Hello:
                processHelloPacket(check_and_cast<PIMHello *>(pkt));
                break;

            case JoinPrune:
                processJoinPrunePacket(check_and_cast<PIMJoinPrune *>(pkt));
                break;

            case Register:
                processRegisterPacket(check_and_cast<PIMRegister *>(pkt));
                break;

            case RegisterStop:
                processRegisterStopPacket(check_and_cast<PIMRegisterStop *>(pkt));
                break;

            case Assert:
                processAssertPacket(check_and_cast<PIMAssert *>(pkt));
                break;

            case Graft:
                EV_WARN << "Ignoring PIM-DM Graft packet.\n";
                delete pkt;
                break;

            case GraftAck:
                EV_WARN << "Ignoring PIM-DM GraftAck packet.\n";
                delete pkt;
                break;

            case StateRefresh:
                EV_WARN << "Ignoring PIM-DM StateRefresh packet.\n";
                delete pkt;
                break;

            case Bootstrap:
                delete pkt;
                break;

            case CandidateRPAdvertisement:
                delete pkt;
                break;

            default:
                throw cRuntimeError("PIMSM: received unknown PIM packet: %s (%s)", pkt->getName(), pkt->getClassName());
        }
    }
    else
        EV << "PIMSM::handleMessage: Wrong message" << endl;
}

void PIMSM::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG)
{
    Enter_Method_Silent();
    printNotificationBanner(signalID, obj);
    Route *route;
    IPv4Datagram *datagram;
    PIMInterface *pimInterface;

    if (signalID == NF_IPv4_MCAST_REGISTERED) {
        EV << "pimSM::receiveChangeNotification - NEW IGMP ADDED" << endl;
        IPv4MulticastGroupInfo *info = check_and_cast<IPv4MulticastGroupInfo *>(obj);
        pimInterface = pimIft->getInterfaceById(info->ie->getInterfaceId());
        if (pimInterface && pimInterface->getMode() == PIMInterface::SparseMode)
            multicastReceiverAdded(info->ie, info->groupAddress);
    }
    else if (signalID == NF_IPv4_MCAST_UNREGISTERED) {
        EV << "pimSM::receiveChangeNotification - IGMP REMOVED" << endl;
        IPv4MulticastGroupInfo *info = check_and_cast<IPv4MulticastGroupInfo *>(obj);
        pimInterface = pimIft->getInterfaceById(info->ie->getInterfaceId());
        if (pimInterface && pimInterface->getMode() == PIMInterface::SparseMode)
            multicastReceiverRemoved(info->ie, info->groupAddress);
    }
    else if (signalID == NF_IPv4_NEW_MULTICAST) {
        EV << "PimSM::receiveChangeNotification - NEW MULTICAST" << endl;
        datagram = check_and_cast<IPv4Datagram *>(obj);
        IPv4Address srcAddr = datagram->getSrcAddress();
        IPv4Address destAddr = datagram->getDestAddress();
        unroutableMulticastPacketArrived(srcAddr, destAddr);
    }
    else if (signalID == NF_IPv4_DATA_ON_RPF) {
        EV << "pimSM::receiveChangeNotification - DATA ON RPF" << endl;
        datagram = check_and_cast<IPv4Datagram *>(obj);
        PIMInterface *incomingInterface = getIncomingInterface(datagram);
        if (incomingInterface && incomingInterface->getMode() == PIMInterface::SparseMode) {
            route = findRouteG(datagram->getDestAddress());
            if (route)
                multicastPacketArrivedOnRpfInterface(route);
            route = findRouteSG(datagram->getSrcAddress(), datagram->getDestAddress());
            if (route)
                multicastPacketArrivedOnRpfInterface(route);
        }
    }
    else if (signalID == NF_IPv4_DATA_ON_NONRPF) {
        datagram = check_and_cast<IPv4Datagram *>(obj);
        PIMInterface *incomingInterface = getIncomingInterface(datagram);
        if (incomingInterface && incomingInterface->getMode() == PIMInterface::SparseMode) {
            IPv4Address srcAddr = datagram->getSrcAddress();
            IPv4Address destAddr = datagram->getDestAddress();
            if ((route = findRouteSG(srcAddr, destAddr)) != nullptr)
                multicastPacketArrivedOnNonRpfInterface(route, incomingInterface->getInterfaceId());
            else if ((route = findRouteG(destAddr)) != nullptr)
                multicastPacketArrivedOnNonRpfInterface(route, incomingInterface->getInterfaceId());
        }
    }
    else if (signalID == NF_IPv4_MDATA_REGISTER) {
        EV << "pimSM::receiveChangeNotification - REGISTER DATA" << endl;
        datagram = check_and_cast<IPv4Datagram *>(obj);
        PIMInterface *incomingInterface = getIncomingInterface(datagram);
        route = findRouteSG(datagram->getSrcAddress(), datagram->getDestAddress());
        if (incomingInterface && incomingInterface->getMode() == PIMInterface::SparseMode)
            multicastPacketForwarded(datagram);
    }
    else if (signalID == NF_PIM_NEIGHBOR_ADDED || signalID == NF_PIM_NEIGHBOR_DELETED || signalID == NF_PIM_NEIGHBOR_CHANGED) {
        PIMNeighbor *neighbor = check_and_cast<PIMNeighbor *>(obj);
        updateDesignatedRouterAddress(neighbor->getInterfacePtr());
    }
}

void PIMSM::processJoinPrunePacket(PIMJoinPrune *pkt)
{
    EV_INFO << "Received JoinPrune packet.\n";

    emit(rcvdJoinPrunePkSignal, pkt);

    IPv4ControlInfo *ctrl = check_and_cast<IPv4ControlInfo *>(pkt->getControlInfo());
    InterfaceEntry *inInterface = ift->getInterfaceById(ctrl->getInterfaceId());
    int holdTime = pkt->getHoldTime();
    IPv4Address upstreamNeighbor = pkt->getUpstreamNeighborAddress();

    for (unsigned int i = 0; i < pkt->getJoinPruneGroupsArraySize(); i++) {
        JoinPruneGroup group = pkt->getJoinPruneGroups(i);
        IPv4Address groupAddr = group.getGroupAddress();

        // go through list of joined sources
        for (unsigned int j = 0; j < group.getJoinedSourceAddressArraySize(); j++) {
            EncodedAddress& source = group.getJoinedSourceAddress(j);
            if (source.S) {
                if (source.W) // (*,G) Join
                    processJoinG(groupAddr, source.IPaddress, upstreamNeighbor, holdTime, inInterface);
                else if (source.R) // (S,G,rpt) Join
                    processJoinSGrpt(source.IPaddress, groupAddr, upstreamNeighbor, holdTime, inInterface);
                else // (S,G) Join
                    processJoinSG(source.IPaddress, groupAddr, upstreamNeighbor, holdTime, inInterface);
            }
        }

        // go through list of pruned sources
        for (unsigned int j = 0; j < group.getPrunedSourceAddressArraySize(); j++) {
            EncodedAddress& source = group.getPrunedSourceAddress(j);
            if (source.S) {
                if (source.W) // (*,G) Prune
                    processPruneG(groupAddr, upstreamNeighbor, inInterface);
                else if (source.R) // (S,G,rpt) Prune
                    processPruneSGrpt(source.IPaddress, groupAddr, upstreamNeighbor, inInterface);
                else // (S,G) Prune
                    processPruneSG(source.IPaddress, groupAddr, upstreamNeighbor, inInterface);
            }
        }
    }

    delete pkt;
}

void PIMSM::processJoinG(IPv4Address group, IPv4Address rp, IPv4Address upstreamNeighborField, int holdTime, InterfaceEntry *inInterface)
{
    EV_DETAIL << "Processing Join(*," << group << ") received on interface '" << inInterface->getName() << "'.\n";

    // TODO RP check

    //
    // Downstream per-interface (*,G) state machine; event = Receive Join(*,G)
    //

    // check UpstreamNeighbor field
    if (upstreamNeighborField != inInterface->ipv4Data()->getIPAddress())
        return;

    bool newRoute = false;
    Route *routeG = findRouteG(group);
    if (!routeG) {
        routeG = addNewRouteG(group, Route::PRUNED);
        newRoute = true;
    }

    DownstreamInterface *downstream = routeG->findDownstreamInterfaceByInterfaceId(inInterface->getInterfaceId());

    if (downstream) {
        // A Join(*,G) is received on interface I with its Upstream
        // Neighbor Address set to the router's primary IP address on I.
        if (downstream->joinPruneState == DownstreamInterface::NO_INFO) {
            // The (*,G) downstream state machine on interface I transitions
            // to the Join state.  The Expiry Timer (ET) is started and set
            // to the HoldTime from the triggering Join/Prune message.
            downstream->joinPruneState = DownstreamInterface::JOIN;
            downstream->startExpiryTimer(holdTime);
        }
        else if (downstream->joinPruneState == DownstreamInterface::JOIN) {
            // The (*,G) downstream state machine on interface I remains in
            // Join state, and the Expiry Timer (ET) is restarted, set to
            // maximum of its current value and the HoldTime from the
            // triggering Join/Prune message.
            if (simTime() + holdTime > downstream->expiryTimer->getArrivalTime())
                restartTimer(downstream->expiryTimer, holdTime);
        }
        else if (downstream->joinPruneState == DownstreamInterface::PRUNE_PENDING) {
            // The (*,G) downstream state machine on interface I transitions
            // to the Join state.  The Prune-Pending Timer is canceled
            // (without triggering an expiry event).  The Expiry Timer is
            // restarted, set to maximum of its current value and the
            // HoldTime from the triggering Join/Prune message.
            cancelAndDeleteTimer(downstream->prunePendingTimer);
            if (simTime() + holdTime > downstream->expiryTimer->getArrivalTime())
                restartTimer(downstream->expiryTimer, holdTime);
        }
    }

    // XXX Join(*,G) messages can affect (S,G,rpt) downstream state machines too

    // check upstream state transition
    updateJoinDesired(routeG);

    if (!newRoute && !routeG->upstreamInterface) {    // I am RP
        routeG->clearFlag(Route::REGISTER);
    }
}

void PIMSM::processJoinSG(IPv4Address source, IPv4Address group, IPv4Address upstreamNeighborField, int holdTime, InterfaceEntry *inInterface)
{
    EV_DETAIL << "Processing Join(" << source << ", " << group << ") received on interface '" << inInterface->getName() << "'.'n";

    //
    // Downstream per-interface (S,G) state machine; event = Receive Join(S,G)
    //

    // check UpstreamNeighbor field
    if (upstreamNeighborField != inInterface->ipv4Data()->getIPAddress())
        return;

    Route *routeSG = findRouteSG(source, group);
    if (!routeSG) {    // create (S,G) route between RP and source DR
        routeSG = addNewRouteSG(source, group, Route::PRUNED);
        routeSG->startKeepAliveTimer(keepAlivePeriod);
    }

    DownstreamInterface *downstream = routeSG->findDownstreamInterfaceByInterfaceId(inInterface->getInterfaceId());

    if (downstream) {
        // A Join(S,G) is received on interface I with its Upstream
        // Neighbor Address set to the router's primary IP address on I.
        if (downstream->joinPruneState == DownstreamInterface::NO_INFO) {
            // The (S,G) downstream state machine on interface I transitions
            // to the Join state.  The Expiry Timer (ET) is started and set
            // to the HoldTime from the triggering Join/Prune message.
            downstream->joinPruneState = DownstreamInterface::JOIN;
            downstream->startExpiryTimer(holdTime);
        }
        else if (downstream->joinPruneState == DownstreamInterface::JOIN) {
            // The (S,G) downstream state machine on interface I remains in
            // Join state, and the Expiry Timer (ET) is restarted, set to
            // maximum of its current value and the HoldTime from the
            // triggering Join/Prune message.
            if (simTime() + holdTime > downstream->expiryTimer->getArrivalTime())
                restartTimer(downstream->expiryTimer, holdTime);
        }
        else if (downstream->joinPruneState == DownstreamInterface::PRUNE_PENDING) {
            // The (S,G) downstream state machine on interface I transitions
            // to the Join state.  The Prune-Pending Timer is canceled
            // (without triggering an expiry event).  The Expiry Timer is
            // restarted, set to maximum of its current value and the
            // HoldTime from the triggering Join/Prune message.
            cancelAndDeleteTimer(downstream->prunePendingTimer);
            if (simTime() + holdTime > downstream->expiryTimer->getArrivalTime())
                restartTimer(downstream->expiryTimer, holdTime);
        }
    }

    // check upstream state transition
    updateJoinDesired(routeSG);
}

void PIMSM::processJoinSGrpt(IPv4Address source, IPv4Address group, IPv4Address upstreamNeighborField, int holdTime, InterfaceEntry *inInterface)
{
    // TODO
}

void PIMSM::processPruneG(IPv4Address group, IPv4Address upstreamNeighborField, InterfaceEntry *inInterface)
{
    EV_DETAIL << "Processing Prune(*," << group << ") received on interface '" << inInterface->getName() << "'.\n";

    //
    // Downstream per-interface (*,G) state machine; event = Receive Prune(*,G)
    //

    // check UpstreamNeighbor field
    if (upstreamNeighborField != inInterface->ipv4Data()->getIPAddress())
        return;

    Route *routeG = findRouteG(group);
    if (routeG) {
        DownstreamInterface *downstream = routeG->findDownstreamInterfaceByInterfaceId(inInterface->getInterfaceId());
        if (downstream && downstream->joinPruneState == DownstreamInterface::JOIN) {
            // The (*,G) downstream state machine on interface I transitions
            // to the Prune-Pending state.  The Prune-Pending Timer is
            // started.  It is set to the J/P_Override_Interval(I) if the
            // router has more than one neighbor on that interface;
            // otherwise, it is set to zero, causing it to expire
            // immediately.
            downstream->joinPruneState = DownstreamInterface::PRUNE_PENDING;
            double pruneOverrideInterval = pimNbt->getNumNeighbors(inInterface->getInterfaceId()) > 1 ?
                joinPruneOverrideInterval() : 0.0;
            downstream->startPrunePendingTimer(pruneOverrideInterval);
        }
    }

    // check upstream state transition
    if (routeG)
        updateJoinDesired(routeG);
}

void PIMSM::processPruneSG(IPv4Address source, IPv4Address group, IPv4Address upstreamNeighborField, InterfaceEntry *inInterface)
{
    EV_DETAIL << "Processing Prune(" << source << ", " << group << ") received on interface '" << inInterface->getName() << "'.'n";

    //
    // Downstream per-interface (S,G) state machine; event = Receive Prune(S,G)
    //

    // check UpstreamNeighbor field
    if (upstreamNeighborField != inInterface->ipv4Data()->getIPAddress())
        return;

    Route *routeSG = findRouteSG(source, group);
    if (routeSG) {
        DownstreamInterface *downstream = routeSG->findDownstreamInterfaceByInterfaceId(inInterface->getInterfaceId());
        if (downstream && downstream->joinPruneState == DownstreamInterface::JOIN) {
            // The (S,G) downstream state machine on interface I transitions
            // to the Prune-Pending state.  The Prune-Pending Timer is
            // started.  It is set to the J/P_Override_Interval(I) if the
            // router has more than one neighbor on that interface;
            // otherwise, it is set to zero, causing it to expire
            // immediately.
            downstream->joinPruneState = DownstreamInterface::PRUNE_PENDING;
            double pruneOverrideInterval = pimNbt->getNumNeighbors(inInterface->getInterfaceId()) > 1 ?
                joinPruneOverrideInterval() : 0.0;
            downstream->startPrunePendingTimer(pruneOverrideInterval);
        }
    }

    // check upstream state transition
    if (routeSG)
        updateJoinDesired(routeSG);
}

void PIMSM::processPruneSGrpt(IPv4Address source, IPv4Address group, IPv4Address upstreamNeighborField, InterfaceEntry *inInterface)
{
    // TODO
}

/**
 * The method is used for processing PIM Register message sent from source DR.
 * If PIM Register isn't Null and route doesn't exist, it is created and PIM Register-Stop
 * is sent. If PIM Register is Null, Register-Stop is sent.
 */
void PIMSM::processRegisterPacket(PIMRegister *pkt)
{
    EV_INFO << "Received Register packet.\n";

    emit(rcvdRegisterPkSignal, pkt);

    IPv4Datagram *encapData = check_and_cast<IPv4Datagram *>(pkt->decapsulate());
    IPv4Address source = encapData->getSrcAddress();
    IPv4Address group = encapData->getDestAddress();
    Route *routeG = findRouteG(group);
    Route *routeSG = findRouteSG(source, group);

    if (!pkt->getN()) {    // It is Null Register ?
        if (!routeG) {
            routeG = addNewRouteG(group, Route::PRUNED);
        }

        if (!routeSG) {
            routeSG = addNewRouteSG(source, group, Route::PRUNED);
            routeSG->startKeepAliveTimer(keepAlivePeriod);
        }
        else if (routeSG->keepAliveTimer) {
            EV << " (S,G) KAT timer refresh" << endl;
            restartTimer(routeSG->keepAliveTimer, KAT);
        }

        if (!routeG->isInheritedOlistNull()) {    // we have some active receivers
            routeSG->clearFlag(Route::PRUNED);

            if (!routeSG->isFlagSet(Route::SPT_BIT)) {    // only if isn't build SPT between RP and registering DR
                // decapsulate and forward the inner packet to inherited_olist(S,G,rpt)
                // XXX we are forwarding on inherited_olist(*,G)
                for (auto & elem : routeG->downstreamInterfaces) {
                    DownstreamInterface *downstream = elem;
                    if (downstream->isInInheritedOlist())
                        forwardMulticastData(encapData->dup(), downstream->getInterfaceId());
                }

                // send Join(S,G) toward source to establish SPT between RP and registering DR
                sendPIMJoin(group, source, routeSG->upstreamInterface->rpfNeighbor(), SG);

                // send register-stop packet
                IPv4ControlInfo *ctrlInfo = check_and_cast<IPv4ControlInfo *>(pkt->getControlInfo());
                sendPIMRegisterStop(ctrlInfo->getDestAddr(), ctrlInfo->getSrcAddr(), group, source);
            }
        }
    }

    if (routeG) {
        if (routeG->isFlagSet(Route::PRUNED) || pkt->getN()) {
            IPv4ControlInfo *ctrlInfo = check_and_cast<IPv4ControlInfo *>(pkt->getControlInfo());
            sendPIMRegisterStop(ctrlInfo->getDestAddr(), ctrlInfo->getSrcAddr(), group, source);
        }
    }

    delete encapData;
    delete pkt;
}

/**
 * The method is used for processing PIM Register-Stop message sent from RP.
 * If the message is received Register Tunnel between RP and source DR is set
 * from Join status revert to Prune status. Also Register Stop Timer is created
 * for periodic sending PIM Register Null messages.
 */
void PIMSM::processRegisterStopPacket(PIMRegisterStop *pkt)
{
    EV_INFO << "Received RegisterStop packet.\n";

    emit(rcvdRegisterStopPkSignal, pkt);

    // TODO support wildcard source address
    Route *routeSG = findRouteSG(pkt->getSourceAddress(), pkt->getGroupAddress());
    if (routeSG) {
        if (routeSG->registerState == Route::RS_JOIN || routeSG->registerState == Route::RS_JOIN_PENDING) {
            routeSG->registerState = Route::RS_PRUNE;
            cancelAndDeleteTimer(routeSG->registerStopTimer);

            // The Register-Stop Timer is set to a random value chosen
            // uniformly from the interval ( 0.5 * Register_Suppression_Time,
            // 1.5 * Register_Suppression_Time) minus Register_Probe_Time.
            // Subtracting off Register_Probe_Time is a bit unnecessary because
            // it is really small compared to Register_Suppression_Time, but
            // this was in the old spec and is kept for compatibility.
            routeSG->startRegisterStopTimer(uniform(0.5 * registerSuppressionTime, 1.5 * registerSuppressionTime) - registerProbeTime);
        }
    }
}

void PIMSM::processAssertPacket(PIMAssert *pkt)
{
    IPv4ControlInfo *ctrlInfo = check_and_cast<IPv4ControlInfo *>(pkt->getControlInfo());
    int incomingInterfaceId = ctrlInfo->getInterfaceId();
    IPv4Address source = pkt->getSourceAddress();
    IPv4Address group = pkt->getGroupAddress();
    AssertMetric receivedMetric = AssertMetric(pkt->getR(), pkt->getMetricPreference(), pkt->getMetric(), ctrlInfo->getSrcAddr());

    EV_INFO << "Received Assert(" << (source.isUnspecified() ? "*" : source.str()) << ", " << group << ")"
            << " packet on interface '" << ift->getInterfaceById(incomingInterfaceId)->getName() << "'.\n";

    emit(rcvdAssertPkSignal, pkt);

    if (!source.isUnspecified() && !receivedMetric.rptBit) {
        Route *routeSG = findRouteSG(source, group);
        if (routeSG) {
            PimsmInterface *incomingInterface = routeSG->upstreamInterface->getInterfaceId() == incomingInterfaceId ?
                static_cast<PimsmInterface *>(routeSG->upstreamInterface) :
                static_cast<PimsmInterface *>(routeSG->getDownstreamInterfaceByInterfaceId(incomingInterfaceId));
            ASSERT(incomingInterface);

            Interface::AssertState stateBefore = incomingInterface->assertState;
            processAssertSG(incomingInterface, receivedMetric);

            if (stateBefore != Interface::NO_ASSERT_INFO || incomingInterface->assertState != Interface::NO_ASSERT_INFO) {
                // processed by SG
                delete pkt;
                return;
            }
        }
    }

    // process (*,G) asserts and (S,G) asserts for which there is no assert state in (S,G) routes
    Route *routeG = findRouteG(group);
    if (routeG) {
        PimsmInterface *incomingInterface = routeG->upstreamInterface->getInterfaceId() == incomingInterfaceId ?
            static_cast<PimsmInterface *>(routeG->upstreamInterface) :
            static_cast<PimsmInterface *>(routeG->getDownstreamInterfaceByInterfaceId(incomingInterfaceId));
        ASSERT(incomingInterface);
        processAssertG(incomingInterface, receivedMetric);
    }

    delete pkt;
}

void PIMSM::processAssertSG(PimsmInterface *interface, const AssertMetric& receivedMetric)
{
    Route *routeSG = interface->route();
    AssertMetric myMetric = interface->couldAssert() ? // XXX check routeG metric too
        routeSG->metric.setAddress(interface->ie->ipv4Data()->getIPAddress()) :
        AssertMetric::PIM_INFINITE;

    // A "preferred assert" is one with a better metric than the current winner.
    bool isPreferredAssert = receivedMetric < interface->winnerMetric;

    // An "acceptable assert" is one that has a better metric than my_assert_metric(S,G,I).
    // An assert is never considered acceptable if its metric is infinite.
    bool isAcceptableAssert = receivedMetric < myMetric;

    // An "inferior assert" is one with a worse metric than my_assert_metric(S,G,I).
    // An assert is never considered inferior if my_assert_metric(S,G,I) is infinite.
    bool isInferiorAssert = myMetric < receivedMetric;

    if (interface->assertState == Interface::NO_ASSERT_INFO) {
        if (isInferiorAssert && !receivedMetric.rptBit && interface->couldAssert()) {
            // An assert is received for (S,G) with the RPT bit cleared that
            // is inferior to our own assert metric.  The RPT bit cleared
            // indicates that the sender of the assert had (S,G) forwarding
            // state on this interface.  If the assert is inferior to our
            // metric, then we must also have (S,G) forwarding state (i.e.,
            // CouldAssert(S,G,I)==TRUE) as (S,G) asserts beat (*,G) asserts,
            // and so we should be the assert winner.  We transition to the
            // "I am Assert Winner" state and perform Actions A1 (below).
            interface->assertState = Interface::I_WON_ASSERT;
            interface->winnerMetric = myMetric;
            sendPIMAssert(routeSG->source, routeSG->group, myMetric, interface->ie, false);
            interface->startAssertTimer(assertTime - assertOverrideInterval);
        }
        else if (receivedMetric.rptBit && interface->couldAssert()) {
            // An assert is received for (S,G) on I with the RPT bit set
            // (it's a (*,G) assert).  CouldAssert(S,G,I) is TRUE only if we
            // have (S,G) forwarding state on this interface, so we should be
            // the assert winner.  We transition to the "I am Assert Winner"
            // state and perform Actions A1 (below).
            interface->assertState = Interface::I_WON_ASSERT;
            interface->winnerMetric = myMetric;
            sendPIMAssert(routeSG->source, routeSG->group, myMetric, interface->ie, false);
            interface->startAssertTimer(assertTime - assertOverrideInterval);
        }
        else if (isAcceptableAssert && !receivedMetric.rptBit && interface->assertTrackingDesired()) {
            // We're interested in (S,G) Asserts, either because I is a
            // downstream interface for which we have (S,G) or (*,G)
            // forwarding state, or because I is the upstream interface for S
            // and we have (S,G) forwarding state.  The received assert has a
            // better metric than our own, so we do not win the Assert.  We
            // transition to "I am Assert Loser" and perform Actions A6
            // (below).
            interface->assertState = Interface::I_LOST_ASSERT;
            interface->winnerMetric = receivedMetric;
            interface->startAssertTimer(assertTime);
            // TODO If (I is RPF_interface(S)) AND (UpstreamJPState(S,G) == true)
            //      set SPTbit(S,G) to TRUE.
        }
    }
    else if (interface->assertState == Interface::I_WON_ASSERT) {
        if (isInferiorAssert) {
            // We receive an (S,G) assert or (*,G) assert mentioning S that
            // has a worse metric than our own.  Whoever sent the assert is
            // in error, and so we resend an (S,G) Assert and restart the
            // Assert Timer (Actions A3 below).
            sendPIMAssert(routeSG->source, routeSG->group, myMetric, interface->ie, false);
            restartTimer(interface->assertTimer, assertTime - assertOverrideInterval);
        }
        else if (isPreferredAssert) {
            // We receive an (S,G) assert that has a better metric than our
            // own.  We transition to "I am Assert Loser" state and perform
            // Actions A2 (below).  Note that this may affect the value of
            // JoinDesired(S,G) and PruneDesired(S,G,rpt), which could cause
            // transitions in the upstream (S,G) or (S,G,rpt) state machines.
            interface->assertState = Interface::I_LOST_ASSERT;
            interface->winnerMetric = receivedMetric;
            restartTimer(interface->assertTimer, assertTime);
        }
    }
    else if (interface->assertState == Interface::I_LOST_ASSERT) {
        if (isPreferredAssert) {
            // We receive an assert that is better than that of the current
            // assert winner.  We stay in Loser state and perform Actions A2
            // below.
            interface->winnerMetric = receivedMetric;
            restartTimer(interface->assertTimer, assertTime);
        }
        else if (isAcceptableAssert && !receivedMetric.rptBit && receivedMetric.address == interface->winnerMetric.address) {
            // We receive an assert from the current assert winner that is
            // better than our own metric for this (S,G) (although the metric
            // may be worse than the winner's previous metric).  We stay in
            // Loser state and perform Actions A2 below.
            interface->winnerMetric = receivedMetric;
            restartTimer(interface->assertTimer, assertTime);
        }
        else if (isInferiorAssert    /* or AssertCancel */ && receivedMetric.address == interface->winnerMetric.address) {
            // We receive an assert from the current assert winner that is
            // worse than our own metric for this group (typically, because
            // the winner's metric became worse or because it is an assert
            // cancel).  We transition to NoInfo state, deleting the (S,G)
            // assert information and allowing the normal PIM Join/Prune
            // mechanisms to operate.  Usually, we will eventually re-assert
            // and win when data packets from S have started flowing again.
            interface->deleteAssertInfo();
        }
    }
}

void PIMSM::processAssertG(PimsmInterface *interface, const AssertMetric& receivedMetric)
{
    Route *routeG = interface->route();
    AssertMetric myMetric = interface->couldAssert() ?
        routeG->metric.setAddress(interface->ie->ipv4Data()->getIPAddress()) :
        AssertMetric::PIM_INFINITE;

    // A "preferred assert" is one with a better metric than the current winner.
    bool isPreferredAssert = receivedMetric < interface->winnerMetric;

    // An "acceptable assert" is one that has a better metric than my_assert_metric(S,G,I).
    // An assert is never considered acceptable if its metric is infinite.
    bool isAcceptableAssert = receivedMetric < myMetric;

    // An "inferior assert" is one with a worse metric than my_assert_metric(S,G,I).
    // An assert is never considered inferior if my_assert_metric(S,G,I) is infinite.
    bool isInferiorAssert = myMetric < receivedMetric;

    if (interface->assertState == Interface::NO_ASSERT_INFO) {
        if (isInferiorAssert && receivedMetric.rptBit && interface->couldAssert()) {
            // An Inferior (*,G) assert is received for G on Interface I.  If
            // CouldAssert(*,G,I) is TRUE, then I is our downstream
            // interface, and we have (*,G) forwarding state on this
            // interface, so we should be the assert winner.  We transition
            // to the "I am Assert Winner" state and perform Actions:
            interface->assertState = Interface::I_WON_ASSERT;
            sendPIMAssert(IPv4Address::UNSPECIFIED_ADDRESS, routeG->group, myMetric, interface->ie, true);
            interface->startAssertTimer(assertTime - assertOverrideInterval);
            interface->winnerMetric = myMetric;
        }
        else if (isAcceptableAssert && receivedMetric.rptBit && interface->assertTrackingDesired()) {
            // We're interested in (*,G) Asserts, either because I is a
            // downstream interface for which we have (*,G) forwarding state,
            // or because I is the upstream interface for RP(G) and we have
            // (*,G) forwarding state.  We get a (*,G) Assert that has a
            // better metric than our own, so we do not win the Assert.  We
            // transition to "I am Assert Loser" and perform Actions:
            interface->assertState = Interface::I_LOST_ASSERT;
            interface->winnerMetric = receivedMetric;
            interface->startAssertTimer(assertTime);
        }
    }
    else if (interface->assertState == Interface::I_WON_ASSERT) {
        if (isInferiorAssert) {
            // We receive a (*,G) assert that has a worse metric than our
            // own.  Whoever sent the assert has lost, and so we resend a
            // (*,G) Assert and restart the Assert Timer (Actions A3 below).
            sendPIMAssert(IPv4Address::UNSPECIFIED_ADDRESS, routeG->group, myMetric, interface->ie, true);
            restartTimer(interface->assertTimer, assertTime - assertOverrideInterval);
        }
        else if (isPreferredAssert) {
            // We receive a (*,G) assert that has a better metric than our
            // own.  We transition to "I am Assert Loser" state and perform
            // Actions A2 (below).
            interface->assertState = Interface::I_LOST_ASSERT;
            interface->winnerMetric = receivedMetric;
            restartTimer(interface->assertTimer, assertTime);
        }
    }
    else if (interface->assertState == Interface::I_LOST_ASSERT) {
        if (isPreferredAssert && receivedMetric.rptBit) {
            // We receive a (*,G) assert that is better than that of the
            // current assert winner.  We stay in Loser state and perform
            // Actions A2 below.
            interface->winnerMetric = receivedMetric;
            restartTimer(interface->assertTimer, assertTime);
        }
        else if (isAcceptableAssert && receivedMetric.address == interface->winnerMetric.address && receivedMetric.rptBit) {
            // We receive a (*,G) assert from the current assert winner that
            // is better than our own metric for this group (although the
            // metric may be worse than the winner's previous metric).  We
            // stay in Loser state and perform Actions A2 below.
            //interface->winnerAddress = ...;
            interface->winnerMetric = receivedMetric;
            restartTimer(interface->assertTimer, assertTime);
        }
        else if (isInferiorAssert    /*or AssertCancel*/ && receivedMetric.address == interface->winnerMetric.address) {
            // We receive an assert from the current assert winner that is
            // worse than our own metric for this group (typically because
            // the winner's metric became worse or is now an assert cancel).
            // We transition to NoInfo state, delete this (*,G) assert state
            // (Actions A5), and allow the normal PIM Join/Prune mechanisms
            // to operate.  Usually, we will eventually re-assert and win
            // when data packets for G have started flowing again.
            interface->deleteAssertInfo();
        }
    }
}

//=============================================================================
//                   Processing Timers
//=============================================================================

void PIMSM::processKeepAliveTimer(cMessage *timer)
{
    EV << "pimSM::processKeepAliveTimer: route will be deleted" << endl;
    Route *route = static_cast<Route *>(timer->getContextPointer());
    ASSERT(route->type == SG);
    ASSERT(timer == route->keepAliveTimer);

    delete timer;
    route->keepAliveTimer = nullptr;
    deleteMulticastRoute(route);
}

void PIMSM::processRegisterStopTimer(cMessage *timer)
{
    EV << "pimSM::processRegisterStopTimer: " << endl;
    Route *routeSG = static_cast<Route *>(timer->getContextPointer());
    ASSERT(timer == routeSG->registerStopTimer);
    ASSERT(routeSG->type == SG);

    delete timer;
    routeSG->registerStopTimer = nullptr;

    if (routeSG->registerState == Route::RS_PRUNE) {
        routeSG->registerState = Route::RS_JOIN_PENDING;
        sendPIMRegisterNull(routeSG->source, routeSG->group);
        routeSG->startRegisterStopTimer(registerProbeTime);
    }
    else if (routeSG->registerState == Route::RS_JOIN_PENDING) {
        routeSG->registerState = Route::RS_JOIN;
    }
}

/**
 * The method is used to process PIM Expiry Timer. It is timer for (S,G) and (*,G).
 * When Expiry timer expires,route is removed from multicast routing table.
 */
void PIMSM::processExpiryTimer(cMessage *timer)
{
    EV << "pimSM::processExpiryTimer: " << endl;

    PimsmInterface *interface = static_cast<PimsmInterface *>(timer->getContextPointer());
    Route *route = interface->route();

    if (interface != route->upstreamInterface) {
        //
        // Downstream Join/Prune State Machine; event: ET expires
        //
        DownstreamInterface *downstream = check_and_cast<DownstreamInterface *>(interface);
        downstream->joinPruneState = DownstreamInterface::NO_INFO;
        cancelAndDeleteTimer(downstream->prunePendingTimer);
        downstream->expiryTimer = nullptr;
        delete timer;

        // upstream state machine
        if (route->isInheritedOlistNull()) {
            route->setFlags(Route::PRUNED);
            if (route->type == G && !IamRP(route->rpAddr))
                sendPIMPrune(route->group, route->rpAddr, route->upstreamInterface->rpfNeighbor(), G);
            else if (route->type == SG)
                sendPIMPrune(route->group, route->source, route->upstreamInterface->rpfNeighbor(), SG);

            cancelAndDeleteTimer(route->joinTimer);
        }
    }
    if (route->upstreamInterface->expiryTimer && interface == route->upstreamInterface) {
        for (unsigned i = 0; i < route->downstreamInterfaces.size(); ) {
            if (route->downstreamInterfaces[i]->expiryTimer)
                route->removeDownstreamInterface(i);
            else
                i++;
        }
        if (IamRP(rpAddr) && route->type == G) {
            EV << "ET for (*,G) route on RP expires - go to stopped" << endl;
            cancelAndDeleteTimer(route->upstreamInterface->expiryTimer);
        }
        else
            deleteMulticastRoute(route);
    }
}

void PIMSM::processJoinTimer(cMessage *timer)
{
    EV << "pimSM::processJoinTimer:" << endl;

    Route *route = static_cast<Route *>(timer->getContextPointer());
    ASSERT(timer == route->joinTimer);
    IPv4Address joinAddr = route->type == G ? route->rpAddr : route->source;

    if (!route->isInheritedOlistNull()) {
        sendPIMJoin(route->group, joinAddr, route->upstreamInterface->nextHop, route->type);
        restartTimer(route->joinTimer, joinPrunePeriod);
    }
    else {
        delete timer;
        route->joinTimer = nullptr;
    }
}

/**
 * Prune Pending Timer is used for delaying of Prune message sending
 * (for possible overriding Join from another PIM neighbor)
 */
void PIMSM::processPrunePendingTimer(cMessage *timer)
{
    EV << "pimSM::processPrunePendingTimer:" << endl;
    DownstreamInterface *downstream = static_cast<DownstreamInterface *>(timer->getContextPointer());
    ASSERT(timer == downstream->prunePendingTimer);
    ASSERT(downstream->joinPruneState == DownstreamInterface::PRUNE_PENDING);
    Route *route = downstream->route();

    if (route->type == G || route->type == SG) {
        //
        // Downstream (*,G)/(S,G) Join/Prune State Machine; event: PPT expires
        //

        // go to NO_INFO state
        downstream->joinPruneState = DownstreamInterface::NO_INFO;
        cancelAndDeleteTimer(downstream->expiryTimer);
        downstream->prunePendingTimer = nullptr;
        delete timer;

        // optionally send PruneEcho message
        if (pimNbt->getNumNeighbors(downstream->ie->getInterfaceId()) > 1) {
            // A PruneEcho is simply a Prune message sent by the
            // upstream router on a LAN with its own address in the Upstream
            // Neighbor Address field.  Its purpose is to add additional
            // reliability so that if a Prune that should have been
            // overridden by another router is lost locally on the LAN, then
            // the PruneEcho may be received and cause the override to happen.
            IPv4Address pruneAddr = route->type == G ? route->rpAddr : route->source;
            IPv4Address upstreamNeighborField = downstream->ie->ipv4Data()->getIPAddress();
            sendPIMPrune(route->group, pruneAddr, upstreamNeighborField, route->type);
        }
    }
    else if (route->type == SGrpt) {
        //
        // Downstream (S,G,rpt) Join/Prune State Machine; event: PPT expires
        //

        // go to PRUNE state
        // TODO
    }

    // Now check upstream state transitions
    updateJoinDesired(route);
}

void PIMSM::processAssertTimer(cMessage *timer)
{
    PimsmInterface *interfaceData = static_cast<PimsmInterface *>(timer->getContextPointer());
    ASSERT(timer == interfaceData->assertTimer);
    ASSERT(interfaceData->assertState != Interface::NO_ASSERT_INFO);

    Route *route = interfaceData->route();
    if (route->type == SG || route->type == G) {
        //
        // (S,G) Assert State Machine; event: AT(S,G,I) expires OR
        // (*,G) Assert State Machine; event: AT(*,G,I) expires
        //
        EV_DETAIL << "AssertTimer(" << (route->type == G ? "*" : route->source.str()) << ", "
                  << route->group << ", " << interfaceData->ie->getName() << ") has expired.\n";

        if (interfaceData->assertState == Interface::I_WON_ASSERT) {
            // The (S,G) or (*,G) Assert Timer expires.  As we're in the Winner state,
            // we must still have (S,G) or (*,G) forwarding state that is actively
            // being kept alive.  We resend the (S,G) or (*,G) Assert and restart the
            // Assert Timer.  Note that the assert
            // winner's Assert Timer is engineered to expire shortly before
            // timers on assert losers; this prevents unnecessary thrashing
            // of the forwarder and periodic flooding of duplicate packets.
            sendPIMAssert(route->source, route->group, route->metric, interfaceData->ie, route->type == G);
            restartTimer(interfaceData->assertTimer, assertTime - assertOverrideInterval);
            return;
        }
        else if (interfaceData->assertState == Interface::I_LOST_ASSERT) {
            // The (S,G) or (*,G) Assert Timer expires.  We transition to NoInfo
            // state, deleting the (S,G) or (*,G) assert information.
            EV_DEBUG << "Going into NO_ASSERT_INFO state.\n";
            interfaceData->deleteAssertInfo();    // deleted timer
            return;
        }
    }

    delete timer;
}

/**
 * The method is used to restart ET. ET is used for outgoing interfaces
 * and whole route in router. After ET expires, outgoing interface is
 * removed or if there aren't any outgoing interface, route is removed
 * after ET expires.
 */
void PIMSM::restartExpiryTimer(Route *route, InterfaceEntry *originIntf, int holdTime)
{
    EV << "pimSM::restartExpiryTimer: next ET @ " << simTime() + holdTime << " for type: ";

    if (route) {
        // ET for route
        if (route->upstreamInterface && route->upstreamInterface->expiryTimer)
            restartTimer(route->upstreamInterface->expiryTimer, holdTime);

        // ET for outgoing interfaces
        for (unsigned i = 0; i < route->downstreamInterfaces.size(); i++) {    // if exist ET and for given interface
            DownstreamInterface *downstream = route->downstreamInterfaces[i];
            if (downstream->expiryTimer && (downstream->getInterfaceId() == originIntf->getInterfaceId())) {
                EV <<    /*timer->getStateType() << " , " <<*/ route->group << " , " << route->source << ", int: " << downstream->ie->getName() << endl;
                restartTimer(downstream->expiryTimer, holdTime);
                break;
            }
        }
    }
}

//=============================================================================
//                         Signal Handlers
//=============================================================================

void PIMSM::unroutableMulticastPacketArrived(IPv4Address source, IPv4Address group)
{
    IPv4Route *routeTowardSource = rt->findBestMatchingRoute(source);    // rt->getInterfaceForDestAddr(source);
    if (!routeTowardSource)
        return;

    PIMInterface *rpfInterface = pimIft->getInterfaceById(routeTowardSource->getInterface()->getInterfaceId());
    if (!rpfInterface || rpfInterface->getMode() != PIMInterface::SparseMode)
        return;

    InterfaceEntry *interfaceTowardRP = rt->getInterfaceForDestAddr(rpAddr);

    // RPF check and check if I am DR of the source
    if ((interfaceTowardRP != routeTowardSource->getInterface()) && routeTowardSource->getGateway().isUnspecified()) {
        EV_DETAIL << "New multicast source observed: source=" << source << ", group=" << group << ".\n";

        // create new (S,G) route
        Route *newRouteSG = addNewRouteSG(source, group, Route::PRUNED | Route::REGISTER | Route::SPT_BIT);
        newRouteSG->startKeepAliveTimer(keepAlivePeriod);
        newRouteSG->registerState = Route::RS_JOIN;

        // create new (*,G) route
        addNewRouteG(newRouteSG->group, Route::PRUNED | Route::REGISTER);
    }
}

void PIMSM::multicastReceiverRemoved(InterfaceEntry *ie, IPv4Address group)
{
    EV_DETAIL << "No more receiver for group " << group << " on interface '" << ie->getName() << "'.\n";

    Route *routeG = findRouteG(group);
    if (routeG) {
        DownstreamInterface *downstream = routeG->getDownstreamInterfaceByInterfaceId(ie->getInterfaceId());
        downstream->setLocalReceiverInclude(false);
        updateJoinDesired(routeG);
    }
}

void PIMSM::multicastReceiverAdded(InterfaceEntry *ie, IPv4Address group)
{
    EV_DETAIL << "Multicast receiver added for group " << group << " on interface '" << ie->getName() << "'.\n";

    Route *routeG = findRouteG(group);
    if (!routeG)
        routeG = addNewRouteG(group, Route::PRUNED);

    DownstreamInterface *downstream = routeG->getDownstreamInterfaceByInterfaceId(ie->getInterfaceId());
    downstream->setLocalReceiverInclude(true);

    updateJoinDesired(routeG);
}

/**
 * The method process notification about data which appears on RPF interface. It means that source
 * is still active. The result is resetting of Keep Alive Timer. Also if first data packet arrive to
 * last hop router in RPT, switchover to SPT has to be considered.
 */
void PIMSM::multicastPacketArrivedOnRpfInterface(Route *route)
{
    if (route->type == SG) {    // (S,G) route
        // set KeepAlive timer
        if (    /*DirectlyConnected(route->source) ||*/
            (!route->isFlagSet(Route::PRUNED) && !route->isInheritedOlistNull()))
        {
            EV_DETAIL << "Data arrived on RPF interface, restarting KAT(" << route->source << ", " << route->group << ") timer.\n";

            if (!route->keepAliveTimer)
                route->startKeepAliveTimer(keepAlivePeriod);
            else
                restartTimer(route->keepAliveTimer, keepAlivePeriod);
        }

        // Update SPT bit

        /* TODO
             void
             Update_SPTbit(S,G,iif) {
               if ( iif == RPF_interface(S)
                     AND JoinDesired(S,G) == TRUE
                     AND ( DirectlyConnected(S) == TRUE
                           OR RPF_interface(S) != RPF_interface(RP(G))
                           OR inherited_olist(S,G,rpt) == nullptr
                           OR ( ( RPF'(S,G) == RPF'(*,G) ) AND
                                ( RPF'(S,G) != nullptr ) )
                           OR ( I_Am_Assert_Loser(S,G,iif) ) {
                  Set SPTbit(S,G) to TRUE
               }
             }
         */
        route->setFlags(Route::SPT_BIT);
    }

    // check switch from RP tree to the SPT
    if (route->isFlagSet(Route::SPT_BIT)) {
        /*
             void
             CheckSwitchToSpt(S,G) {
               if ( ( pim_include(*,G) (-) pim_exclude(S,G)
                      (+) pim_include(S,G) != nullptr )
                    AND SwitchToSptDesired(S,G) ) {
         # Note: Restarting the KAT will result in the SPT switch
                      set KeepaliveTimer(S,G) to Keepalive_Period
               }
             }
         */
    }
}

void PIMSM::multicastPacketArrivedOnNonRpfInterface(Route *route, int interfaceId)
{
    if (route->type == G || route->type == SG) {
        //
        // (S,G) Assert State Machine; event: An (S,G) data packet arrives on interface I
        // OR
        // (*,G) Assert State Machine; event: A data packet destined for G arrives on interface I
        //
        DownstreamInterface *downstream = route->findDownstreamInterfaceByInterfaceId(interfaceId);
        if (downstream && downstream->couldAssert() && downstream->assertState == Interface::NO_ASSERT_INFO) {
            // An (S,G) data packet arrived on an downstream interface that
            // is in our (S,G) or (*,G) outgoing interface list.  We optimistically
            // assume that we will be the assert winner for this (S,G) or (*,G), and
            // so we transition to the "I am Assert Winner" state and perform
            // Actions A1 (below), which will initiate the assert negotiation
            // for (S,G) or (*,G).
            downstream->assertState = Interface::I_WON_ASSERT;
            downstream->winnerMetric = route->metric.setAddress(downstream->ie->ipv4Data()->getIPAddress());
            sendPIMAssert(route->source, route->group, downstream->winnerMetric, downstream->ie, route->type == G);
            downstream->startAssertTimer(assertTime - assertOverrideInterval);
        }
        else if (route->type == SG && (!downstream || downstream->assertState == Interface::NO_ASSERT_INFO)) {
            // When in NO_ASSERT_INFO state before and after consideration of the received message,
            // then call (*,G) assert processing.
            Route *routeG = findRouteG(route->group);
            if (routeG)
                multicastPacketArrivedOnNonRpfInterface(routeG, interfaceId);
        }
    }
}

void PIMSM::multicastPacketForwarded(IPv4Datagram *datagram)
{
    IPv4Address source = datagram->getSrcAddress();
    IPv4Address group = datagram->getDestAddress();

    Route *routeSG = findRouteSG(source, group);
    if (!routeSG || !routeSG->isFlagSet(Route::REGISTER) || !routeSG->isFlagSet(Route::PRUNED))
        return;

    // send Register message to RP

    InterfaceEntry *interfaceTowardRP = rt->getInterfaceForDestAddr(routeSG->rpAddr);

    if (routeSG->registerState == Route::RS_JOIN) {
        // refresh KAT timer
        if (routeSG->keepAliveTimer) {
            EV << " (S,G) KAT timer refresh" << endl;
            restartTimer(routeSG->keepAliveTimer, KAT);
        }

        sendPIMRegister(datagram, routeSG->rpAddr, interfaceTowardRP->getInterfaceId());
    }
}

//============================================================================
//                       Internal events
//============================================================================

/*
 * JoinDesired(*,G) -> FALSE/TRUE
 * JoinDesired(S,G) -> FALSE/TRUE
 */
void PIMSM::joinDesiredChanged(Route *route)
{
    if (route->type == G) {
        Route *routeG = route;

        if (routeG->isFlagSet(Route::PRUNED) && routeG->joinDesired()) {
            //
            // Upstream (*,G) State Machine; event: JoinDesired(S,G) -> TRUE
            //
            routeG->clearFlag(Route::PRUNED);
            if (routeG->upstreamInterface) {
                sendPIMJoin(routeG->group, routeG->rpAddr, routeG->upstreamInterface->rpfNeighbor(), G);
                routeG->startJoinTimer(joinPrunePeriod);
            }
        }
        else if (!routeG->isFlagSet(Route::PRUNED) && !routeG->joinDesired()) {
            //
            // Upstream (*,G) State Machine; event: JoinDesired(S,G) -> FALSE
            //
            routeG->setFlags(Route::PRUNED);
            cancelAndDeleteTimer(routeG->joinTimer);
            if (routeG->upstreamInterface)
                sendPIMPrune(routeG->group, routeG->rpAddr, routeG->upstreamInterface->rpfNeighbor(), G);
        }
    }
    else if (route->type == SG) {
        Route *routeSG = route;

        if (routeSG->isFlagSet(Route::PRUNED) && routeSG->joinDesired()) {
            //
            // Upstream (S,G) State Machine; event: JoinDesired(S,G) -> TRUE
            //
            routeSG->clearFlag(Route::PRUNED);
            if (!routeSG->isSourceDirectlyConnected()) {
                sendPIMJoin(routeSG->group, routeSG->source, routeSG->upstreamInterface->rpfNeighbor(), SG);
                routeSG->startJoinTimer(joinPrunePeriod);
            }
        }
        else if (!routeSG->isFlagSet(Route::PRUNED) && !route->joinDesired()) {
            //
            // Upstream (S,G) State Machine; event: JoinDesired(S,G) -> FALSE
            //

            // The upstream (S,G) state machine transitions to NotJoined
            // state.  Send Prune(S,G) to the appropriate upstream neighbor,
            // which is RPF'(S,G).  Cancel the Join Timer (JT), and set
            // SPTbit(S,G) to FALSE.
            routeSG->setFlags(Route::PRUNED);
            routeSG->clearFlag(Route::SPT_BIT);
            cancelAndDeleteTimer(routeSG->joinTimer);
            if (!routeSG->isSourceDirectlyConnected())
                sendPIMPrune(routeSG->group, routeSG->source, routeSG->upstreamInterface->rpfNeighbor(), SG);
        }
    }
}

void PIMSM::designatedRouterAddressHasChanged(InterfaceEntry *ie)
{
    // TODO
}

void PIMSM::iAmDRHasChanged(InterfaceEntry *ie, bool iAmDR)
{
    // TODO
}

//============================================================================
//                      Sending PIM packets
//============================================================================

void PIMSM::sendPIMJoin(IPv4Address group, IPv4Address source, IPv4Address upstreamNeighbor, RouteType routeType)
{
    EV_INFO << "Sending Join(S=" << (routeType == G ? "*" : source.str()) << ", G=" << group << ") to neighbor " << upstreamNeighbor << ".\n";

    PIMJoinPrune *msg = new PIMJoinPrune();
    msg->setType(JoinPrune);
    msg->setName("PIMJoin");
    msg->setUpstreamNeighborAddress(upstreamNeighbor);
    msg->setHoldTime(joinPruneHoldTime());

    msg->setJoinPruneGroupsArraySize(1);
    JoinPruneGroup& multGroup = msg->getJoinPruneGroups(0);
    multGroup.setGroupAddress(group);
    multGroup.setJoinedSourceAddressArraySize(1);
    EncodedAddress& encodedAddr = multGroup.getJoinedSourceAddress(0);
    encodedAddr.IPaddress = source;
    encodedAddr.S = true;
    encodedAddr.W = (routeType == G);
    encodedAddr.R = (routeType == G);

    msg->setByteLength(PIM_HEADER_LENGTH
            + ENCODED_UNICODE_ADDRESS_LENGTH
            + 4
            + ENCODED_GROUP_ADDRESS_LENGTH
            + 4
            + ENCODED_SOURCE_ADDRESS_LENGTH);

    emit(sentJoinPrunePkSignal, msg);

    InterfaceEntry *interfaceToRP = rt->getInterfaceForDestAddr(source);
    sendToIP(msg, IPv4Address::UNSPECIFIED_ADDRESS, ALL_PIM_ROUTERS_MCAST, interfaceToRP->getInterfaceId(), 1);
}

void PIMSM::sendPIMPrune(IPv4Address group, IPv4Address source, IPv4Address upstreamNeighbor, RouteType routeType)
{
    EV_INFO << "Sending Prune(S=" << (routeType == G ? "*" : source.str()) << ", G=" << group << ") to neighbor " << upstreamNeighbor << ".\n";

    PIMJoinPrune *msg = new PIMJoinPrune();
    msg->setType(JoinPrune);
    msg->setName("PIMPrune");
    msg->setUpstreamNeighborAddress(upstreamNeighbor);
    msg->setHoldTime(joinPruneHoldTime());

    msg->setJoinPruneGroupsArraySize(1);
    JoinPruneGroup& multGroup = msg->getJoinPruneGroups(0);
    multGroup.setGroupAddress(group);
    multGroup.setPrunedSourceAddressArraySize(1);
    EncodedAddress& encodedAddr = multGroup.getPrunedSourceAddress(0);
    encodedAddr.IPaddress = source;
    encodedAddr.S = true;
    encodedAddr.W = (routeType == G);
    encodedAddr.R = (routeType == G);

    msg->setByteLength(PIM_HEADER_LENGTH
            + ENCODED_UNICODE_ADDRESS_LENGTH
            + 4
            + ENCODED_GROUP_ADDRESS_LENGTH
            + 4
            + ENCODED_SOURCE_ADDRESS_LENGTH);

    emit(sentJoinPrunePkSignal, msg);

    InterfaceEntry *interfaceToRP = rt->getInterfaceForDestAddr(source);
    sendToIP(msg, IPv4Address::UNSPECIFIED_ADDRESS, ALL_PIM_ROUTERS_MCAST, interfaceToRP->getInterfaceId(), 1);
}

void PIMSM::sendPIMRegisterNull(IPv4Address multOrigin, IPv4Address multGroup)
{
    EV << "pimSM::sendPIMRegisterNull" << endl;

    // only if (S,G exist)
    //if (getRouteFor(multDest,multSource))
    if (findRouteG(multGroup)) {
        PIMRegister *msg = new PIMRegister();
        msg->setName("PIMRegister(Null)");
        msg->setType(Register);
        msg->setN(true);
        msg->setB(false);

        msg->setByteLength(PIM_HEADER_LENGTH + 4);

        // set encapsulated packet (IPv4 header only)
        IPv4Datagram *datagram = new IPv4Datagram();
        datagram->setDestAddress(multGroup);
        datagram->setSrcAddress(multOrigin);
        datagram->setTransportProtocol(IP_PROT_PIM);
        datagram->setByteLength(IP_HEADER_BYTES);
        msg->encapsulate(datagram);

        emit(sentRegisterPkSignal, msg);

        InterfaceEntry *interfaceToRP = rt->getInterfaceForDestAddr(rpAddr);
        sendToIP(msg, IPv4Address::UNSPECIFIED_ADDRESS, rpAddr, interfaceToRP->getInterfaceId(), MAX_TTL);
    }
}

void PIMSM::sendPIMRegister(IPv4Datagram *datagram, IPv4Address dest, int outInterfaceId)
{
    EV << "pimSM::sendPIMRegister - encapsulating data packet into Register packet and sending to RP" << endl;

    PIMRegister *msg = new PIMRegister("PIMRegister");
    msg->setType(Register);
    msg->setN(false);
    msg->setB(false);

    msg->setByteLength(PIM_HEADER_LENGTH + 4);

    IPv4Datagram *datagramCopy = datagram->dup();
    delete datagramCopy->removeControlInfo();
    msg->encapsulate(datagramCopy);

    emit(sentRegisterPkSignal, msg);

    sendToIP(msg, IPv4Address::UNSPECIFIED_ADDRESS, dest, outInterfaceId, MAX_TTL);
}

void PIMSM::sendPIMRegisterStop(IPv4Address source, IPv4Address dest, IPv4Address multGroup, IPv4Address multSource)
{
    EV << "pimSM::sendPIMRegisterStop" << endl;

    // create PIM Register datagram
    PIMRegisterStop *msg = new PIMRegisterStop();

    // set PIM packet
    msg->setName("PIMRegisterStop");
    msg->setType(RegisterStop);
    msg->setSourceAddress(multSource);
    msg->setGroupAddress(multGroup);

    msg->setByteLength(PIM_HEADER_LENGTH + ENCODED_GROUP_ADDRESS_LENGTH + ENCODED_UNICODE_ADDRESS_LENGTH);

    emit(sentRegisterStopPkSignal, msg);

    // set IP packet
    InterfaceEntry *interfaceToDR = rt->getInterfaceForDestAddr(dest);
    sendToIP(msg, source, dest, interfaceToDR->getInterfaceId(), MAX_TTL);
}

void PIMSM::sendPIMAssert(IPv4Address source, IPv4Address group, AssertMetric metric, InterfaceEntry *ie, bool rptBit)
{
    EV_INFO << "Sending Assert(S= " << source << ", G= " << group << ") message on interface '" << ie->getName() << "'\n";

    PIMAssert *pkt = new PIMAssert("PIMAssert");
    pkt->setGroupAddress(group);
    pkt->setSourceAddress(source);
    pkt->setR(rptBit);
    pkt->setMetricPreference(metric.preference);
    pkt->setMetric(metric.metric);

    pkt->setByteLength(PIM_HEADER_LENGTH
            + ENCODED_GROUP_ADDRESS_LENGTH
            + ENCODED_UNICODE_ADDRESS_LENGTH
            + 8);

    emit(sentAssertPkSignal, pkt);

    sendToIP(pkt, IPv4Address::UNSPECIFIED_ADDRESS, ALL_PIM_ROUTERS_MCAST, ie->getInterfaceId(), 1);
}

void PIMSM::sendToIP(PIMPacket *packet, IPv4Address srcAddr, IPv4Address destAddr, int outInterfaceId, short ttl)
{
    IPv4ControlInfo *ctrl = new IPv4ControlInfo();
    ctrl->setSrcAddr(srcAddr);
    ctrl->setDestAddr(destAddr);
    ctrl->setProtocol(IP_PROT_PIM);
    ctrl->setTimeToLive(ttl);
    ctrl->setInterfaceId(outInterfaceId);
    packet->setControlInfo(ctrl);
    send(packet, "ipOut");
}

/**
 * The method is used as abstraction for encapsulation multicast data to Register packet.
 * The method create message MultData with multicast source address and multicast group address
 * and send the message from RP to RPT.
 */
void PIMSM::forwardMulticastData(IPv4Datagram *datagram, int outInterfaceId)
{
    EV << "pimSM::forwardMulticastData" << endl;

    //
    // Note: we should inject the datagram somehow into the normal IPv4 forwarding path.
    //
    cPacket *data = datagram->decapsulate();

    // set control info
    IPv4ControlInfo *ctrl = new IPv4ControlInfo();
    ctrl->setDestAddr(datagram->getDestAddress());
    // XXX ctrl->setSrcAddr(datagram->getSrcAddress()); // FIXME IP won't accept if the source is non-local
    ctrl->setInterfaceId(outInterfaceId);
    ctrl->setTimeToLive(MAX_TTL - 2);    //one minus for source DR router and one for RP router // XXX specification???
    ctrl->setProtocol(datagram->getTransportProtocol());
    data->setControlInfo(ctrl);
    send(data, "ipOut");
}

//============================================================================
//                     Update Actions
//============================================================================
void PIMSM::updateJoinDesired(Route *route)
{
    bool oldValue = route->joinDesired();
    bool newValue = false;
    if (route->type == RP)
        newValue = !route->isImmediateOlistNull();
    if (route->type == G)
        newValue = !route->isImmediateOlistNull()    /* || JoinDesired(*,*,RP(G)) AND AssertWinner(*,G,RPF_interface(RP(G))) != nullptr */;
    else if (route->type == SG)
        newValue = !route->isImmediateOlistNull() || (route->keepAliveTimer && !route->isInheritedOlistNull());

    if (newValue != oldValue) {
        route->setFlag(Route::JOIN_DESIRED, newValue);
        joinDesiredChanged(route);

        if (route->type == RP) {
            // TODO
        }
        else if (route->type == G) {
            for (auto & elem : sgRoutes) {
                if (elem.second->gRoute == route)
                    updateJoinDesired(elem.second);
            }
        }
    }
}

//
// To be called when the set of neighbors changes,
// or when the dr_priority or address of a neighbor changes.
//
void PIMSM::updateDesignatedRouterAddress(InterfaceEntry *ie)
{
    int interfaceId = ie->getInterfaceId();
    int numNeighbors = pimNbt->getNumNeighbors(interfaceId);

    bool eachNeighborHasPriority = true;
    for (int i = 0; i < numNeighbors && eachNeighborHasPriority; i++)
        if (pimNbt->getNeighbor(interfaceId, i))
            eachNeighborHasPriority = false;


    IPv4Address drAddress = ie->ipv4Data()->getIPAddress();
    int drPriority = this->designatedRouterPriority;
    for (int i = 0; i < numNeighbors; i++) {
        PIMNeighbor *neighbor = pimNbt->getNeighbor(interfaceId, i);
        bool isBetter = eachNeighborHasPriority ?
            (neighbor->getDRPriority() > drPriority ||
             (neighbor->getDRPriority() == drPriority &&
              neighbor->getAddress() > drAddress)) :
            (neighbor->getAddress() > drAddress);
        if (isBetter) {
            drPriority = neighbor->getDRPriority();
            drAddress = neighbor->getAddress();
        }
    }

    PIMInterface *pimInterface = pimIft->getInterfaceById(interfaceId);
    ASSERT(pimInterface);
    IPv4Address oldDRAddress = pimInterface->getDRAddress();
    if (drAddress != oldDRAddress) {
        pimInterface->setDRAddress(drAddress);
        designatedRouterAddressHasChanged(ie);

        IPv4Address myAddress = ie->ipv4Data()->getIPAddress();
        bool iWasDR = oldDRAddress.isUnspecified() || oldDRAddress == myAddress;
        bool iAmDR = drAddress == myAddress;
        if (iWasDR != iAmDR)
            iAmDRHasChanged(ie, iAmDR);
    }
}

void PIMSM::updateCouldAssert(DownstreamInterface *downstream)
{
    // TODO

    // CouldAssert(S,G,I) =
    // SPTbit(S,G)==TRUE
    // AND (RPF_interface(S) != I)
    // AND (I in ( ( joins(*,*,RP(G)) (+) joins(*,G) (-) prunes(S,G,rpt) )
    //            (+) ( pim_include(*,G) (-) pim_exclude(S,G) )
    //            (-) lost_assert(*,G)
    //            (+) joins(S,G) (+) pim_include(S,G) ) )

    // CouldAssert(*,G,I) =
    // ( I in ( joins(*,*,RP(G)) (+) joins(*,G)
    //          (+) pim_include(*,G)) )
    // AND (RPF_interface(RP(G)) != I)
}

void PIMSM::updateAssertTrackingDesired(PimsmInterface *interface)
{
    // TODO

    // AssertTrackingDesired(S,G,I) =
    // (I in ( ( joins(*,*,RP(G)) (+) joins(*,G) (-) prunes(S,G,rpt) )
    //         (+) ( pim_include(*,G) (-) pim_exclude(S,G) )
    //         (-) lost_assert(*,G)
    //         (+) joins(S,G) ) )
    // OR (local_receiver_include(S,G,I) == TRUE
    //     AND (I_am_DR(I) OR (AssertWinner(S,G,I) == me)))
    // OR ((RPF_interface(S) == I) AND (JoinDesired(S,G) == TRUE))
    // OR ((RPF_interface(RP(G)) == I) AND (JoinDesired(*,G) == TRUE)
    //     AND (SPTbit(S,G) == FALSE))

    // AssertTrackingDesired(*,G,I) =
    //   CouldAssert(*,G,I)
    //   OR (local_receiver_include(*,G,I)==TRUE
    //       AND (I_am_DR(I) OR AssertWinner(*,G,I) == me))
    //   OR (RPF_interface(RP(G)) == I AND RPTJoinDesired(G))
}

//============================================================================
//                                Helpers
//============================================================================

bool PIMSM::IamDR(InterfaceEntry *ie)
{
    PIMInterface *pimInterface = pimIft->getInterfaceById(ie->getInterfaceId());
    ASSERT(pimInterface);
    IPv4Address drAddress = pimInterface->getDRAddress();
    return drAddress.isUnspecified() || drAddress == ie->ipv4Data()->getIPAddress();
}

PIMInterface *PIMSM::getIncomingInterface(IPv4Datagram *datagram)
{
    cGate *g = datagram->getArrivalGate();
    if (g) {
        InterfaceEntry *ie = ift->getInterfaceByNetworkLayerGateIndex(g->getIndex());
        if (ie)
            return pimIft->getInterfaceById(ie->getInterfaceId());
    }
    return nullptr;
}

bool PIMSM::deleteMulticastRoute(Route *route)
{
    if (removeRoute(route)) {
        // remove route from IPv4 routing table
        IPv4MulticastRoute *ipv4Route = findIPv4Route(route->source, route->group);
        if (ipv4Route)
            rt->deleteMulticastRoute(ipv4Route);

        // unlink
        if (route->type == G) {
            for (auto & elem : sgRoutes)
                if (elem.second->gRoute == route)
                    elem.second->gRoute = nullptr;

        }

        delete route;
        return true;
    }
    return false;
}

void PIMSM::clearRoutes()
{
    // delete IPv4 routes
    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < rt->getNumMulticastRoutes(); i++) {
            IPv4MulticastRoute *ipv4Route = rt->getMulticastRoute(i);
            if (ipv4Route->getSource() == this) {
                rt->deleteMulticastRoute(ipv4Route);
                changed = true;
                break;
            }
        }
    }

    // clear local tables
    for (auto & elem : gRoutes)
        delete elem.second;
    gRoutes.clear();

    for (auto & elem : sgRoutes)
        delete elem.second;
    sgRoutes.clear();
}

PIMSM::Route *PIMSM::addNewRouteG(IPv4Address group, int flags)
{
    Route *newRouteG = new Route(this, G, IPv4Address::UNSPECIFIED_ADDRESS, group);
    newRouteG->setFlags(flags);
    newRouteG->rpAddr = rpAddr;

    // set upstream interface toward RP and set metric
    if (!IamRP(rpAddr)) {
        IPv4Route *routeToRP = rt->findBestMatchingRoute(rpAddr);
        if (routeToRP) {
            InterfaceEntry *ieTowardRP = routeToRP->getInterface();
            IPv4Address rpfNeighbor = routeToRP->getGateway();
            if (!pimNbt->findNeighbor(ieTowardRP->getInterfaceId(), rpfNeighbor) &&
                pimNbt->getNumNeighbors(ieTowardRP->getInterfaceId()) > 0)
            {
                PIMNeighbor *neighbor = pimNbt->getNeighbor(ieTowardRP->getInterfaceId(), 0);
                if (neighbor)
                    rpfNeighbor = neighbor->getAddress();
            }
            newRouteG->upstreamInterface = new UpstreamInterface(newRouteG, ieTowardRP, rpfNeighbor);
            newRouteG->metric = AssertMetric(true, routeToRP->getAdminDist(), routeToRP->getMetric());
        }
    }

    // add downstream interfaces
    for (int i = 0; i < pimIft->getNumInterfaces(); i++) {
        PIMInterface *pimInterface = pimIft->getInterface(i);
        if (pimInterface->getMode() == PIMInterface::SparseMode &&
            (!newRouteG->upstreamInterface || pimInterface->getInterfacePtr() != newRouteG->upstreamInterface->ie))
        {
            DownstreamInterface *downstream = new DownstreamInterface(newRouteG, pimInterface->getInterfacePtr(), DownstreamInterface::NO_INFO);
            newRouteG->addDownstreamInterface(downstream);
        }
    }

    SourceAndGroup sg(IPv4Address::UNSPECIFIED_ADDRESS, group);
    gRoutes[sg] = newRouteG;
    rt->addMulticastRoute(createIPv4Route(newRouteG));

    // set (*,G) route in (S,G) and (S,G,rpt) routes
    for (auto & elem : sgRoutes) {
        if (elem.first.group == group) {
            Route *sgRoute = elem.second;
            sgRoute->gRoute = newRouteG;
        }
    }

    return newRouteG;
}

PIMSM::Route *PIMSM::addNewRouteSG(IPv4Address source, IPv4Address group, int flags)
{
    ASSERT(!source.isUnspecified());
    ASSERT(group.isMulticast() && !group.isLinkLocalMulticast());

    Route *newRouteSG = new Route(this, SG, source, group);
    newRouteSG->setFlags(flags);
    newRouteSG->rpAddr = rpAddr;

    // set upstream interface toward source and set metric
    IPv4Route *routeToSource = rt->findBestMatchingRoute(source);
    if (routeToSource) {
        InterfaceEntry *ieTowardSource = routeToSource->getInterface();
        IPv4Address rpfNeighbor = routeToSource->getGateway();

        if (rpfNeighbor.isUnspecified())
            newRouteSG->setFlag(Route::SOURCE_DIRECTLY_CONNECTED, true);
        else {
            if (!pimNbt->findNeighbor(ieTowardSource->getInterfaceId(), rpfNeighbor) &&
                pimNbt->getNumNeighbors(ieTowardSource->getInterfaceId()) > 0)
            {
                PIMNeighbor *neighbor = pimNbt->getNeighbor(ieTowardSource->getInterfaceId(), 0);
                if (neighbor)
                    rpfNeighbor = neighbor->getAddress();
            }
        }
        newRouteSG->upstreamInterface = new UpstreamInterface(newRouteSG, ieTowardSource, rpfNeighbor);
        newRouteSG->metric = AssertMetric(false, routeToSource->getAdminDist(), routeToSource->getMetric());
    }

    // add downstream interfaces
    for (int i = 0; i < pimIft->getNumInterfaces(); i++) {
        PIMInterface *pimInterface = pimIft->getInterface(i);
        if (pimInterface->getMode() == PIMInterface::SparseMode && pimInterface->getInterfacePtr() != newRouteSG->upstreamInterface->ie) {
            DownstreamInterface *downstream = new DownstreamInterface(newRouteSG, pimInterface->getInterfacePtr(), DownstreamInterface::NO_INFO);
            newRouteSG->addDownstreamInterface(downstream);
        }
    }

    SourceAndGroup sg(source, group);
    sgRoutes[sg] = newRouteSG;
    rt->addMulticastRoute(createIPv4Route(newRouteSG));

    // set (*,G) route if exists
    newRouteSG->gRoute = findRouteG(group);

    // set (S,G,rpt) route if exists
    Route *sgrptRoute = nullptr;    // TODO
    newRouteSG->sgrptRoute = sgrptRoute;

    // set (S,G) route in (S,G,rpt) route
    if (sgrptRoute) {
//        sgrptRoute->sgRoute = newRouteSG;
    }

    return newRouteSG;
}

IPv4MulticastRoute *PIMSM::createIPv4Route(Route *route)
{
    IPv4MulticastRoute *newRoute = new IPv4MulticastRoute();
    newRoute->setOrigin(route->source);
    newRoute->setOriginNetmask(route->source.isUnspecified() ? IPv4Address::UNSPECIFIED_ADDRESS : IPv4Address::ALLONES_ADDRESS);
    newRoute->setMulticastGroup(route->group);
    newRoute->setSourceType(IMulticastRoute::PIM_SM);
    newRoute->setSource(this);
    if (route->upstreamInterface)
        newRoute->setInInterface(new IMulticastRoute::InInterface(route->upstreamInterface->ie));
    unsigned int numOutInterfaces = route->downstreamInterfaces.size();
    for (unsigned int i = 0; i < numOutInterfaces; ++i) {
        DownstreamInterface *downstream = route->downstreamInterfaces[i];
        newRoute->addOutInterface(new PIMSMOutInterface(downstream));
    }
    return newRoute;
}

bool PIMSM::removeRoute(Route *route)
{
    SourceAndGroup sg(route->source, route->group);
    if (route->type == G)
        return gRoutes.erase(sg);
    else
        return sgRoutes.erase(sg);
}

PIMSM::Route *PIMSM::findRouteG(IPv4Address group)
{
    SourceAndGroup sg(IPv4Address::UNSPECIFIED_ADDRESS, group);
    auto it = gRoutes.find(sg);
    return it != gRoutes.end() ? it->second : nullptr;
}

PIMSM::Route *PIMSM::findRouteSG(IPv4Address source, IPv4Address group)
{
    ASSERT(!source.isUnspecified());
    SourceAndGroup sg(source, group);
    auto it = sgRoutes.find(sg);
    return it != sgRoutes.end() ? it->second : nullptr;
}

IPv4MulticastRoute *PIMSM::findIPv4Route(IPv4Address source, IPv4Address group)
{
    unsigned int numMulticastRoutes = rt->getNumMulticastRoutes();
    for (unsigned int i = 0; i < numMulticastRoutes; ++i) {
        IPv4MulticastRoute *ipv4Route = rt->getMulticastRoute(i);
        if (ipv4Route->getSource() == this && ipv4Route->getOrigin() == source && ipv4Route->getMulticastGroup() == group)
            return ipv4Route;
    }
    return nullptr;
}

void PIMSM::cancelAndDeleteTimer(cMessage *& timer)
{
    cancelAndDelete(timer);
    timer = nullptr;
}

void PIMSM::restartTimer(cMessage *timer, double interval)
{
    cancelEvent(timer);
    scheduleAt(simTime() + interval, timer);
}

PIMSM::Route::Route(PIMSM *owner, RouteType type, IPv4Address origin, IPv4Address group)
    : RouteEntry(owner, origin, group), type(type), rpAddr(IPv4Address::UNSPECIFIED_ADDRESS),
    rpRoute(nullptr), gRoute(nullptr), sgrptRoute(nullptr),
    sequencenumber(0), keepAliveTimer(nullptr), joinTimer(nullptr), registerState(RS_NO_INFO), registerStopTimer(nullptr),
    upstreamInterface(nullptr)
{
}

PIMSM::Route::~Route()
{
    owner->cancelAndDelete(keepAliveTimer);
    owner->cancelAndDelete(registerStopTimer);
    owner->cancelAndDelete(joinTimer);
    delete upstreamInterface;
    for (auto & elem : downstreamInterfaces)
        delete elem;
}

void PIMSM::Route::startKeepAliveTimer(double keepAlivePeriod)
{
    ASSERT(this->type == SG);
    keepAliveTimer = new cMessage("PIMKeepAliveTimer", KeepAliveTimer);
    keepAliveTimer->setContextPointer(this);
    owner->scheduleAt(simTime() + keepAlivePeriod, keepAliveTimer);
}

void PIMSM::Route::startRegisterStopTimer(double interval)
{
    registerStopTimer = new cMessage("PIMRegisterStopTimer", RegisterStopTimer);
    registerStopTimer->setContextPointer(this);
    owner->scheduleAt(simTime() + interval, registerStopTimer);
}

void PIMSM::Route::startJoinTimer(double joinPrunePeriod)
{
    joinTimer = new cMessage("PIMJoinTimer", JoinTimer);
    joinTimer->setContextPointer(this);
    owner->scheduleAt(simTime() + joinPrunePeriod, joinTimer);
}

PIMSM::DownstreamInterface *PIMSM::Route::findDownstreamInterfaceByInterfaceId(int interfaceId)
{
    for (auto & elem : downstreamInterfaces) {
        DownstreamInterface *downstream = elem;
        if (downstream->getInterfaceId() == interfaceId)
            return downstream;
    }
    return nullptr;
}

PIMSM::DownstreamInterface *PIMSM::Route::getDownstreamInterfaceByInterfaceId(int interfaceId)
{
    DownstreamInterface *downstream = findDownstreamInterfaceByInterfaceId(interfaceId);
    if (!downstream)
        throw cRuntimeError("getDownstreamInterfaceByInterfaceId(): interface %d not found", interfaceId);
    return downstream;
}

int PIMSM::Route::findDownstreamInterface(InterfaceEntry *ie)
{
    for (unsigned int i = 0; i < downstreamInterfaces.size(); i++) {
        DownstreamInterface *downstream = downstreamInterfaces[i];
        if (downstream->ie == ie)
            return i;
    }
    return -1;
}

bool PIMSM::Route::isImmediateOlistNull()
{
    for (auto & elem : downstreamInterfaces)
        if (elem->isInImmediateOlist())
            return false;

    return true;
}

bool PIMSM::Route::isInheritedOlistNull()
{
    for (auto & elem : downstreamInterfaces)
        if (elem->isInInheritedOlist())
            return false;

    return true;
}

void PIMSM::Route::addDownstreamInterface(DownstreamInterface *outInterface)
{
    ASSERT(outInterface);

    auto it = downstreamInterfaces.begin();
    for ( ; it != downstreamInterfaces.end(); ++it) {
        if ((*it)->ie == outInterface->ie)
            break;
    }

    if (it != downstreamInterfaces.end()) {
        delete *it;
        *it = outInterface;
    }
    else {
        downstreamInterfaces.push_back(outInterface);
    }
}

void PIMSM::Route::removeDownstreamInterface(unsigned int i)
{
    // remove corresponding out interface from the IPv4 route,
    // because it refers to the downstream interface to be deleted
    IPv4MulticastRoute *ipv4Route = pimsm()->findIPv4Route(source, group);
    ipv4Route->removeOutInterface(i);

    DownstreamInterface *outInterface = downstreamInterfaces[i];
    downstreamInterfaces.erase(downstreamInterfaces.begin() + i);
    delete outInterface;
}

PIMSM::PimsmInterface::PimsmInterface(Route *owner, InterfaceEntry *ie)
    : Interface(owner, ie), expiryTimer(nullptr)
{
}

PIMSM::PimsmInterface::~PimsmInterface()
{
    pimsm()->cancelAndDelete(expiryTimer);
    pimsm()->cancelAndDelete(assertTimer);
}

void PIMSM::PimsmInterface::startExpiryTimer(double holdTime)
{
    expiryTimer = new cMessage("PIMExpiryTimer", ExpiryTimer);
    expiryTimer->setContextPointer(this);
    pimsm()->scheduleAt(simTime() + holdTime, expiryTimer);
}

PIMSM::DownstreamInterface::~DownstreamInterface()
{
    pimsm()->cancelAndDelete(prunePendingTimer);
}

bool PIMSM::DownstreamInterface::isInImmediateOlist() const
{
    // immediate_olist(*,*,RP) = joins(*,*,RP)
    // immediate_olist(*,G) = joins(*,G) (+) pim_include(*,G) (-) lost_assert(*,G)
    // immediate_olist(S,G) = joins(S,G) (+) pim_include(S,G) (-) lost_assert(S,G)
    switch (route()->type) {
        case RP:
            return joinPruneState != NO_INFO;

        case G:
            return assertState != I_LOST_ASSERT && (joinPruneState != NO_INFO || pimInclude());

        case SG:
            return assertState != I_LOST_ASSERT && (joinPruneState != NO_INFO || pimInclude());

        case SGrpt:
            ASSERT(false);
            break;
    }
    return false;
}

bool PIMSM::DownstreamInterface::isInInheritedOlist() const
{
    // inherited_olist(S,G,rpt) = ( joins(*,*,RP(G)) (+) joins(*,G) (-) prunes(S,G,rpt) )
    //                            (+) ( pim_include(*,G) (-) pim_exclude(S,G))
    //                            (-) ( lost_assert(*,G) (+) lost_assert(S,G,rpt) )
    // inherited_olist(S,G) = inherited_olist(S,G,rpt) (+)
    //                        joins(S,G) (+) pim_include(S,G) (-) lost_assert(S,G)
    Route *route = this->route();
    int interfaceId = ie->getInterfaceId();
    bool include = false;

    switch (route->type) {
        case RP:
            // joins(*,*,RP(G))
            include |= joinPruneState != NO_INFO;
            break;

        case G:    // inherited_olist(S,G,rpt) when there is not (S,G,rpt) state
            // joins(*,*,RP(G))
            if (route->rpRoute) {
                DownstreamInterface *downstream = route->rpRoute->findDownstreamInterfaceByInterfaceId(interfaceId);
                include |= downstream && downstream->joinPruneState != NO_INFO;
            }
            include |= joinPruneState != NO_INFO;
            include |= pimInclude();
            include &= assertState != I_LOST_ASSERT;
            break;

        case SGrpt:
            // joins(*,*,RP(G)
            if (route->rpRoute) {
                DownstreamInterface *downstream = route->rpRoute->findDownstreamInterfaceByInterfaceId(interfaceId);
                include |= downstream && downstream->joinPruneState != NO_INFO;
            }
            // (+) joins(*,G)
            if (route->gRoute) {
                DownstreamInterface *downstream = route->gRoute->findDownstreamInterfaceByInterfaceId(interfaceId);
                include |= downstream && downstream->joinPruneState != NO_INFO;
            }

            // TODO (-) prunes(S,G,prt)

            if (route->gRoute) {
                DownstreamInterface *downstream = route->gRoute->findDownstreamInterfaceByInterfaceId(interfaceId);
                // (+) ( pim_include(*,G) (-) pim_exclude(S,G))
                include |= (downstream && downstream->pimInclude()) && !pimExclude();
                // (-) lost_assert(*,G)
                include &= !downstream || downstream->assertState != I_LOST_ASSERT;
            }

            // (-) lost_assert(S,G)
            include &= assertState != I_LOST_ASSERT;
            break;

        case SG:
            // inherited_olist(S,G,rpt)
            if (route->sgrptRoute) {
                DownstreamInterface *downstream = route->sgrptRoute->findDownstreamInterfaceByInterfaceId(interfaceId);
                include |= downstream && downstream->isInInheritedOlist();
            }
            else if (route->gRoute) {
                DownstreamInterface *downstream = route->gRoute->findDownstreamInterfaceByInterfaceId(interfaceId);
                include |= downstream && downstream->isInInheritedOlist();
            }
            // (+) joins(S,G)
            include |= joinPruneState != NO_INFO;
            // (+) pim_include(S,G)
            include |= pimInclude();
            // (-) lost_assert(S,G)
            include &= assertState != I_LOST_ASSERT;
            break;
    }
    return include;
}

void PIMSM::DownstreamInterface::startPrunePendingTimer(double joinPruneOverrideInterval)
{
    ASSERT(!prunePendingTimer);
    prunePendingTimer = new cMessage("PIMPrunePendingTimer", PrunePendingTimer);
    prunePendingTimer->setContextPointer(this);
    pimsm()->scheduleAt(simTime() + joinPruneOverrideInterval, prunePendingTimer);
}
}    // namespace inet

