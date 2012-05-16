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


#include "LinkStateUpdateHandler.h"

#include "OSPFArea.h"
#include "OSPFcommon.h"
#include "OSPFNeighbor.h"
#include "OSPFRouter.h"


class LSAProcessingMarker
{
private:
    unsigned int index;

public:
    LSAProcessingMarker(unsigned int counter) : index(counter) { EV << "    --> Processing LSA(" << index << ")\n"; }
    ~LSAProcessingMarker()  { EV << "    <-- LSA(" << index << ") processed.\n"; }
};


OSPF::LinkStateUpdateHandler::LinkStateUpdateHandler(OSPF::Router* containingRouter) :
    OSPF::IMessageHandler(containingRouter)
{
}

/**
 * @see RFC2328 Section 13.
 */
void OSPF::LinkStateUpdateHandler::processPacket(OSPFPacket* packet, OSPF::Interface* intf, OSPF::Neighbor* neighbor)
{
    router->getMessageHandler()->printEvent("Link State update packet received", intf, neighbor);

    OSPFLinkStateUpdatePacket* lsUpdatePacket = check_and_cast<OSPFLinkStateUpdatePacket*> (packet);
    bool shouldRebuildRoutingTable = false;

    if (neighbor->getState() >= OSPF::Neighbor::EXCHANGE_STATE) {
        OSPF::AreaID areaID = lsUpdatePacket->getAreaID();
        OSPF::Area* area = router->getAreaByID(areaID);
        LSAType currentType = ROUTERLSA_TYPE;
        unsigned int currentLSAIndex = 0;

        EV << "  Processing packet contents:\n";

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
                default: break;
            }

            for (unsigned int i = 0; i < lsaCount; i++) {
                OSPFLSA* currentLSA;

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
                    default: break;
                }

                if (!validateLSChecksum(currentLSA)) {
                    continue;
                }

                LSAType lsaType = static_cast<LSAType> (currentLSA->getHeader().getLsType());
                if ((lsaType != ROUTERLSA_TYPE) &&
                    (lsaType != NETWORKLSA_TYPE) &&
                    (lsaType != SUMMARYLSA_NETWORKS_TYPE) &&
                    (lsaType != SUMMARYLSA_ASBOUNDARYROUTERS_TYPE) &&
                    (lsaType != AS_EXTERNAL_LSA_TYPE))
                {
                    continue;
                }

                LSAProcessingMarker marker(currentLSAIndex++);
                EV << "    ";
                printLSAHeader(currentLSA->getHeader(), ev.getOStream());
                EV << "\n";

                if ((lsaType == AS_EXTERNAL_LSA_TYPE) && (!area->getExternalRoutingCapability())) {
                    continue;
                }
                OSPF::LSAKeyType lsaKey;

                lsaKey.linkStateID = currentLSA->getHeader().getLinkStateID();
                lsaKey.advertisingRouter = currentLSA->getHeader().getAdvertisingRouter();

                OSPFLSA* lsaInDatabase = router->findLSA(lsaType, lsaKey, areaID);
                unsigned short lsAge = currentLSA->getHeader().getLsAge();
                AcknowledgementFlags ackFlags;

                ackFlags.floodedBackOut = false;
                ackFlags.lsaIsNewer = false;
                ackFlags.lsaIsDuplicate = false;
                ackFlags.impliedAcknowledgement = false;
                ackFlags.lsaReachedMaxAge = (lsAge == MAX_AGE);
                ackFlags.noLSAInstanceInDatabase = (lsaInDatabase == NULL);
                ackFlags.anyNeighborInExchangeOrLoadingState = router->hasAnyNeighborInStates(OSPF::Neighbor::EXCHANGE_STATE | OSPF::Neighbor::LOADING_STATE);

                if ((ackFlags.lsaReachedMaxAge) && (ackFlags.noLSAInstanceInDatabase) && (!ackFlags.anyNeighborInExchangeOrLoadingState)) {
                    if (intf->getType() == OSPF::Interface::BROADCAST) {
                        if ((intf->getState() == OSPF::Interface::DESIGNATED_ROUTER_STATE) ||
                            (intf->getState() == OSPF::Interface::BACKUP_STATE) ||
                            (intf->getDesignatedRouter() == OSPF::NULL_DESIGNATEDROUTERID))
                        {
                            intf->sendLSAcknowledgement(&(currentLSA->getHeader()), IPv4Address::ALL_OSPF_ROUTERS_MCAST);
                        } else {
                            intf->sendLSAcknowledgement(&(currentLSA->getHeader()), IPv4Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST);
                        }
                    } else {
                        if (intf->getType() == OSPF::Interface::POINTTOPOINT) {
                            intf->sendLSAcknowledgement(&(currentLSA->getHeader()), IPv4Address::ALL_OSPF_ROUTERS_MCAST);
                        } else {
                            intf->sendLSAcknowledgement(&(currentLSA->getHeader()), neighbor->getAddress());
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
                    LSATrackingInfo* info = (!ackFlags.noLSAInstanceInDatabase) ? dynamic_cast<LSATrackingInfo*> (lsaInDatabase) : NULL;
                    if ((!ackFlags.noLSAInstanceInDatabase) &&
                        (info != NULL) &&
                        (info->getSource() == LSATrackingInfo::FLOODED) &&
                        (info->getInstallTime() < MIN_LS_ARRIVAL))
                    {
                        continue;
                    }
                    ackFlags.floodedBackOut = router->floodLSA(currentLSA, areaID, intf, neighbor);
                    if (!ackFlags.noLSAInstanceInDatabase) {
                        OSPF::LSAKeyType lsaKey;

                        lsaKey.linkStateID = lsaInDatabase->getHeader().getLinkStateID();
                        lsaKey.advertisingRouter = lsaInDatabase->getHeader().getAdvertisingRouter();

                        router->removeFromAllRetransmissionLists(lsaKey);
                    }
                    shouldRebuildRoutingTable |= router->installLSA(currentLSA, areaID);

                    // Add externalIPRoute in IPRoutingTable if this route is learned by BGP
                    if (currentType == AS_EXTERNAL_LSA_TYPE)
                    {
                        OSPFASExternalLSA* externalLSA = &(lsUpdatePacket->getAsExternalLSAs(0));
                        if (externalLSA->getContents().getExternalRouteTag() == OSPF_EXTERNAL_ROUTES_LEARNED_BY_BGP)
                        {
                            IPv4Address externalAddr = currentLSA->getHeader().getLinkStateID();
                            int ifName = intf->getIfIndex();
                           router->addExternalRouteInIPTable(externalAddr, externalLSA->getContents(), ifName);
                        }
                    }

                    EV << "    (update installed)\n";

                    acknowledgeLSA(currentLSA->getHeader(), intf, ackFlags, lsUpdatePacket->getRouterID());
                    if ((currentLSA->getHeader().getAdvertisingRouter() == router->getRouterID()) ||
                        ((lsaType == NETWORKLSA_TYPE) &&
                         (router->isLocalAddress(currentLSA->getHeader().getLinkStateID()))))
                    {
                        if (ackFlags.noLSAInstanceInDatabase) {
                            currentLSA->getHeader().setLsAge(MAX_AGE);
                            router->floodLSA(currentLSA, areaID);
                        } else {
                            if (ackFlags.lsaIsNewer) {
                                long sequenceNumber = currentLSA->getHeader().getLsSequenceNumber();
                                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                                    lsaInDatabase->getHeader().setLsAge(MAX_AGE);
                                    router->floodLSA(lsaInDatabase, areaID);
                                } else {
                                    lsaInDatabase->getHeader().setLsSequenceNumber(sequenceNumber + 1);
                                    router->floodLSA(lsaInDatabase, areaID);
                                }
                            }
                        }
                    }
                    continue;
                }
                if (neighbor->isLSAOnRequestList(lsaKey)) {
                    neighbor->processEvent(OSPF::Neighbor::BAD_LINK_STATE_REQUEST);
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
                    OSPFLinkStateUpdatePacket* updatePacket = intf->createUpdatePacket(lsaInDatabase);
                    if (updatePacket != NULL) {
                        int ttl = (intf->getType() == OSPF::Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;

                        if (intf->getType() == OSPF::Interface::BROADCAST) {
                            if ((intf->getState() == OSPF::Interface::DESIGNATED_ROUTER_STATE) ||
                                (intf->getState() == OSPF::Interface::BACKUP_STATE) ||
                                (intf->getDesignatedRouter() == OSPF::NULL_DESIGNATEDROUTERID))
                            {
                                router->getMessageHandler()->sendPacket(updatePacket, IPv4Address::ALL_OSPF_ROUTERS_MCAST, intf->getIfIndex(), ttl);
                            } else {
                                router->getMessageHandler()->sendPacket(updatePacket, IPv4Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST, intf->getIfIndex(), ttl);
                            }
                        } else {
                            if (intf->getType() == OSPF::Interface::POINTTOPOINT) {
                                router->getMessageHandler()->sendPacket(updatePacket, IPv4Address::ALL_OSPF_ROUTERS_MCAST, intf->getIfIndex(), ttl);
                            } else {
                                router->getMessageHandler()->sendPacket(updatePacket, neighbor->getAddress(), intf->getIfIndex(), ttl);
                            }
                        }
                    }
                }
            }
            currentType = static_cast<LSAType> (currentType + 1);
            if (currentType == SUMMARYLSA_NETWORKS_TYPE) {
                currentType = static_cast<LSAType> (currentType + 1);
            }
        }
    }

    if (shouldRebuildRoutingTable) {
        router->rebuildRoutingTable();
    }
}

