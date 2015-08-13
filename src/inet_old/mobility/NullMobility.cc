//
// Copyright (C) 2006 Andras Varga
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


#include "inet_old/mobility/NullMobility.h"


#define coreEV (ev.isDisabled()||!coreDebug) ? (std::ostream&)ev : EV << logName() << "::BasicMobility: "

namespace inet {

Define_Module(NullMobility);

void NullMobility::handleSelfMsg(cMessage *msg)
{
    ASSERT(false);
}

} // namespace inet

