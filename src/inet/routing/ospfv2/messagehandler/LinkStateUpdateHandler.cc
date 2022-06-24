//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/routing/ospfv2/messagehandler/LinkStateUpdateHandler.h"

#include "inet/routing/ospfv2/Ospfv2Crc.h"
#include "inet/routing/ospfv2/neighbor/Ospfv2Neighbor.h"
#include "inet/routing/ospfv2/router/Ospfv2Area.h"
#include "inet/routing/ospfv2/router/Ospfv2Common.h"
#include "inet/routing/ospfv2/router/Ospfv2Router.h"

namespace inet {

namespace ospfv2 {

using namespace ospf;

class INET_API LsaProcessingMarker
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
void LinkStateUpdateHandler::processPacket(Packet *packet, Ospfv2Interface *intf, Neighbor *neighbor)
{
    router->getMessageHandler()->printEvent("Link State update packet received", intf, neighbor);

    const auto& lsUpdatePacket = packet->peekAtFront<Ospfv2LinkStateUpdatePacket>();
    bool shouldRebuildRoutingTable = false;

    if (neighbor->getState() >= Neighbor::EXCHANGE_STATE) {
        AreaId areaID = lsUpdatePacket->getAreaID();
        Ospfv2Area *area = router->getAreaByID(areaID);

        EV_INFO << "  Processing packet contents:\n";

        for (unsigned int i = 0; i < lsUpdatePacket->getOspfLSAsArraySize(); i++) {
            const Ospfv2Lsa *currentLSA = lsUpdatePacket->getOspfLSAs(i);

            if (!validateLSChecksum(currentLSA)) {
                continue;
            }

            Ospfv2LsaType lsaType = static_cast<Ospfv2LsaType>(currentLSA->getHeader().getLsType());
            if ((lsaType != ROUTERLSA_TYPE) &&
                (lsaType != NETWORKLSA_TYPE) &&
                (lsaType != SUMMARYLSA_NETWORKS_TYPE) &&
                (lsaType != SUMMARYLSA_ASBOUNDARYROUTERS_TYPE) &&
                (lsaType != AS_EXTERNAL_LSA_TYPE))
            {
                continue;
            }

            LsaProcessingMarker marker(i);
            EV_DETAIL << "    " << currentLSA->getHeader() << "\n";

            // FIXME area maybe nullptr
            if ((lsaType == AS_EXTERNAL_LSA_TYPE) && !(area != nullptr && area->getExternalRoutingCapability())) {
                continue;
            }
            LsaKeyType lsaKey;

            lsaKey.linkStateID = currentLSA->getHeader().getLinkStateID();
            lsaKey.advertisingRouter = currentLSA->getHeader().getAdvertisingRouter();

            Ospfv2Lsa *lsaInDatabase = router->findLSA(lsaType, lsaKey, areaID);
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
                if (intf->getType() == Ospfv2Interface::BROADCAST) {
                    if ((intf->getState() == Ospfv2Interface::DESIGNATED_ROUTER_STATE) ||
                        (intf->getState() == Ospfv2Interface::BACKUP_STATE) ||
                        (intf->getDesignatedRouter() == NULL_DESIGNATEDROUTERID))
                    {
                        intf->sendLsAcknowledgement(&(currentLSA->getHeader()), Ipv4Address::ALL_OSPF_ROUTERS_MCAST);
                    }
                    else {
                        intf->sendLsAcknowledgement(&(currentLSA->getHeader()), Ipv4Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST);
                    }
                }
                else {
                    if (intf->getType() == Ospfv2Interface::POINTTOPOINT) {
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
                    int ttl = (intf->getType() == Ospfv2Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;

                    if (intf->getType() == Ospfv2Interface::BROADCAST) {
                        if ((intf->getState() == Ospfv2Interface::DESIGNATED_ROUTER_STATE) ||
                            (intf->getState() == Ospfv2Interface::BACKUP_STATE) ||
                            (intf->getDesignatedRouter() == NULL_DESIGNATEDROUTERID))
                        {
                            router->getMessageHandler()->sendPacket(updatePacket, Ipv4Address::ALL_OSPF_ROUTERS_MCAST, intf, ttl);
                        }
                        else {
                            router->getMessageHandler()->sendPacket(updatePacket, Ipv4Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST, intf, ttl);
                        }
                    }
                    else {
                        if (intf->getType() == Ospfv2Interface::POINTTOPOINT) {
                            router->getMessageHandler()->sendPacket(updatePacket, Ipv4Address::ALL_OSPF_ROUTERS_MCAST, intf, ttl);
                        }
                        else {
                            router->getMessageHandler()->sendPacket(updatePacket, neighbor->getAddress(), intf, ttl);
                        }
                    }
                }
            }
        }
    }

    if (shouldRebuildRoutingTable)
        router->rebuildRoutingTable();
}

void LinkStateUpdateHandler::acknowledgeLSA(const Ospfv2LsaHeader& lsaHeader,
        Ospfv2Interface *intf,
        LinkStateUpdateHandler::AcknowledgementFlags acknowledgementFlags,
        RouterId lsaSource)
{
    bool sendDirectAcknowledgment = false;

    if (!acknowledgementFlags.floodedBackOut) {
        if (intf->getState() == Ospfv2Interface::BACKUP_STATE) {
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
        const auto& ackPacket = makeShared<Ospfv2LinkStateAcknowledgementPacket>();

        ackPacket->setType(LINKSTATE_ACKNOWLEDGEMENT_PACKET);
        ackPacket->setRouterID(Ipv4Address(router->getRouterID()));
        ackPacket->setAreaID(Ipv4Address(intf->getArea()->getAreaID()));
        ackPacket->setAuthenticationType(intf->getAuthenticationType());

        ackPacket->setLsaHeadersArraySize(1);
        ackPacket->setLsaHeaders(0, lsaHeader);

        ackPacket->setPacketLengthField(B(OSPFv2_HEADER_LENGTH + OSPFv2_LSA_HEADER_LENGTH).get());
        ackPacket->setChunkLength(B(ackPacket->getPacketLengthField()));

        AuthenticationKeyType authKey = intf->getAuthenticationKey();
        for (int i = 0; i < 8; i++) {
            ackPacket->setAuthentication(i, authKey.bytes[i]);
        }

        setOspfCrc(ackPacket, intf->getCrcMode());

        Packet *pk = new Packet();
        pk->insertAtBack(ackPacket);

        int ttl = (intf->getType() == Ospfv2Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;

        if (intf->getType() == Ospfv2Interface::BROADCAST) {
            if ((intf->getState() == Ospfv2Interface::DESIGNATED_ROUTER_STATE) ||
                (intf->getState() == Ospfv2Interface::BACKUP_STATE) ||
                (intf->getDesignatedRouter() == NULL_DESIGNATEDROUTERID))
            {
                router->getMessageHandler()->sendPacket(pk, Ipv4Address::ALL_OSPF_ROUTERS_MCAST, intf, ttl);
            }
            else {
                router->getMessageHandler()->sendPacket(pk, Ipv4Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST, intf, ttl);
            }
        }
        else {
            if (intf->getType() == Ospfv2Interface::POINTTOPOINT) {
                router->getMessageHandler()->sendPacket(pk, Ipv4Address::ALL_OSPF_ROUTERS_MCAST, intf, ttl);
            }
            else {
                Neighbor *neighbor = intf->getNeighborById(lsaSource);
                ASSERT(neighbor);
                router->getMessageHandler()->sendPacket(pk, neighbor->getAddress(), intf, ttl);
            }
        }
    }
}

} // namespace ospfv2

} // namespace inet

