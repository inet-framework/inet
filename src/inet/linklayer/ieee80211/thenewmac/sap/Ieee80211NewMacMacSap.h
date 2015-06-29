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

#ifndef __INET_IEEE80211NEWMACMACSAP_H
#define __INET_IEEE80211NEWMACMACSAP_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/common/ModuleAccess.h"

namespace inet {
namespace ieee80211 {

class INET_API Ieee80211NewMacMacSap : public cSimpleModule
{
    protected:
        MACAddress address;
        InterfaceEntry *interfaceEntry;

    protected:
        virtual void handleMessage(cMessage *msg) override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        void initialize(int stage);
        void registerInterface();
        InterfaceEntry *createInterfaceEntry();
};

} /* namespace inet */
} /* namespace ieee80211 */

#endif // ifndef __INET_IEEE80211NEWMACMACSAP_H
