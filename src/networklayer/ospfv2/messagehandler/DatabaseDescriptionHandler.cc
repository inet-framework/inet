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


#include "DatabaseDescriptionHandler.h"

#include "OSPFArea.h"
#include "OSPFInterface.h"
#include "OSPFNeighbor.h"
#include "OSPFRouter.h"


OSPF::DatabaseDescriptionHandler::DatabaseDescriptionHandler(OSPF::Router* containingRouter) :
    OSPF::IMessageHandler(containingRouter)
{
}

void OSPF::DatabaseDescriptionHandler::processPacket(OSPFPacket* packet, OSPF::Interface* intf, OSPF::Neighbor* neighbor)
{
    router->getMessageHandler()->printEvent("Database Description packet received", intf, neighbor);

    OSPFDatabaseDescriptionPacket* ddPacket = check_and_cast<OSPFDatabaseDescriptionPacket*> (packet);

    OSPF::Neighbor::NeighborStateType neighborState = neighbor->getState();

    if ((ddPacket->getInterfaceMTU() <= intf->getMTU()) &&
        (neighborState > OSPF::Neighbor::ATTEMPT_STATE))
    {
        switch (neighborState) {
            case OSPF::Neighbor::TWOWAY_STATE:
                break;
            case OSPF::Neighbor::INIT_STATE:
                neighbor->processEvent(OSPF::Neighbor::TWOWAY_RECEIVED);
                break;
            case OSPF::Neighbor::EXCHANGE_START_STATE:
                {
                    OSPFDDOptions& ddOptions = ddPacket->getDdOptions();

                    if (ddOptions.I_Init && ddOptions.M_More && ddOptions.MS_MasterSlave &&
                        (ddPacket->getLsaHeadersArraySize() == 0))
                    {
                        if (neighbor->getNeighborID() > router->getRouterID()) {
                            OSPF::Neighbor::DDPacketID packetID;
                            packetID.ddOptions = ddOptions;
                            packetID.options = ddPacket->getOptions();
                            packetID.sequenceNumber = ddPacket->getDdSequenceNumber();

                            neighbor->setOptions(packetID.options);
                            neighbor->setDatabaseExchangeRelationship(OSPF::Neighbor::SLAVE);
                            neighbor->setDDSequenceNumber(packetID.sequenceNumber);
                            neighbor->setLastReceivedDDPacket(packetID);

                            if (!processDDPacket(ddPacket, intf, neighbor, true)) {
                                break;
                            }

                            neighbor->processEvent(OSPF::Neighbor::NEGOTIATION_DONE);
                            if (!neighbor->isLinkStateRequestListEmpty() &&
                                !neighbor->isRequestRetransmissionTimerActive())
                            {
                                neighbor->sendLinkStateRequestPacket();
                                neighbor->clearRequestRetransmissionTimer();
                                neighbor->startRequestRetransmissionTimer();
                            }
                        } else {
                            neighbor->sendDatabaseDescriptionPacket(true);
                        }
                    }
                    if (!ddOptions.I_Init && !ddOptions.MS_MasterSlave &&
                        (ddPacket->getDdSequenceNumber() == neighbor->getDDSequenceNumber()) &&
                        (neighbor->getNeighborID() < router->getRouterID()))
                    {
                        OSPF::Neighbor::DDPacketID packetID;
                        packetID.ddOptions = ddOptions;
                        packetID.options = ddPacket->getOptions();
                        packetID.sequenceNumber = ddPacket->getDdSequenceNumber();

                        neighbor->setOptions(packetID.options);
                        neighbor->setDatabaseExchangeRelationship(OSPF::Neighbor::MASTER);
                        neighbor->setLastReceivedDDPacket(packetID);

                        if (!processDDPacket(ddPacket, intf, neighbor, true)) {
                            break;
                        }

                        neighbor->processEvent(OSPF::Neighbor::NEGOTIATION_DONE);
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
            case OSPF::Neighbor::EXCHANGE_STATE:
                {
                    OSPF::Neighbor::DDPacketID packetID;
                    packetID.ddOptions = ddPacket->getDdOptions();
                    packetID.options = ddPacket->getOptions();
                    packetID.sequenceNumber = ddPacket->getDdSequenceNumber();

                    if (packetID != neighbor->getLastReceivedDDPacket()) {
                        if ((packetID.ddOptions.MS_MasterSlave &&
                             (neighbor->getDatabaseExchangeRelationship() != OSPF::Neighbor::SLAVE)) ||
                            (!packetID.ddOptions.MS_MasterSlave &&
                             (neighbor->getDatabaseExchangeRelationship() != OSPF::Neighbor::MASTER)) ||
                            packetID.ddOptions.I_Init ||
                            (packetID.options != neighbor->getLastReceivedDDPacket().options))
                        {
                            neighbor->processEvent(OSPF::Neighbor::SEQUENCE_NUMBER_MISMATCH);
                        } else {
                            if (((neighbor->getDatabaseExchangeRelationship() == OSPF::Neighbor::MASTER) &&
                                 (packetID.sequenceNumber == neighbor->getDDSequenceNumber())) ||
                                ((neighbor->getDatabaseExchangeRelationship() == OSPF::Neighbor::SLAVE) &&
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
                            } else {
                                neighbor->processEvent(OSPF::Neighbor::SEQUENCE_NUMBER_MISMATCH);
                            }
                        }
                    } else {
                        if (neighbor->getDatabaseExchangeRelationship() == OSPF::Neighbor::SLAVE) {
                            neighbor->retransmitDatabaseDescriptionPacket();
                        }
                    }
                }
                break;
            case OSPF::Neighbor::LOADING_STATE:
            case OSPF::Neighbor::FULL_STATE:
                {
                    OSPF::Neighbor::DDPacketID packetID;
                    packetID.ddOptions = ddPacket->getDdOptions();
                    packetID.options = ddPacket->getOptions();
                    packetID.sequenceNumber = ddPacket->getDdSequenceNumber();

                    if ((packetID != neighbor->getLastReceivedDDPacket()) ||
                        (packetID.ddOptions.I_Init))
                    {
                        neighbor->processEvent(OSPF::Neighbor::SEQUENCE_NUMBER_MISMATCH);
                    } else {
                        if (neighbor->getDatabaseExchangeRelationship() == OSPF::Neighbor::SLAVE) {
                            if (!neighbor->retransmitDatabaseDescriptionPacket()) {
                                neighbor->processEvent(OSPF::Neighbor::SEQUENCE_NUMBER_MISMATCH);
                            }
                        }
                    }
                }
                break;
            default: break;
        }
    }
}

bool OSPF::DatabaseDescriptionHandler::processDDPacket(OSPFDatabaseDescriptionPacket* ddPacket, OSPF::Interface* intf, OSPF::Neighbor* neighbor, bool inExchangeStart)
{
    EV << "  Processing packet contents(ddOptions="
       << ((ddPacket->getDdOptions().I_Init) ? "I " : "_ ")
       << ((ddPacket->getDdOptions().M_More) ? "M " : "_ ")
       << ((ddPacket->getDdOptions().MS_MasterSlave) ? "MS" : "__")
       << "; seqNumber="
       << ddPacket->getDdSequenceNumber()
       << "):\n";

    unsigned int headerCount = ddPacket->getLsaHeadersArraySize();

    for (unsigned int i = 0; i < headerCount; i++) {
        OSPFLSAHeader& currentHeader = ddPacket->getLsaHeaders(i);
        LSAType lsaType = static_cast<LSAType> (currentHeader.getLsType());

        EV << "    ";
        printLSAHeader(currentHeader, ev.getOStream());

        if ((lsaType < ROUTERLSA_TYPE) || (lsaType > AS_EXTERNAL_LSA_TYPE) ||
            ((lsaType == AS_EXTERNAL_LSA_TYPE) && (!intf->getArea()->getExternalRoutingCapability())))
        {
            EV << " Error!\n";
            neighbor->processEvent(OSPF::Neighbor::SEQUENCE_NUMBER_MISMATCH);
            return false;
        } else {
            OSPF::LSAKeyType lsaKey;

            lsaKey.linkStateID = currentHeader.getLinkStateID();
            lsaKey.advertisingRouter = currentHeader.getAdvertisingRouter();

            OSPFLSA* lsaInDatabase = router->findLSA(lsaType, lsaKey, intf->getArea()->getAreaID());

            // operator< and operator== on OSPFLSAHeaders determines which one is newer(less means older)
            if ((lsaInDatabase == NULL) || (lsaInDatabase->getHeader() < currentHeader)) {
                EV << " (newer)";
                neighbor->addToRequestList(&currentHeader);
            }
        }
        EV << "\n";
    }

    if (neighbor->getDatabaseExchangeRelationship() == OSPF::Neighbor::MASTER) {
        neighbor->incrementDDSequenceNumber();
        if ((neighbor->getDatabaseSummaryListCount() == 0) && !ddPacket->getDdOptions().M_More) {
            neighbor->processEvent(OSPF::Neighbor::EXCHANGE_DONE);  // does nothing in ExchangeStart
        } else {
            if (!inExchangeStart) {
                neighbor->sendDatabaseDescriptionPacket();
            }
        }
    } else {
        neighbor->setDDSequenceNumber(ddPacket->getDdSequenceNumber());
        if (!inExchangeStart) {
            neighbor->sendDatabaseDescriptionPacket();
        }
        if (!ddPacket->getDdOptions().M_More &&
            (neighbor->getDatabaseSummaryListCount() == 0))
        {
            neighbor->processEvent(OSPF::Neighbor::EXCHANGE_DONE);  // does nothing in ExchangeStart
        }
    }
    return true;
}
