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
void OSPF::LinkStateUpdateHandler::ProcessPacket(OSPFPacket* packet, OSPF::Interface* intf, OSPF::Neighbor* neighbor)
{
    router->GetMessageHandler()->PrintEvent("Link State Update packet received", intf, neighbor);

    OSPFLinkStateUpdatePacket* lsUpdatePacket      = check_and_cast<OSPFLinkStateUpdatePacket*> (packet);
    bool                       rebuildRoutingTable = false;

    if (neighbor->GetState() >= OSPF::Neighbor::ExchangeState) {
        OSPF::AreaID areaID          = lsUpdatePacket->getAreaID().getInt();
        OSPF::Area*  area            = router->GetArea(areaID);
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

                if (!ValidateLSChecksum(currentLSA)) {
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
                PrintLSAHeader(currentLSA->getHeader(), ev.getOStream());
                EV << "\n";

                if ((lsaType == ASExternalLSAType) && (!area->GetExternalRoutingCapability())) {
                    continue;
                }
                OSPF::LSAKeyType lsaKey;

                lsaKey.linkStateID = currentLSA->getHeader().getLinkStateID();
                lsaKey.advertisingRouter = currentLSA->getHeader().getAdvertisingRouter().getInt();

                OSPFLSA*                lsaInDatabase = router->FindLSA(lsaType, lsaKey, areaID);
                unsigned short          lsAge         = currentLSA->getHeader().getLsAge();
                AcknowledgementFlags    ackFlags;

                ackFlags.floodedBackOut = false;
                ackFlags.lsaIsNewer = false;
                ackFlags.lsaIsDuplicate = false;
                ackFlags.impliedAcknowledgement = false;
                ackFlags.lsaReachedMaxAge = (lsAge == MAX_AGE);
                ackFlags.noLSAInstanceInDatabase = (lsaInDatabase == NULL);
                ackFlags.anyNeighborInExchangeOrLoadingState = router->HasAnyNeighborInStates(OSPF::Neighbor::ExchangeState | OSPF::Neighbor::LoadingState);

                if ((ackFlags.lsaReachedMaxAge) && (ackFlags.noLSAInstanceInDatabase) && (!ackFlags.anyNeighborInExchangeOrLoadingState)) {
                    if (intf->GetType() == OSPF::Interface::Broadcast) {
                        if ((intf->GetState() == OSPF::Interface::DesignatedRouterState) ||
                            (intf->GetState() == OSPF::Interface::BackupState) ||
                            (intf->GetDesignatedRouter() == OSPF::NullDesignatedRouterID))
                        {
                            intf->SendLSAcknowledgement(&(currentLSA->getHeader()), OSPF::AllSPFRouters);
                        } else {
                            intf->SendLSAcknowledgement(&(currentLSA->getHeader()), OSPF::AllDRouters);
                        }
                    } else {
                        if (intf->GetType() == OSPF::Interface::PointToPoint) {
                            intf->SendLSAcknowledgement(&(currentLSA->getHeader()), OSPF::AllSPFRouters);
                        } else {
                            intf->SendLSAcknowledgement(&(currentLSA->getHeader()), neighbor->GetAddress());
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
                        (info->GetSource() == LSATrackingInfo::Flooded) &&
                        (info->GetInstallTime() < MIN_LS_ARRIVAL))
                    {
                        continue;
                    }
                    ackFlags.floodedBackOut = router->FloodLSA(currentLSA, areaID, intf, neighbor);
                    if (!ackFlags.noLSAInstanceInDatabase) {
                        OSPF::LSAKeyType lsaKey;

                        lsaKey.linkStateID = lsaInDatabase->getHeader().getLinkStateID();
                        lsaKey.advertisingRouter = lsaInDatabase->getHeader().getAdvertisingRouter().getInt();

                        router->RemoveFromAllRetransmissionLists(lsaKey);
                    }
                    rebuildRoutingTable |= router->InstallLSA(currentLSA, areaID);

                    EV << "    (update installed)\n";

                    AcknowledgeLSA(currentLSA->getHeader(), intf, ackFlags, lsUpdatePacket->getRouterID().getInt());
                    if ((currentLSA->getHeader().getAdvertisingRouter().getInt() == router->GetRouterID()) ||
                        ((lsaType == NetworkLSAType) &&
                         (router->IsLocalAddress(IPv4AddressFromULong(currentLSA->getHeader().getLinkStateID())))))
                    {
                        if (ackFlags.noLSAInstanceInDatabase) {
                            currentLSA->getHeader().setLsAge(MAX_AGE);
                            router->FloodLSA(currentLSA, areaID);
                        } else {
                            if (ackFlags.lsaIsNewer) {
                                long sequenceNumber = currentLSA->getHeader().getLsSequenceNumber();
                                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                                    lsaInDatabase->getHeader().setLsAge(MAX_AGE);
                                    router->FloodLSA(lsaInDatabase, areaID);
                                } else {
                                    lsaInDatabase->getHeader().setLsSequenceNumber(sequenceNumber + 1);
                                    router->FloodLSA(lsaInDatabase, areaID);
                                }
                            }
                        }
                    }
                    continue;
                }
                if (neighbor->IsLSAOnRequestList(lsaKey)) {
                    neighbor->ProcessEvent(OSPF::Neighbor::BadLinkStateRequest);
                    break;
                }
                if (ackFlags.lsaIsDuplicate) {
                    if (neighbor->IsLSAOnRetransmissionList(lsaKey)) {
                        neighbor->RemoveFromRetransmissionList(lsaKey);
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
                if (!neighbor->IsOnTransmittedLSAList(lsaKey)) {
                    OSPFLinkStateUpdatePacket* updatePacket = intf->CreateUpdatePacket(lsaInDatabase);
                    if (updatePacket != NULL) {
                        int ttl = (intf->GetType() == OSPF::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;

                        if (intf->GetType() == OSPF::Interface::Broadcast) {
                            if ((intf->GetState() == OSPF::Interface::DesignatedRouterState) ||
                                (intf->GetState() == OSPF::Interface::BackupState) ||
                                (intf->GetDesignatedRouter() == OSPF::NullDesignatedRouterID))
                            {
                                router->GetMessageHandler()->SendPacket(updatePacket, OSPF::AllSPFRouters, intf->GetIfIndex(), ttl);
                            } else {
                                router->GetMessageHandler()->SendPacket(updatePacket, OSPF::AllDRouters, intf->GetIfIndex(), ttl);
                            }
                        } else {
                            if (intf->GetType() == OSPF::Interface::PointToPoint) {
                                router->GetMessageHandler()->SendPacket(updatePacket, OSPF::AllSPFRouters, intf->GetIfIndex(), ttl);
                            } else {
                                router->GetMessageHandler()->SendPacket(updatePacket, neighbor->GetAddress(), intf->GetIfIndex(), ttl);
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
        if (intf->GetState() == OSPF::Interface::BackupState) {
            if ((acknowledgementFlags.lsaIsNewer && (lsaSource == intf->GetDesignatedRouter().routerID)) ||
                (acknowledgementFlags.lsaIsDuplicate && acknowledgementFlags.impliedAcknowledgement))
            {
                intf->AddDelayedAcknowledgement(lsaHeader);
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
                intf->AddDelayedAcknowledgement(lsaHeader);
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
        ackPacket->setRouterID(router->GetRouterID());
        ackPacket->setAreaID(intf->GetArea()->GetAreaID());
        ackPacket->setAuthenticationType(intf->GetAuthenticationType());
        OSPF::AuthenticationKeyType authKey = intf->GetAuthenticationKey();
        for (int i = 0; i < 8; i++) {
            ackPacket->setAuthentication(i, authKey.bytes[i]);
        }

        ackPacket->setLsaHeadersArraySize(1);
        ackPacket->setLsaHeaders(0, lsaHeader);

        ackPacket->setPacketLength(0); // TODO: Calculate correct length
        ackPacket->setChecksum(0); // TODO: Calculate correct cheksum(16-bit one's complement of the entire packet)

        int ttl = (intf->GetType() == OSPF::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;

        if (intf->GetType() == OSPF::Interface::Broadcast) {
            if ((intf->GetState() == OSPF::Interface::DesignatedRouterState) ||
                (intf->GetState() == OSPF::Interface::BackupState) ||
                (intf->GetDesignatedRouter() == OSPF::NullDesignatedRouterID))
            {
                router->GetMessageHandler()->SendPacket(ackPacket, OSPF::AllSPFRouters, intf->GetIfIndex(), ttl);
            } else {
                router->GetMessageHandler()->SendPacket(ackPacket, OSPF::AllDRouters, intf->GetIfIndex(), ttl);
            }
        } else {
            if (intf->GetType() == OSPF::Interface::PointToPoint) {
                router->GetMessageHandler()->SendPacket(ackPacket, OSPF::AllSPFRouters, intf->GetIfIndex(), ttl);
            } else {
                router->GetMessageHandler()->SendPacket(ackPacket, intf->GetNeighborByID(lsaSource)->GetAddress(), intf->GetIfIndex(), ttl);
            }
        }
    }
}
