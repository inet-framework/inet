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

#include "HelloHandler.h"
#include "IPControlInfo.h"
#include "OSPFRouter.h"
#include "OSPFArea.h"
#include "OSPFInterface.h"
#include "OSPFNeighbor.h"

OSPF::HelloHandler::HelloHandler(OSPF::Router* containingRouter) :
    OSPF::IMessageHandler(containingRouter)
{
}

void OSPF::HelloHandler::ProcessPacket(OSPFPacket* packet, OSPF::Interface* intf, OSPF::Neighbor* unused)
{
    OSPFHelloPacket* helloPacket         = check_and_cast<OSPFHelloPacket*> (packet);
    bool             rebuildRoutingTable = false;

    /* The values of the Network Mask, HelloInterval,
       and RouterDeadInterval fields in the received Hello packet must
       be checked against the values configured for the receiving
       interface.  Any mismatch causes processing to stop and the
       packet to be dropped.
     */
    if ((intf->GetHelloInterval() == helloPacket->getHelloInterval()) &&
        (intf->GetRouterDeadInterval() == helloPacket->getRouterDeadInterval()))
    {
        OSPF::Interface::OSPFInterfaceType interfaceType = intf->GetType();
        /* There is one exception to the above rule: on point-to-point
           networks and on virtual links, the Network Mask in the received
           Hello Packet should be ignored.
         */
        if (!((interfaceType != OSPF::Interface::PointToPoint) &&
              (interfaceType != OSPF::Interface::Virtual) &&
              (intf->GetAddressRange().mask != IPv4AddressFromULong(helloPacket->getNetworkMask().getInt()))
             )
           )
        {
            /* The setting of the E-bit found in the Hello Packet's Options field must match this area's
               ExternalRoutingCapability.
             */
            if (intf->GetArea()->GetExternalRoutingCapability() == helloPacket->getOptions().E_ExternalRoutingCapability) {
                IPControlInfo*      controlInfo             = check_and_cast<IPControlInfo *> (helloPacket->getControlInfo());
                OSPF::IPv4Address   srcAddress              = IPv4AddressFromULong(controlInfo->getSrcAddr().getInt());
                bool                neighborChanged         = false;
                bool                neighborsDRStateChanged = false;
                bool                drChanged               = false;
                bool                backupSeen              = false;
                OSPF::Neighbor*     neighbor;

                /* If the receiving interface connects to a broadcast, Point-to-
                   MultiPoint or NBMA network the source is identified by the IP
                   source address found in the Hello's IP header.
                 */
                if ((interfaceType == OSPF::Interface::Broadcast) ||
                    (interfaceType == OSPF::Interface::PointToMultiPoint) ||
                    (interfaceType == OSPF::Interface::NBMA))
                {
                    neighbor = intf->GetNeighborByAddress(srcAddress);
                } else {
                    /* If the receiving interface connects to a point-to-point link or a virtual link,
                       the source is identified by the Router ID found in the Hello's OSPF packet header.
                     */
                    neighbor = intf->GetNeighborByID(helloPacket->getRouterID().getInt());
                }

                if (neighbor != NULL) {
                    router->GetMessageHandler()->PrintEvent("Hello packet received", intf, neighbor);

                    IPv4Address                 designatedAddress   = neighbor->GetDesignatedRouter().ipInterfaceAddress;
                    IPv4Address                 backupAddress       = neighbor->GetBackupDesignatedRouter().ipInterfaceAddress;
                    char                        newPriority         = helloPacket->getRouterPriority();
                    unsigned long               source              = controlInfo->getSrcAddr().getInt();
                    unsigned long               newDesignatedRouter = helloPacket->getDesignatedRouter().getInt();
                    unsigned long               newBackupRouter     = helloPacket->getBackupDesignatedRouter().getInt();
                    OSPF::DesignatedRouterID    dRouterID;

                    if ((interfaceType == OSPF::Interface::Virtual) &&
                        (neighbor->GetState() == OSPF::Neighbor::DownState))
                    {
                        neighbor->SetPriority(helloPacket->getRouterPriority());
                        neighbor->SetRouterDeadInterval(helloPacket->getRouterDeadInterval());
                    }

                    /* If a change in the neighbor's Router Priority field
                       was noted, the receiving interface's state machine is
                       scheduled with the event NeighborChange.
                     */
                    if (neighbor->GetPriority() != newPriority) {
                        neighborChanged = true;
                    }

                    /* If the neighbor is both declaring itself to be Designated
                       Router(Hello Packet's Designated Router field = Neighbor IP
                       address) and the Backup Designated Router field in the
                       packet is equal to 0.0.0.0 and the receiving interface is in
                       state Waiting, the receiving interface's state machine is
                       scheduled with the event BackupSeen.
                     */
                    if ((newDesignatedRouter == source) &&
                        (newBackupRouter == 0) &&
                        (intf->GetState() == OSPF::Interface::WaitingState))
                    {
                        backupSeen = true;
                    } else {
                        /* Otherwise, if the neighbor is declaring itself to be Designated Router and it
                           had not previously, or the neighbor is not declaring itself
                           Designated Router where it had previously, the receiving
                           interface's state machine is scheduled with the event
                           NeighborChange.
                         */
                        if (((newDesignatedRouter == source) &&
                             (newDesignatedRouter != ULongFromIPv4Address(designatedAddress))) ||
                            ((newDesignatedRouter != source) &&
                             (source == ULongFromIPv4Address(designatedAddress))))
                        {
                            neighborChanged = true;
                            neighborsDRStateChanged = true;
                        }
                    }

                    /* If the neighbor is declaring itself to be Backup Designated
                       Router(Hello Packet's Backup Designated Router field =
                       Neighbor IP address) and the receiving interface is in state
                       Waiting, the receiving interface's state machine is
                       scheduled with the event BackupSeen.
                     */
                    if ((newBackupRouter == source) &&
                        (intf->GetState() == OSPF::Interface::WaitingState))
                    {
                        backupSeen = true;
                    } else {
                        /* Otherwise, if the neighbor is declaring itself to be Backup Designated Router
                           and it had not previously, or the neighbor is not declaring
                           itself Backup Designated Router where it had previously, the
                           receiving interface's state machine is scheduled with the
                           event NeighborChange.
                         */
                        if (((newBackupRouter == source) &&
                             (newBackupRouter != ULongFromIPv4Address(backupAddress))) ||
                            ((newBackupRouter != source) &&
                             (source == ULongFromIPv4Address(backupAddress))))
                        {
                            neighborChanged = true;
                        }
                    }

                    neighbor->SetNeighborID(helloPacket->getRouterID().getInt());
                    neighbor->SetPriority(newPriority);
                    neighbor->SetAddress(srcAddress);
                    dRouterID.routerID = newDesignatedRouter;
                    dRouterID.ipInterfaceAddress = IPv4AddressFromULong(newDesignatedRouter);
                    if (newDesignatedRouter != ULongFromIPv4Address(designatedAddress)) {
                        designatedAddress = dRouterID.ipInterfaceAddress;
                        drChanged = true;
                    }
                    neighbor->SetDesignatedRouter(dRouterID);
                    dRouterID.routerID = newBackupRouter;
                    dRouterID.ipInterfaceAddress = IPv4AddressFromULong(newBackupRouter);
                    if (newBackupRouter != ULongFromIPv4Address(backupAddress)) {
                        backupAddress = dRouterID.ipInterfaceAddress;
                        drChanged = true;
                    }
                    neighbor->SetBackupDesignatedRouter(dRouterID);
                    if (drChanged) {
                        neighbor->SetUpDesignatedRouters(false);
                    }

                    /* If the neighbor router's Designated or Backup Designated Router
                       has changed it's necessary to look up the Router IDs belonging to the
                       new addresses.
                     */
                    if (!neighbor->DesignatedRoutersAreSetUp()) {
                        OSPF::Neighbor* designated = intf->GetNeighborByAddress(designatedAddress);
                        OSPF::Neighbor* backup     = intf->GetNeighborByAddress(backupAddress);

                        if (designated != NULL) {
                            dRouterID.routerID = designated->GetNeighborID();
                            dRouterID.ipInterfaceAddress = designated->GetAddress();
                            neighbor->SetDesignatedRouter(dRouterID);
                        }
                        if (backup != NULL) {
                            dRouterID.routerID = backup->GetNeighborID();
                            dRouterID.ipInterfaceAddress = backup->GetAddress();
                            neighbor->SetBackupDesignatedRouter(dRouterID);
                        }
                        if ((designated != NULL) && (backup != NULL)) {
                            neighbor->SetUpDesignatedRouters(true);
                        }
                    }
                } else {
                    OSPF::DesignatedRouterID    dRouterID;
                    bool                        designatedSetUp = false;
                    bool                        backupSetUp     = false;

                    neighbor = new OSPF::Neighbor(helloPacket->getRouterID().getInt());
                    neighbor->SetPriority(helloPacket->getRouterPriority());
                    neighbor->SetAddress(srcAddress);
                    neighbor->SetRouterDeadInterval(helloPacket->getRouterDeadInterval());

                    router->GetMessageHandler()->PrintEvent("Hello packet received", intf, neighbor);

                    dRouterID.routerID = helloPacket->getDesignatedRouter().getInt();
                    dRouterID.ipInterfaceAddress = IPv4AddressFromULong(dRouterID.routerID);

                    OSPF::Neighbor* designated = intf->GetNeighborByAddress(dRouterID.ipInterfaceAddress);

                    // Get the Designated Router ID from the corresponding Neighbor Object.
                    if (designated != NULL) {
                        if (designated->GetNeighborID() != dRouterID.routerID) {
                            dRouterID.routerID = designated->GetNeighborID();
                        }
                        designatedSetUp = true;
                    }
                    neighbor->SetDesignatedRouter(dRouterID);

                    dRouterID.routerID = helloPacket->getBackupDesignatedRouter().getInt();
                    dRouterID.ipInterfaceAddress = IPv4AddressFromULong(dRouterID.routerID);

                    OSPF::Neighbor* backup = intf->GetNeighborByAddress(dRouterID.ipInterfaceAddress);

                    // Get the Backup Designated Router ID from the corresponding Neighbor Object.
                    if (backup != NULL) {
                        if (backup->GetNeighborID() != dRouterID.routerID) {
                            dRouterID.routerID = backup->GetNeighborID();
                        }
                        backupSetUp = true;
                    }
                    neighbor->SetBackupDesignatedRouter(dRouterID);
                    if (designatedSetUp && backupSetUp) {
                        neighbor->SetUpDesignatedRouters(true);
                    }
                    intf->AddNeighbor(neighbor);
                }

                neighbor->ProcessEvent(OSPF::Neighbor::HelloReceived);
                if ((interfaceType == OSPF::Interface::NBMA) &&
                    (intf->GetRouterPriority() == 0) &&
                    (neighbor->GetState() >= OSPF::Neighbor::InitState))
                {
                    intf->SendHelloPacket(neighbor->GetAddress());
                }

                unsigned long   interfaceAddress       = ULongFromIPv4Address(intf->GetAddressRange().address);
                unsigned int    neighborsNeighborCount = helloPacket->getNeighborArraySize();
                unsigned int    i;
                /* The list of neighbors contained in the Hello Packet is
                   examined.  If the router itself appears in this list, the
                   neighbor state machine should be executed with the event TwoWayReceived.
                 */
                for (i = 0; i < neighborsNeighborCount; i++) {
                    if (helloPacket->getNeighbor(i).getInt() == interfaceAddress) {
                        neighbor->ProcessEvent(OSPF::Neighbor::TwoWayReceived);
                        break;
                    }
                }
                /* Otherwise, the neighbor state machine should
                   be executed with the event OneWayReceived, and the processing
                   of the packet stops.
                 */
                if (i == neighborsNeighborCount) {
                    neighbor->ProcessEvent(OSPF::Neighbor::OneWayReceived);
                }

                if (neighborChanged) {
                    intf->ProcessEvent(OSPF::Interface::NeighborChange);
                    /* In some cases neighbors get stuck in TwoWay state after a DR
                       or Backup change. (CalculateDesignatedRouter runs before the
                       neighbors' signal of DR change + this router does not become
                       neither DR nor backup -> IsAdjacencyOK does not get called.)
                       So to make it work(workaround) we'll call IsAdjacencyOK for
                       all neighbors in TwoWay state from here. This shouldn't break
                       anything because if the neighbor state doesn't have to change
                       then NeedAdjacency returns false and nothing happnes in
                       IsAdjacencyOK.
                     */
                    unsigned int neighborCount = intf->GetNeighborCount();
                    for (i = 0; i < neighborCount; i++) {
                        OSPF::Neighbor* stuckNeighbor = intf->GetNeighbor(i);
                        if (stuckNeighbor->GetState() == OSPF::Neighbor::TwoWayState) {
                            stuckNeighbor->ProcessEvent(OSPF::Neighbor::IsAdjacencyOK);
                        }
                    }

                    if (neighborsDRStateChanged) {
                        OSPF::RouterLSA* routerLSA = intf->GetArea()->FindRouterLSA(router->GetRouterID());

                        if (routerLSA != NULL) {
                            long sequenceNumber = routerLSA->getHeader().getLsSequenceNumber();
                            if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                                routerLSA->getHeader().setLsAge(MAX_AGE);
                                intf->GetArea()->FloodLSA(routerLSA);
                                routerLSA->IncrementInstallTime();
                            } else {
                                OSPF::RouterLSA* newLSA = intf->GetArea()->OriginateRouterLSA();

                                newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
                                newLSA->getHeader().setLsChecksum(0);    // TODO: calculate correct LS checksum
                                rebuildRoutingTable |= routerLSA->Update(newLSA);
                                delete newLSA;

                                intf->GetArea()->FloodLSA(routerLSA);
                            }
                        }
                    }
                }

                if (backupSeen) {
                    intf->ProcessEvent(OSPF::Interface::BackupSeen);
                }
            }
        }
    }

    if (rebuildRoutingTable) {
        router->RebuildRoutingTable();
    }
}
