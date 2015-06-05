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

#ifndef __INET_IEEE80211MACMSDUFROMLLC_H
#define __INET_IEEE80211MACMSDUFROMLLC_H

#include "inet/common/INETDefs.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/ieee80211/thenewmac/macsorts/Ieee80211MacMacsorts.h"
#include "inet/linklayer/ieee80211/thenewmac/base/Ieee80211MacMacProcessBase.h"
#include "inet/linklayer/ieee80211/thenewmac/macmib/Ieee80211MacMacmib.h"

namespace inet {
namespace ieee80211 {

class INET_API IIeee80211MacMsduFromLlc
{
    protected:
        virtual void handleMaUnitDataRequest() = 0;
        virtual void handleMsduConfirm() = 0;
};

class INET_API Ieee80211MacMsduFromLlc : public IIeee80211MacMsduFromLlc, public Ieee80211MacMacProcessBase
{
    protected:
        Ieee80211MacMacsorts *macsorts = nullptr;
        Ieee80211MacMacmibPackage *macmib = nullptr;

    protected:
        void handleMaUnitDataRequest();
        void handleMsduConfirm();
};

} /* namespace inet */
} /* namespace ieee80211 */

#endif // ifndef __INET_IEEE80211MACMSDUFROMLLC_H
