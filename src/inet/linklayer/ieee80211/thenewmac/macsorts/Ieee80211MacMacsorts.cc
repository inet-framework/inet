//
// Copyright (C) 2015 OpenSim Ltd.
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

#include "Ieee80211MacMacsorts.h"

namespace inet {
namespace ieee80211 {

Define_Module(Ieee80211MacMacsorts);

simsignal_t Ieee80211MacMacsorts::intraMacRemoteVariablesChanged = cComponent::registerSignal("intraMacRemoteVariablesChanged");

int Ieee80211MacNamedStaticIntDataValues::sMaxMsduLng = 2304;
int Ieee80211MacNamedStaticIntDataValues::sMacHdrLng = 24;
int Ieee80211MacNamedStaticIntDataValues::sWepHdrLng = 28;
int Ieee80211MacNamedStaticIntDataValues::sWepAddLng = 8;
int Ieee80211MacNamedStaticIntDataValues::sWdsAddLng = 6;
int Ieee80211MacNamedStaticIntDataValues::sCrcLng = 4;
int Ieee80211MacNamedStaticIntDataValues::sMaxMpduLng  = (sMaxMsduLng + sMacHdrLng + sWdsAddLng + sWepAddLng + sCrcLng); /* max octets in an MPDU */
int Ieee80211MacNamedStaticIntDataValues::sTsOctet = 24;
int Ieee80211MacNamedStaticIntDataValues::sMinFragLng  = 256;
int Ieee80211MacNamedStaticIntDataValues::sMaxFragNum  = (sMaxMsduLng / (sMinFragLng - sMacHdrLng - sCrcLng)); /* maximum fragment number */
int Ieee80211MacNamedStaticIntDataValues::sAckCtsLng  = 112; /* bits in ACK and CTS frames */


void Ieee80211MacMacsorts::handleMessage(cMessage* msg)
{
    throw cRuntimeError("This module doesn't handle self messages");
}

void Ieee80211MacMacsorts::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        intraMacRemoteVariables = new Ieee80211MacMacsortsIntraMacRemoteVariables(this);
    }
}

void Ieee80211MacMacsorts::emitIntraMacRemoteVariablesChangedSignal()
{
    emit(intraMacRemoteVariablesChanged, true);
}

} /* namespace inet */
} /* namespace ieee80211 */

