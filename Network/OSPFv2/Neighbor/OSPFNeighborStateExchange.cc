#include "OSPFNeighborStateExchange.h"
#include "OSPFNeighborStateDown.h"
#include "OSPFNeighborStateInit.h"
#include "OSPFNeighborStateTwoWay.h"
#include "OSPFNeighborStateExchangeStart.h"
#include "OSPFNeighborStateFull.h"
#include "OSPFNeighborStateLoading.h"
#include "MessageHandler.h"
#include "OSPFInterface.h"
#include "OSPFArea.h"
#include "OSPFRouter.h"

void OSPF::NeighborStateExchange::ProcessEvent (OSPF::Neighbor* neighbor, OSPF::Neighbor::NeighborEventType event)
{
    if ((event == OSPF::Neighbor::KillNeighbor) || (event == OSPF::Neighbor::LinkDown)) {
        MessageHandler* messageHandler = neighbor->GetInterface ()->GetArea ()->GetRouter ()->GetMessageHandler ();
        neighbor->Reset ();
        messageHandler->ClearTimer (neighbor->GetInactivityTimer ());
        ChangeState (neighbor, new OSPF::NeighborStateDown, this);
    }
    if (event == OSPF::Neighbor::InactivityTimer) {
        neighbor->Reset ();
        if (neighbor->GetInterface ()->GetType () == OSPF::Interface::NBMA) {
            MessageHandler* messageHandler = neighbor->GetInterface ()->GetArea ()->GetRouter ()->GetMessageHandler ();
            messageHandler->StartTimer (neighbor->GetPollTimer (), neighbor->GetInterface ()->GetPollInterval ());
        }
        ChangeState (neighbor, new OSPF::NeighborStateDown, this);
    }
    if (event == OSPF::Neighbor::OneWayReceived) {
        neighbor->Reset ();
        ChangeState (neighbor, new OSPF::NeighborStateInit, this);
    }
    if (event == OSPF::Neighbor::HelloReceived) {
        MessageHandler* messageHandler = neighbor->GetInterface ()->GetArea ()->GetRouter ()->GetMessageHandler ();
        messageHandler->ClearTimer (neighbor->GetInactivityTimer ());
        messageHandler->StartTimer (neighbor->GetInactivityTimer (), neighbor->GetRouterDeadInterval ());
    }
    if (event == OSPF::Neighbor::IsAdjacencyOK) {
        if (!neighbor->NeedAdjacency ()) {
            neighbor->Reset ();
            ChangeState (neighbor, new OSPF::NeighborStateTwoWay, this);
        }
    }
    if ((event == OSPF::Neighbor::SequenceNumberMismatch) || (event == OSPF::Neighbor::BadLinkStateRequest)) {
        MessageHandler* messageHandler = neighbor->GetInterface ()->GetArea ()->GetRouter ()->GetMessageHandler ();
        neighbor->Reset ();
        neighbor->IncrementDDSequenceNumber ();
        neighbor->SendDatabaseDescriptionPacket (true);
        messageHandler->StartTimer (neighbor->GetDDRetransmissionTimer (), neighbor->GetInterface ()->GetRetransmissionInterval ());
        ChangeState (neighbor, new OSPF::NeighborStateExchangeStart, this);
    }
    if (event == OSPF::Neighbor::ExchangeDone) {
        if (neighbor->IsLinkStateRequestListEmpty ()) {
            MessageHandler* messageHandler = neighbor->GetInterface ()->GetArea ()->GetRouter ()->GetMessageHandler ();
            messageHandler->StartTimer (neighbor->GetDDRetransmissionTimer (), neighbor->GetRouterDeadInterval ());
            neighbor->ClearRequestRetransmissionTimer ();
            ChangeState (neighbor, new OSPF::NeighborStateFull, this);
        } else {
            MessageHandler* messageHandler = neighbor->GetInterface ()->GetArea ()->GetRouter ()->GetMessageHandler ();
            messageHandler->StartTimer (neighbor->GetDDRetransmissionTimer (), neighbor->GetRouterDeadInterval ());
            ChangeState (neighbor, new OSPF::NeighborStateLoading, this);
        }
    }
    if (event == OSPF::Neighbor::UpdateRetransmissionTimer) {
        neighbor->RetransmitUpdatePacket ();
        neighbor->StartUpdateRetransmissionTimer ();
    }
    if (event == OSPF::Neighbor::RequestRetransmissionTimer) {
        neighbor->SendLinkStateRequestPacket ();
        neighbor->StartRequestRetransmissionTimer ();
    }
}
