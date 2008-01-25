#include "OSPFInterfaceStateDesignatedRouter.h"
#include "OSPFInterfaceStateDown.h"
#include "OSPFInterfaceStateLoopback.h"
#include "OSPFArea.h"
#include "OSPFRouter.h"
#include "MessageHandler.h"

void OSPF::InterfaceStateDesignatedRouter::ProcessEvent (OSPF::Interface* intf, OSPF::Interface::InterfaceEventType event)
{
    if (event == OSPF::Interface::NeighborChange) {
        CalculateDesignatedRouter (intf);
    }
    if (event == OSPF::Interface::InterfaceDown) {
        intf->Reset ();
        ChangeState (intf, new OSPF::InterfaceStateDown, this);
    }
    if (event == OSPF::Interface::LoopIndication) {
        intf->Reset ();
        ChangeState (intf, new OSPF::InterfaceStateLoopback, this);
    }
    if (event == OSPF::Interface::HelloTimer) {
        if (intf->GetType () == OSPF::Interface::Broadcast) {
            intf->SendHelloPacket (OSPF::AllSPFRouters);
        } else {    // OSPF::Interface::NBMA
            unsigned long neighborCount = intf->GetNeighborCount ();
            int           ttl           = (intf->GetType () == OSPF::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;
            for (unsigned long i = 0; i < neighborCount; i++) {
                intf->SendHelloPacket (intf->GetNeighbor (i)->GetAddress (), ttl);
            }
        }
        intf->GetArea ()->GetRouter ()->GetMessageHandler ()->StartTimer (intf->GetHelloTimer (), intf->GetHelloInterval ());
    }
    if (event == OSPF::Interface::AcknowledgementTimer) {
        intf->SendDelayedAcknowledgements ();
    }
}

