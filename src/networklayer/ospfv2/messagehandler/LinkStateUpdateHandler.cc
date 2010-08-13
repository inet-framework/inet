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
#include "OSPFcommon.h"
#include "OSPFRouter.h"
#include "OSPFArea.h"
#include "OSPFNeighbor.h"

class LSAProcessingMarker
{
private:
    unsigned int index;

public:
    LSAProcessingMarker(unsigned int counter) : index(counter) { EV << "    --> Processing LSA(" << index << ")\n"; }
    ~LSAProcessingMarker()                                      { EV << "    <-- LSA(" << index << ") processed.\n"; }
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

    OSPFLinkStateUpdatePacket* lsUpdatePacket      = check_and_cast<OSPFLinkStateUpdatePacket*> (packet);
    bool                       rebuildRoutingTable = false;

    if (neighbor->getState() >= OSPF::Neighbor::ExchangeState) {
        OSPF::AreaID areaID          = lsUpdatePacket->getAreaID().getInt();
        OSPF::Area*  area            = router->getArea(areaID);
        LSAType      currentType     = RouterLSAType;
        unsigned int currentLSAIndex = 0;

        EV << "  Processing packet contents:\n";

        while (currentType <= ASExternalLSAType) {
            unsigned int lsaCount = 0;

            switch (currentType) {
                case RouterLSAType:
                    lsaCount = lsUpdatePacket->getRouterLSAsArraySize();
                    break;
                case NetworkLSAType:
                    lsaCount = lsUpdatePacket->getNetworkLSAsArraySize();
                    break;
                case SummaryLSA_NetworksType:
                case SummaryLSA_ASBoundaryRoutersType:
                    lsaCount = lsUpdatePacket->getSummaryLSAsArraySize();
                    break;
                case ASExternalLSAType:
                    lsaCount = lsUpdatePacket->getAsExternalLSAsArraySize();
                    break;
                default: break;
            }

            for (unsigned int i = 0; i < lsaCount; i++) {
                OSPFLSA* currentLSA;

                switch (currentType) {
                    case RouterLSAType:
                        currentLSA = (&(lsUpdatePacket->getRouterLSAs(i)));
                        break;
                    case NetworkLSAType:
                        currentLSA = (&(lsUpdatePacket->getNetworkLSAs(i)));
                        break;
                    case SummaryLSA_NetworksType:
                    case SummaryLSA_ASBoundaryRoutersType:
                        currentLSA = (&(lsUpdatePacket->getSummaryLSAs(i)));
                        break;
                    case ASExternalLSAType:
                        currentLSA = (&(lsUpdatePacket->getAsExternalLSAs(i)));
                        break;
                    default: break;
                }

                if (!validateLSChecksum(currentLSA)) {
                    continue;
                }

                LSAType lsaType = static_cast<LSAType> (currentLSA->getHeader().getLsType());
                if ((lsaType != RouterLSAType) &&
                    (lsaType != NetworkLSAType) &&
                    (lsaType != SummaryLSA_NetworksType) &&
                    (lsaType != SummaryLSA_ASBoundaryRoutersType) &&
                    (lsaType != ASExternalLSAType))
                {
                    continue;
                }

                LSAProcessingMarker marker(currentLSAIndex++);
                EV << "    ";
                printLSAHeader(currentLSA->getHeader(), ev.getOStream());
                EV << "\n";

                if ((lsaType == ASExternalLSAType) && (!area->getExternalRoutingCapability())) {
                    continue;
                }
                OSPF::LSAKeyType lsaKey;

                lsaKey.linkStateID = currentLSA->getHeader().getLinkStateID();
                lsaKey.advertisingRouter = currentLSA->getHeader().getAdvertisingRouter().getInt();

                OSPFLSA*                lsaInDatabase = router->findLSA(lsaType, lsaKey, areaID);
                unsigned short          lsAge         = currentLSA->getHeader().getLsAge();
                AcknowledgementFlags    ackFlags;

                ackFlags.floodedBackOut = false;
                ackFlags.lsaIsNewer = false;
                ackFlags.lsaIsDuplicate = false;
                ackFlags.impliedAcknowledgement = false;
                ackFlags.lsaReachedMaxAge = (lsAge == MAX_AGE);
                ackFlags.noLSAInstanceInDatabase = (lsaInDatabase == NULL);
                ackFlags.anyNeighborInExchangeOrLoadingState = router->hasAnyNeighborInStates(OSPF::Neighbor::ExchangeState | OSPF::Neighbor::LoadingState);

                if ((ackFlags.lsaReachedMaxAge) && (ackFlags.noLSAInstanceInDatabase) && (!ackFlags.anyNeighborInExchangeOrLoadingState)) {
                    if (intf->getType() == OSPF::Interface::Broadcast) {
                        if ((intf->getState() == OSPF::Interface::DesignatedRouterState) ||
                            (intf->getState() == OSPF::Interface::BackupState) ||
                            (intf->getDesignatedRouter() == OSPF::NullDesignatedRouterID))
                        {
                            intf->sendLSAcknowledgement(&(currentLSA->getHeader()), OSPF::AllSPFRouters);
                        } else {
                            intf->sendLSAcknowledgement(&(currentLSA->getHeader()), OSPF::AllDRouters);
                        }
                    } else {
                        if (intf->getType() == OSPF::Interface::PointToPoint) {
                            intf->sendLSAcknowledgement(&(currentLSA->getHeader()), OSPF::AllSPFRouters);
                        } else {
                            intf->sendLSAcknowledgement(&(currentLSA->getHeader()), neighbor->getAddress());
                        }
                    }
                    continue;
                }

                if (!ackFlags.noLSAInstanceInDatabase) {
                    // operator< and operator== on OSPFLSAHeaders determines which one is newer(less means older)
                    ackFlags.lsaIsNewer = (lsaInDatabase->getHeader() < currentLSA->getHeader());
                    ackFlags.lsaIsDuplicate = (operator== (lsaInDatabase->getHeader(), currentLSA->getHeader()));
                }
                if ((ackFlags.noLSAInstanceInDatabase) || (ackFlags.lsaIsNewer)) {
                    LSATrackingInfo* info = (!ackFlags.noLSAInstanceInDatabase) ? dynamic_cast<LSATrackingInfo*> (lsaInDatabase) : NULL;
                    if ((!ackFlags.noLSAInstanceInDatabase) &&
                        (info != NULL) &&
                        (info->getSource() == LSATrackingInfo::Flooded) &&
                        (info->getInstallTime() < MIN_LS_ARRIVAL))
                    {
                        continue;
                    }
                    ackFlags.floodedBackOut = router->floodLSA(currentLSA, areaID, intf, neighbor);
                    if (!ackFlags.noLSAInstanceInDatabase) {
                        OSPF::LSAKeyType lsaKey;

                        lsaKey.linkStateID = lsaInDatabase->getHeader().getLinkStateID();
                        lsaKey.advertisingRouter = lsaInDatabase->getHeader().getAdvertisingRouter().getInt();

                        router->removeFromAllRetransmissionLists(lsaKey);
                    }
                    rebuildRoutingTable |= router->installLSA(currentLSA, areaID);

                    EV << "    (update installed)\n";

                    AcknowledgeLSA(currentLSA->getHeader(), intf, ackFlags, lsUpdatePacket->getRouterID().getInt());
                    if ((currentLSA->getHeader().getAdvertisingRouter().getInt() == router->getRouterID()) ||
                        ((lsaType == NetworkLSAType) &&
                         (router->isLocalAddress(IPv4AddressFromULong(currentLSA->getHeader().getLinkStateID())))))
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
                    neighbor->processEvent(OSPF::Neighbor::BadLinkStateRequest);
                    break;
                }
                if (ackFlags.lsaIsDuplicate) {
                    if (neighbor->isLinkStateRequestListEmpty(lsaKey)) {
                        neighbor->removeFromRetransmissionList(lsaKey);
                        ackFlags.impliedAcknowledgement = true;
                    }
                    AcknowledgeLSA(currentLSA->getHeader(), intf, ackFlags, lsUpdatePacket->getRouterID().getInt());
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
                        int ttl = (intf->getType() == OSPF::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;

                        if (intf->getType() == OSPF::Interface::Broadcast) {
                            if ((intf->getState() == OSPF::Interface::DesignatedRouterState) ||
                                (intf->getState() == OSPF::Interface::BackupState) ||
                                (intf->getDesignatedRouter() == OSPF::NullDesignatedRouterID))
                            {
                                router->getMessageHandler()->sendPacket(updatePacket, OSPF::AllSPFRouters, intf->getIfIndex(), ttl);
                            } else {
                                router->getMessageHandler()->sendPacket(updatePacket, OSPF::AllDRouters, intf->getIfIndex(), ttl);
                            }
                        } else {
                            if (intf->getType() == OSPF::Interface::PointToPoint) {
                                router->getMessageHandler()->sendPacket(updatePacket, OSPF::AllSPFRouters, intf->getIfIndex(), ttl);
                            } else {
                                router->getMessageHandler()->sendPacket(updatePacket, neighbor->getAddress(), intf->getIfIndex(), ttl);
                            }
                        }
                    }
                }
            }
            currentType = static_cast<LSAType> (currentType + 1);
            if (currentType == SummaryLSA_NetworksType) {
                currentType = static_cast<LSAType> (currentType + 1);
            }
        }
    }

