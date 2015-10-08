//
// Copyright (C) 2015 Andras Varga
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

#include "RandomQoSClassifier.h"
#include "Ieee802Ctrl.h"

namespace inet {

Define_Module(RandomQoSClassifier);

void RandomQoSClassifier::handleMessage(cMessage *msg)
{
    int userPriority = intrand(8);
    Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl*>(msg->getControlInfo());
    ctrl->setUserPriority(userPriority);
    send(msg, "out");
}

} // namespace inet

