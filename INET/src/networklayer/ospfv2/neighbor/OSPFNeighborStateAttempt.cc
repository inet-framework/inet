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

#include "OSPFNeighborStateAttempt.h"
#include "OSPFNeighborStateDown.h"
#include "OSPFNeighborStateInit.h"
#include "MessageHandler.h"
#include "OSPFInterface.h"
#include "OSPFArea.h"
#include "OSPFRouter.h"

void OSPF::NeighborStateAttempt::ProcessEvent(OSPF::Neighbor* neighbor, OSPF::Neighbor::NeighborEventType event)
{
    if ((event == OSPF::Neighbor::KillNeighbor) || (event == OSPF::Neighbor::LinkDown)) {
        MessageHandler* messageHandler = neighbor->GetInterface()->GetArea()->GetRouter()->GetMessageHandler();
        neighbor->Reset();
        messageHandler->ClearTimer(neighbor->GetInactivityTimer());
        ChangeState(neighbor, new OSPF::NeighborStateDown, this);
    }
    if (event == OSPF::Neighbor::InactivityTimer) {
        neighbor->Reset();
        if (neighbor->GetInterface()->GetType() == OSPF::Interface::NBMA) {
            MessageHandler* messageHandler = neighbor->GetInterface()->GetArea()->GetRouter()->GetMessageHandler();
            messageHandler->StartTimer(neighbor->GetPollTimer(), neighbor->GetInterface()->GetPollInterval());
        }
        ChangeState(neighbor, new OSPF::NeighborStateDown, this);
    }
    if (event == OSPF::Neighbor::HelloReceived) {
        MessageHandler* messageHandler = neighbor->GetInterface()->GetArea()->GetRouter()->GetMessageHandler();
        messageHandler->ClearTimer(neighbor->GetInactivityTimer());
        messageHandler->StartTimer(neighbor->GetInactivityTimer(), neighbor->GetRouterDeadInterval());
        ChangeState(neighbor, new OSPF::NeighborStateInit, this);
    }
}
