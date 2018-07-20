//
// Copyright (C) 2006 Andras Babos and Andras Varga
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

#include "inet/routing/ospfv2/messagehandler/LinkStateUpdateHandler.h"

#include "inet/routing/ospfv2/router/OspfArea.h"
#include "inet/routing/ospfv2/router/OspfCommon.h"
#include "inet/routing/ospfv2/neighbor/OspfNeighbor.h"
#include "inet/routing/ospfv2/router/OspfRouter.h"

namespace inet {

namespace ospf {

class LsaProcessingMarker
{
  private:
    unsigned int index;

  public:
    LsaProcessingMarker(unsigned int counter) : index(counter) { EV_INFO << "    --> Processing LSA(" << index << ")\n"; }
    ~LsaProcessingMarker() { EV_INFO << "    <-- LSA(" << index << ") processed.\n"; }
};

LinkStateUpdateHandler::LinkStateUpdateHandler(Router *containingRouter) :
    IMessageHandler(containingRouter)
{
}

/**
 * @see RFC2328 Section 13.
 */
void LinkStateUpdateHandler::processPacket(Packet *packet, Interface *intf, Neighbor *neighbor)
{
    router->getMessageHandler()->printEvent("Link State update packet received", intf, neighbor);

    const auto& lsUpdatePacket = packet->peekAtFront<OspfLinkStateUpdatePacket>();
    bool shouldRebuildRoutingTable = false;

    if (neighbor->getState() >= Neighbor::EXCHANGE_STATE) {
        AreaId areaID = lsUpdatePacket->getAreaID();
        Area *area = router->getAreaByID(areaID);
        LsaType currentType = ROUTERLSA_TYPE;
        unsigned int currentLSAIndex = 0;

        EV_INFO << "  Processing packet contents:\n";

        while (currentType <= AS_EXTERNAL_LSA_TYPE) {
            unsigned int lsaCount = 0;

            switch (currentType) {
                case ROUTERLSA_TYPE:
                    lsaCount = lsUpdatePacket->getRouterLSAsArraySize();
                    break;

                case NETWORKLSA_TYPE:
                    lsaCount = lsUpdatePacket->getNetworkLSAsArraySize();
                    break;

                case SUMMARYLSA_NETWORKS_TYPE:
                case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE:
                    lsaCount = lsUpdatePacket->getSummaryLSAsArraySize();
                    break;

                case AS_EXTERNAL_LSA_TYPE:
                    lsaCount = lsUpdatePacket->getAsExternalLSAsArraySize();
                    break;

                default:
                    throw cRuntimeError("Invalid currentType:%d", currentType);
            }

            for (unsigned int i = 0; i < lsaCount; i++) {
                const OspfLsa *currentLSA;

                switch (currentType) {
                    case ROUTERLSA_TYPE:
                        currentLSA = (&(lsUpdatePacket->getRouterLSAs(i)));
                        break;

                    case NETWORKLSA_TYPE:
                        currentLSA = (&(lsUpdatePacket->getNetworkLSAs(i)));
                        break;

                    case SUMMARYLSA_NETWORKS_TYPE:
                    case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE:
                        currentLSA = (&(lsUpdatePacket->getSummaryLSAs(i)));
                        break;

                    case AS_EXTERNAL_LSA_TYPE:
                        currentLSA = (&(lsUpdatePacket->getAsExternalLSAs(i)));
                        break;

                    default:
                        throw cRuntimeError("Invalid currentType:%d", currentType);
                }

                if (!validateLSChecksum(currentLSA)) {
                    continue;
                }

                LsaType lsaType = static_cast<LsaType>(currentLSA->getHeader().getLsType());
                if ((lsaType != ROUTERLSA_TYPE) &&
                    (lsaType != NETWORKLSA_TYPE) &&
                    (lsaType != SUMMARYLSA_NETWORKS_TYPE) &&
                    (lsaType != SUMMARYLSA_ASBOUNDARYROUTERS_TYPE) &&
                    (lsaType != AS_EXTERNAL_LSA_TYPE))
                {
                    continue;
                }

                LsaProcessingMarker marker(currentLSAIndex++);
                EV_DETAIL << "    " << currentLSA->getHeader() << "\n";

                //FIXME area maybe nullptr
                if ((lsaType == AS_EXTERNAL_LSA_TYPE) && !(area != nullptr && area->getExternalRoutingCapability())) {
                    continue;
                }
                LsaKeyType lsaKey;

                lsaKey.linkStateID = currentLSA->getHeader().getLinkStateID();
                lsaKey.advertisingRouter = currentLSA->getHeader().getAdvertisingRouter();

                OspfLsa *lsaInDatabase = router->findLSA(lsaType, lsaKey, areaID);
                unsigned short lsAge = currentLSA->getHeader().getLsAge();
                AcknowledgementFlags ackFlags;

                ackFlags.floodedBackOut = false;
                ackFlags.lsaIsNewer = false;
                ackFlags.lsaIsDuplicate = false;
                ackFlags.impliedAcknowledgement = false;
                ackFlags.lsaReachedMaxAge = (lsAge == MAX_AGE);
                ackFlags.noLSAInstanceInDatabase = (lsaInDatabase == nullptr);
                ackFlags.anyNeighborInExchangeOrLoadingState = router->hasAnyNeighborInStates(Neighbor::EXCHANGE_STATE | Neighbor::LOADING_STATE);

                if ((ackFlags.lsaReachedMaxAge) && (ackFlags.noLSAInstanceInDatabase) && (!ackFlags.anyNeighborInExchangeOrLoadingState)) {
                    if (intf->getType() == Interface::BROADCAST) {
                        if ((intf->getState() == Interface::DESIGNATED_ROUTER_STATE) ||
                            (intf->getState() == Interface::BACKUP_STATE) ||
                            (intf->getDesignatedRouter() == NULL_DESIGNATEDROUTERID))
                        {
                            intf->sendLsAcknowledgement(&(currentLSA->getHeader()), Ipv4Address::ALL_OSPF_ROUTERS_MCAST);
                        }
                        else {
                            intf->sendLsAcknowledgement(&(currentLSA->getHeader()), Ipv4Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST);
                        }
                    }
                    else {
                        if (intf->getType() == Interface::POINTTOPOINT) {
                            intf->sendLsAcknowledgement(&(currentLSA->getHeader()), Ipv4Address::ALL_OSPF_ROUTERS_MCAST);
                        }
                        else {
                            intf->sendLsAcknowledgement(&(currentLSA->getHeader()), neighbor->getAddress());
                        }
                    }
                    continue;
                }

                if (!ackFlags.noLSAInstanceInDatabase) {
                    // operator< and operator== on OSPFLSAHeaders determines which one is newer(less means older)
                    ackFlags.lsaIsNewer = (lsaInDatabase->getHeader() < currentLSA->getHeader());
                    ackFlags.lsaIsDuplicate = (operator==(lsaInDatabase->getHeader(), currentLSA->getHeader()));
                }
                if ((ackFlags.noLSAInstanceInDatabase) || (ackFlags.lsaIsNewer)) {
                    LsaTrackingInfo *info = (!ackFlags.noLSAInstanceInDatabase) ? dynamic_cast<LsaTrackingInfo *>(lsaInDatabase) : nullptr;
                    if ((!ackFlags.noLSAInstanceInDatabase) &&
                        (info != nullptr) &&
                        (info->getSource() == LsaTrackingInfo::FLOODED) &&
                        (info->getInstallTime() < MIN_LS_ARRIVAL))
                    {
                        continue;
                    }
                    ackFlags.floodedBackOut = router->floodLSA(currentLSA, areaID, intf, neighbor);
                    if (!ackFlags.noLSAInstanceInDatabase) {
                        LsaKeyType lsaKey;

                        lsaKey.linkStateID = lsaInDatabase->getHeader().getLinkStateID();
                        lsaKey.advertisingRouter = lsaInDatabase->getHeader().getAdvertisingRouter();

                        router->removeFromAllRetransmissionLists(lsaKey);
                    }
                    shouldRebuildRoutingTable |= router->installLSA(currentLSA, areaID);

                    // Add externalIPRoute in IPRoutingTable if this route is learned by BGP
                    if (currentType == AS_EXTERNAL_LSA_TYPE) {
                        const OspfAsExternalLsa *externalLSA = &(lsUpdatePacket->getAsExternalLSAs(0));
                        if (externalLSA->getContents().getExternalRouteTag() == OSPF_EXTERNAL_ROUTES_LEARNED_BY_BGP) {
                            Ipv4Address externalAddr = currentLSA->getHeader().getLinkStateID();
                            int ifName = intf->getIfIndex();
                            router->addExternalRouteInIPTable(externalAddr, externalLSA->getContents(), ifName);
                        }
                    }

                    EV_INFO << "    (update installed)\n";

                    acknowledgeLSA(currentLSA->getHeader(), intf, ackFlags, lsUpdatePacket->getRouterID());
                    if ((currentLSA->getHeader().getAdvertisingRouter() == router->getRouterID()) ||
                        ((lsaType == NETWORKLSA_TYPE) &&
                         (router->isLocalAddress(currentLSA->getHeader().getLinkStateID()))))
                    {
                        if (ackFlags.noLSAInstanceInDatabase) {
                            auto lsaCopy = currentLSA->dup();
                            lsaCopy->getHeaderForUpdate().setLsAge(MAX_AGE);
                            router->floodLSA(lsaCopy, areaID);
                        }
                        else {
                            if (ackFlags.lsaIsNewer) {
                                long sequenceNumber = currentLSA->getHeader().getLsSequenceNumber();
                                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                                    lsaInDatabase->getHeaderForUpdate().setLsAge(MAX_AGE);
                                    router->floodLSA(lsaInDatabase, areaID);
                                }
                                else {
                                    lsaInDatabase->getHeaderForUpdate().setLsSequenceNumber(sequenceNumber + 1);
                                    router->floodLSA(lsaInDatabase, areaID);
                                }
                            }
                        }
                    }
                    continue;
                }
                if (neighbor->isLSAOnRequestList(lsaKey)) {
                    neighbor->processEvent(Neighbor::BAD_LINK_STATE_REQUEST);
                    break;
                }
                if (ackFlags.lsaIsDuplicate) {
                    if (neighbor->isLinkStateRequestListEmpty(lsaKey)) {
                        neighbor->removeFromRetransmissionList(lsaKey);
                        ackFlags.impliedAcknowledgement = true;
                    }
                    acknowledgeLSA(currentLSA->getHeader(), intf, ackFlags, lsUpdatePacket->getRouterID());
                    continue;
                }
                if ((lsaInDatabase->getHeader().getLsAge() == MAX_AGE) &&
                    (lsaInDatabase->getHeader().getLsSequenceNumber() == MAX_SEQUENCE_NUMBER))
                {
                    continue;
                }
                if (!neighbor->isOnTransmittedLSAList(lsaKey)) {
                    Packet *updatePacket = intf->createUpdatePacket(lsaInDatabase);
                    if (updatePacket != nullptr) {
                        int ttl = (intf->getType() == Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;

                        if (intf->getType() == Interface::BROADCAST) {
                            if ((intf->getState() == Interface::DESIGNATED_ROUTER_STATE) ||
                                (intf->getState() == Interface::BACKUP_STATE) ||
                                (intf->getDesignatedRouter() == NULL_DESIGNATEDROUTERID))
                            {
                                router->getMessageHandler()->sendPacket(updatePacket, Ipv4Address::ALL_OSPF_ROUTERS_MCAST, intf->getIfIndex(), ttl);
                            }
                            else {
                                router->getMessageHandler()->sendPacket(updatePacket, Ipv4Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST, intf->getIfIndex(), ttl);
                            }
                        }
                        else {
                            if (intf->getType() == Interface::POINTTOPOINT) {
                                router->getMessageHandler()->sendPacket(updatePacket, Ipv4Address::ALL_OSPF_ROUTERS_MCAST, intf->getIfIndex(), ttl);
                            }
                            else {
                                router->getMessageHandler()->sendPacket(updatePacket, neighbor->getAddress(), intf->getIfIndex(), ttl);
                            }
                        }
                    }
                }
            }
            currentType = static_cast<LsaType>(currentType + 1);
            if (currentType == SUMMARYLSA_NETWORKS_TYPE) {
                currentType = static_cast<LsaType>(currentType + 1);
            }
        }
    }

    if (shouldRebuildRoutingTable) {
        router->rebuildRoutingTable();
    }
}

void LinkStateUpdateHandler::acknowledgeLSA(const OspfLsaHeader& lsaHeader,
        Interface *intf,
        LinkStateUpdateHandler::AcknowledgementFlags acknowledgementFlags,
        RouterId lsaSource)
{
    bool sendDirectAcknowledgment = false;

    if (!acknowledgementFlags.floodedBackOut) {
        if (intf->getState() == Interface::BACKUP_STATE) {
            if ((acknowledgementFlags.lsaIsNewer && (lsaSource == intf->getDesignatedRouter().routerID)) ||
                (acknowledgementFlags.lsaIsDuplicate && acknowledgementFlags.impliedAcknowledgement))
            {
                intf->addDelayedAcknowledgement(lsaHeader);
            }
            else {
                if ((acknowledgementFlags.lsaIsDuplicate && !acknowledgementFlags.impliedAcknowledgement) ||
                    (acknowledgementFlags.lsaReachedMaxAge &&
                     acknowledgementFlags.noLSAInstanceInDatabase &&
                     acknowledgementFlags.anyNeighborInExchangeOrLoadingState))
                {
                    sendDirectAcknowledgment = true;
                }
            }
        }
        else {
            if (acknowledgementFlags.lsaIsNewer) {
                intf->addDelayedAcknowledgement(lsaHeader);
            }
            else {
                if ((acknowledgementFlags.lsaIsDuplicate && !acknowledgementFlags.impliedAcknowledgement) ||
                    (acknowledgementFlags.lsaReachedMaxAge &&
                     acknowledgementFlags.noLSAInstanceInDatabase &&
                     acknowledgementFlags.anyNeighborInExchangeOrLoadingState))
                {
                    sendDirectAcknowledgment = true;
                }
            }
        }
    }

    if (sendDirectAcknowledgment) {
        const auto& ackPacket = makeShared<OspfLinkStateAcknowledgementPacket>();

        ackPacket->setType(LINKSTATE_ACKNOWLEDGEMENT_PACKET);
        ackPacket->setRouterID(Ipv4Address(router->getRouterID()));
        ackPacket->setAreaID(Ipv4Address(intf->getArea()->getAreaID()));
        ackPacket->setAuthenticationType(intf->getAuthenticationType());
        AuthenticationKeyType authKey = intf->getAuthenticationKey();
        for (int i = 0; i < 8; i++) {
            ackPacket->setAuthentication(i, authKey.bytes[i]);
        }

        ackPacket->setLsaHeadersArraySize(1);
        ackPacket->setLsaHeaders(0, lsaHeader);

        ackPacket->setChunkLength(OSPF_HEADER_LENGTH + OSPF_LSA_HEADER_LENGTH);
        Packet *pk = new Packet();
        pk->insertAtBack(ackPacket);

        int ttl = (intf->getType() == Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;

        if (intf->getType() == Interface::BROADCAST) {
            if ((intf->getState() == Interface::DESIGNATED_ROUTER_STATE) ||
                (intf->getState() == Interface::BACKUP_STATE) ||
                (intf->getDesignatedRouter() == NULL_DESIGNATEDROUTERID))
            {
                router->getMessageHandler()->sendPacket(pk, Ipv4Address::ALL_OSPF_ROUTERS_MCAST, intf->getIfIndex(), ttl);
            }
            else {
                router->getMessageHandler()->sendPacket(pk, Ipv4Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST, intf->getIfIndex(), ttl);
            }
        }
        else {
            if (intf->getType() == Interface::POINTTOPOINT) {
                router->getMessageHandler()->sendPacket(pk, Ipv4Address::ALL_OSPF_ROUTERS_MCAST, intf->getIfIndex(), ttl);
            }
            else {
                Neighbor *neighbor = intf->getNeighborById(lsaSource);
                ASSERT(neighbor);
                router->getMessageHandler()->sendPacket(pk, neighbor->getAddress(), intf->getIfIndex(), ttl);
            }
        }
    }
}

} // namespace ospf

} // namespace inet

