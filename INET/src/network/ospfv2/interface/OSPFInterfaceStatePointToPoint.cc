#include "OSPFInterfaceStatePointToPoint.h"
#include "OSPFInterfaceStateDown.h"
#include "OSPFInterfaceStateLoopback.h"
#include "OSPFArea.h"
#include "OSPFRouter.h"
#include "MessageHandler.h"

void OSPF::InterfaceStatePointToPoint::ProcessEvent (OSPF::Interface* intf, OSPF::Interface::InterfaceEventType event)
{
    if (event == OSPF::Interface::InterfaceDown) {
        intf->Reset ();
        ChangeState (intf, new OSPF::InterfaceStateDown, this);
    }
    if (event == OSPF::Interface::LoopIndication) {
        intf->Reset ();
        ChangeState (intf, new OSPF::InterfaceStateLoopback, this);
    }
    if (event == OSPF::Interface::HelloTimer) {
        if (intf->GetType () == OSPF::Interface::Virtual) {
            if (intf->GetNeighborCount () > 0) {
                intf->SendHelloPacket (intf->GetNeighbor (0)->GetAddress (), VIRTUAL_LINK_TTL);
            }
        } else {
            intf->SendHelloPacket (OSPF::AllSPFRouters);
        }
        intf->GetArea ()->GetRouter ()->GetMessageHandler ()->StartTimer (intf->GetHelloTimer (), intf->GetHelloInterval ());
    }
    if (event == OSPF::Interface::AcknowledgementTimer) {
        intf->SendDelayedAcknowledgements ();
    }
}