    if (rebuildRoutingTable) {
        router->RebuildRoutingTable();
    }
}

void OSPF::LinkStateUpdateHandler::AcknowledgeLSA(OSPFLSAHeader& lsaHeader,
                                                   OSPF::Interface* intf,
                                                   OSPF::LinkStateUpdateHandler::AcknowledgementFlags acknowledgementFlags,
                                                   OSPF::RouterID lsaSource)
{
    bool sendDirectAcknowledgment = false;

    if (!acknowledgementFlags.floodedBackOut) {
        if (intf->getState() == OSPF::Interface::BackupState) {
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
        OSPFLinkStateAcknowledgementPacket* ackPacket = new OSPFLinkStateAcknowledgementPacket;

        ackPacket->setType(LinkStateAcknowledgementPacket);
        ackPacket->setRouterID(router->getRouterID());
        ackPacket->setAreaID(intf->getArea()->getAreaID());
        ackPacket->setAuthenticationType(intf->getAuthenticationType());
        OSPF::AuthenticationKeyType authKey = intf->getAuthenticationKey();
        for (int i = 0; i < 8; i++) {
            ackPacket->setAuthentication(i, authKey.bytes[i]);
        }

        ackPacket->setLsaHeadersArraySize(1);
        ackPacket->setLsaHeaders(0, lsaHeader);

        ackPacket->setPacketLength(0); // TODO: Calculate correct length
        ackPacket->setChecksum(0); // TODO: Calculate correct cheksum(16-bit one's complement of the entire packet)

        int ttl = (intf->getType() == OSPF::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;

        if (intf->getType() == OSPF::Interface::Broadcast) {
            if ((intf->getState() == OSPF::Interface::DesignatedRouterState) ||
                (intf->getState() == OSPF::Interface::BackupState) ||
                (intf->getDesignatedRouter() == OSPF::NullDesignatedRouterID))
            {
                router->getMessageHandler()->sendPacket(ackPacket, OSPF::AllSPFRouters, intf->getIfIndex(), ttl);
            } else {
                router->getMessageHandler()->sendPacket(ackPacket, OSPF::AllDRouters, intf->getIfIndex(), ttl);
            }
        } else {
            if (intf->getType() == OSPF::Interface::PointToPoint) {
                router->getMessageHandler()->sendPacket(ackPacket, OSPF::AllSPFRouters, intf->getIfIndex(), ttl);
            } else {
                router->getMessageHandler()->sendPacket(ackPacket, intf->getNeighborByID(lsaSource)->getAddress(), intf->getIfIndex(), ttl);
            }
        }
    }
}
