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

#include "inet/routing/ospfv2/messagehandler/DatabaseDescriptionHandler.h"

#include "inet/routing/ospfv2/router/OSPFArea.h"
#include "inet/routing/ospfv2/interface/OSPFInterface.h"
#include "inet/routing/ospfv2/neighbor/OSPFNeighbor.h"
#include "inet/routing/ospfv2/router/OSPFRouter.h"

namespace inet {

namespace ospf {

DatabaseDescriptionHandler::DatabaseDescriptionHandler(Router *containingRouter) :
    IMessageHandler(containingRouter)
{
}

void DatabaseDescriptionHandler::processPacket(OSPFPacket *packet, Interface *intf, Neighbor *neighbor)
{
    router->getMessageHandler()->printEvent("Database Description packet received", intf, neighbor);

    OSPFDatabaseDescriptionPacket *ddPacket = check_and_cast<OSPFDatabaseDescriptionPacket *>(packet);

    Neighbor::NeighborStateType neighborState = neighbor->getState();

    if ((ddPacket->getInterfaceMTU() <= intf->getMTU()) &&
        (neighborState > Neighbor::ATTEMPT_STATE))
    {
        switch (neighborState) {
            case Neighbor::TWOWAY_STATE:
                break;

            case Neighbor::INIT_STATE:
                neighbor->processEvent(Neighbor::TWOWAY_RECEIVED);
                break;

            case Neighbor::EXCHANGE_START_STATE: {
                OSPFDDOptions& ddOptions = ddPacket->getDdOptions();

                if (ddOptions.I_Init && ddOptions.M_More && ddOptions.MS_MasterSlave &&
                    (ddPacket->getLsaHeadersArraySize() == 0))
                {
                    if (neighbor->getNeighborID() > router->getRouterID()) {
                        Neighbor::DDPacketID packetID;
                        packetID.ddOptions = ddOptions;
                        packetID.options = ddPacket->getOptions();
                        packetID.sequenceNumber = ddPacket->getDdSequenceNumber();

                        neighbor->setOptions(packetID.options);
                        neighbor->setDatabaseExchangeRelationship(Neighbor::SLAVE);
                        neighbor->setDDSequenceNumber(packetID.sequenceNumber);
                        neighbor->setLastReceivedDDPacket(packetID);

                        if (!processDDPacket(ddPacket, intf, neighbor, true)) {
                            break;
                        }

                        neighbor->processEvent(Neighbor::NEGOTIATION_DONE);
                        if (!neighbor->isLinkStateRequestListEmpty() &&
                            !neighbor->isRequestRetransmissionTimerActive())
                        {
                            neighbor->sendLinkStateRequestPacket();
                            neighbor->clearRequestRetransmissionTimer();
                            neighbor->startRequestRetransmissionTimer();
                        }
                    }
                    else {
                        neighbor->sendDatabaseDescriptionPacket(true);
                    }
                }
                if (!ddOptions.I_Init && !ddOptions.MS_MasterSlave &&
                    (ddPacket->getDdSequenceNumber() == neighbor->getDDSequenceNumber()) &&
                    (neighbor->getNeighborID() < router->getRouterID()))
                {
                    Neighbor::DDPacketID packetID;
                    packetID.ddOptions = ddOptions;
                    packetID.options = ddPacket->getOptions();
                    packetID.sequenceNumber = ddPacket->getDdSequenceNumber();

                    neighbor->setOptions(packetID.options);
                    neighbor->setDatabaseExchangeRelationship(Neighbor::MASTER);
                    neighbor->setLastReceivedDDPacket(packetID);

                    if (!processDDPacket(ddPacket, intf, neighbor, true)) {
                        break;
                    }

                    neighbor->processEvent(Neighbor::NEGOTIATION_DONE);
                    if (!neighbor->isLinkStateRequestListEmpty() &&
                        !neighbor->isRequestRetransmissionTimerActive())
                    {
                        neighbor->sendLinkStateRequestPacket();
                        neighbor->clearRequestRetransmissionTimer();
                        neighbor->startRequestRetransmissionTimer();
                    }
                }
            }
            break;

            case Neighbor::EXCHANGE_STATE: {
                Neighbor::DDPacketID packetID;
                packetID.ddOptions = ddPacket->getDdOptions();
                packetID.options = ddPacket->getOptions();
                packetID.sequenceNumber = ddPacket->getDdSequenceNumber();

                if (packetID != neighbor->getLastReceivedDDPacket()) {
                    if ((packetID.ddOptions.MS_MasterSlave &&
                         (neighbor->getDatabaseExchangeRelationship() != Neighbor::SLAVE)) ||
                        (!packetID.ddOptions.MS_MasterSlave &&
                         (neighbor->getDatabaseExchangeRelationship() != Neighbor::MASTER)) ||
                        packetID.ddOptions.I_Init ||
                        (packetID.options != neighbor->getLastReceivedDDPacket().options))
                    {
                        neighbor->processEvent(Neighbor::SEQUENCE_NUMBER_MISMATCH);
                    }
                    else {
                        if (((neighbor->getDatabaseExchangeRelationship() == Neighbor::MASTER) &&
                             (packetID.sequenceNumber == neighbor->getDDSequenceNumber())) ||
                            ((neighbor->getDatabaseExchangeRelationship() == Neighbor::SLAVE) &&
                             (packetID.sequenceNumber == (neighbor->getDDSequenceNumber() + 1))))
                        {
                            neighbor->setLastReceivedDDPacket(packetID);
                            if (!processDDPacket(ddPacket, intf, neighbor, false)) {
                                break;
                            }
                            if (!neighbor->isLinkStateRequestListEmpty() &&
                                !neighbor->isRequestRetransmissionTimerActive())
                            {
                                neighbor->sendLinkStateRequestPacket();
                                neighbor->clearRequestRetransmissionTimer();
                                neighbor->startRequestRetransmissionTimer();
                            }
                        }
                        else {
                            neighbor->processEvent(Neighbor::SEQUENCE_NUMBER_MISMATCH);
                        }
                    }
                }
                else {
                    if (neighbor->getDatabaseExchangeRelationship() == Neighbor::SLAVE) {
                        neighbor->retransmitDatabaseDescriptionPacket();
                    }
                }
            }
            break;

            case Neighbor::LOADING_STATE:
            case Neighbor::FULL_STATE: {
                Neighbor::DDPacketID packetID;
                packetID.ddOptions = ddPacket->getDdOptions();
                packetID.options = ddPacket->getOptions();
                packetID.sequenceNumber = ddPacket->getDdSequenceNumber();

                if ((packetID != neighbor->getLastReceivedDDPacket()) ||
                    (packetID.ddOptions.I_Init))
                {
                    neighbor->processEvent(Neighbor::SEQUENCE_NUMBER_MISMATCH);
                }
                else {
                    if (neighbor->getDatabaseExchangeRelationship() == Neighbor::SLAVE) {
                        if (!neighbor->retransmitDatabaseDescriptionPacket()) {
                            neighbor->processEvent(Neighbor::SEQUENCE_NUMBER_MISMATCH);
                        }
                    }
                }
            }
            break;

            default:
                break;
        }
    }
}

bool DatabaseDescriptionHandler::processDDPacket(OSPFDatabaseDescriptionPacket *ddPacket, Interface *intf, Neighbor *neighbor, bool inExchangeStart)
{
    EV_INFO << "  Processing packet contents(ddOptions="
            << ((ddPacket->getDdOptions().I_Init) ? "I " : "_ ")
            << ((ddPacket->getDdOptions().M_More) ? "M " : "_ ")
            << ((ddPacket->getDdOptions().MS_MasterSlave) ? "MS" : "__")
            << "; seqNumber="
            << ddPacket->getDdSequenceNumber()
            << "):\n";

    unsigned int headerCount = ddPacket->getLsaHeadersArraySize();

    for (unsigned int i = 0; i < headerCount; i++) {
        OSPFLSAHeader& currentHeader = ddPacket->getLsaHeaders(i);
        LSAType lsaType = static_cast<LSAType>(currentHeader.getLsType());

        EV_DETAIL << "    " << currentHeader;

        if ((lsaType < ROUTERLSA_TYPE) || (lsaType > AS_EXTERNAL_LSA_TYPE) ||
            ((lsaType == AS_EXTERNAL_LSA_TYPE) && (!intf->getArea()->getExternalRoutingCapability())))
        {
            EV_ERROR << " Error!\n";
            neighbor->processEvent(Neighbor::SEQUENCE_NUMBER_MISMATCH);
            return false;
        }
        else {
            LSAKeyType lsaKey;

            lsaKey.linkStateID = currentHeader.getLinkStateID();
            lsaKey.advertisingRouter = currentHeader.getAdvertisingRouter();

            OSPFLSA *lsaInDatabase = router->findLSA(lsaType, lsaKey, intf->getArea()->getAreaID());

            // operator< and operator== on OSPFLSAHeaders determines which one is newer(less means older)
            if ((lsaInDatabase == NULL) || (lsaInDatabase->getHeader() < currentHeader)) {
                EV_DETAIL << " (newer)";
                neighbor->addToRequestList(&currentHeader);
            }
        }
        EV_DETAIL << "\n";
    }

    if (neighbor->getDatabaseExchangeRelationship() == Neighbor::MASTER) {
        neighbor->incrementDDSequenceNumber();
        if ((neighbor->getDatabaseSummaryListCount() == 0) && !ddPacket->getDdOptions().M_More) {
            neighbor->processEvent(Neighbor::EXCHANGE_DONE);    // does nothing in ExchangeStart
        }
        else {
            if (!inExchangeStart) {
                neighbor->sendDatabaseDescriptionPacket();
            }
        }
    }
    else {
        neighbor->setDDSequenceNumber(ddPacket->getDdSequenceNumber());
        if (!inExchangeStart) {
            neighbor->sendDatabaseDescriptionPacket();
        }
        if (!ddPacket->getDdOptions().M_More &&
            (neighbor->getDatabaseSummaryListCount() == 0))
        {
            neighbor->processEvent(Neighbor::EXCHANGE_DONE);    // does nothing in ExchangeStart
        }
    }
    return true;
}

} // namespace ospf

} // namespace inet

