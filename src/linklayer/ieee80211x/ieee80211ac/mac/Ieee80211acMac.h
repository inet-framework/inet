//
// Copyright (C) 2014 Andrea Tino
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

//
// Ieee80211acMac.h
//

#ifndef IEEE80211ACMAC_H_
#define IEEE80211ACMAC_H_

#include "linklayer/common/WirelessMacBase.h"
#include "INETDefs.h"

/**
 * Class for MAC layer of IEEE 802.11ac protocol.
 *
 * @author Andrea Tino
 */
class INET_API Ieee80211acMac : public WirelessMacBase {
public:
  Ieee80211acMac();

  virtual ~Ieee80211acMac();

}; /* Ieee80211acMac */

#endif