void OSPF::LinkStateUpdateHandler::acknowledgeLSA(OSPFLSAHeader& lsaHeader,
                                                   OSPF::Interface* intf,
                                                   OSPF::LinkStateUpdateHandler::AcknowledgementFlags acknowledgementFlags,
                                                   OSPF::RouterID lsaSource)
{
    bool sendDirectAcknowledgment = false;

    if (!acknowledgementFlags.floodedBackOut) {
        if (intf->getState() == OSPF::Interface::BACKUP_STATE) {
            if ((acknowledgementFlags.lsaIsNewer && (lsaSource == intf->getDesignatedRouter().routerID)) ||
                (acknowledgementFlags.lsaIsDuplicate && acknowledgementFlags.impliedAcknowledgement))
            {
                intf->addDelayedAcknowledgement(lsaHeader);
            } else {
                if ((acknowledgementFlags.lsaIsDuplicate && !acknowledgementFlags.impliedAcknowledgement) ||
                    (acknowledgementFlags.lsaReachedMaxAge &&
                     acknowledgementFlags.noLSAInstanceInDatabase &&
                     acknowledgementFlags.anyNeighborInExchangeOrLoadingState))
                {
                    sendDirectAcknowledgment = true;
                }
            }
        } else {
            if (acknowledgementFlags.lsaIsNewer) {
                intf->addDelayedAcknowledgement(lsaHeader);
            } else {
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
        OSPFLinkStateAcknowledgementPacket* ackPacket = new OSPFLinkStateAcknowledgementPacket();

        ackPacket->setType(LINKSTATE_ACKNOWLEDGEMENT_PACKET);
        ackPacket->setRouterID(IPv4Address(router->getRouterID()));
        ackPacket->setAreaID(IPv4Address(intf->getArea()->getAreaID()));
        ackPacket->setAuthenticationType(intf->getAuthenticationType());
        OSPF::AuthenticationKeyType authKey = intf->getAuthenticationKey();
        for (int i = 0; i < 8; i++) {
            ackPacket->setAuthentication(i, authKey.bytes[i]);
        }

        ackPacket->setLsaHeadersArraySize(1);
        ackPacket->setLsaHeaders(0, lsaHeader);

        ackPacket->setByteLength(OSPF_HEADER_LENGTH + OSPF_LSA_HEADER_LENGTH);

        int ttl = (intf->getType() == OSPF::Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;

        if (intf->getType() == OSPF::Interface::BROADCAST) {
            if ((intf->getState() == OSPF::Interface::DESIGNATED_ROUTER_STATE) ||
                (intf->getState() == OSPF::Interface::BACKUP_STATE) ||
                (intf->getDesignatedRouter() == OSPF::NULL_DESIGNATEDROUTERID))
            {
                router->getMessageHandler()->sendPacket(ackPacket, IPv4Address::ALL_OSPF_ROUTERS_MCAST, intf->getIfIndex(), ttl);
            } else {
                router->getMessageHandler()->sendPacket(ackPacket, IPv4Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST, intf->getIfIndex(), ttl);
            }
        } else {
            if (intf->getType() == OSPF::Interface::POINTTOPOINT) {
                router->getMessageHandler()->sendPacket(ackPacket, IPv4Address::ALL_OSPF_ROUTERS_MCAST, intf->getIfIndex(), ttl);
            } else {
                router->getMessageHandler()->sendPacket(ackPacket, intf->getNeighborByID(lsaSource)->getAddress(), intf->getIfIndex(), ttl);
            }
        }
    }
}
