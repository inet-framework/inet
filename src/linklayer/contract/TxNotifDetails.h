//
// Copyright (C) 2005 Andras Varga
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


#ifndef __INET_TXNOTIFDETAILS_H
#define __INET_TXNOTIFDETAILS_H

#include "INETDefs.h"


// Forward declarations:
class InterfaceEntry;


/**
 * Details class for the NF_PP_TX_BEGIN, NF_PP_TX_END and NF_PP_RX_END
 * notifications (normally triggered from PPP).
 *
 * @see NotificationBoard
 */
//XXX also used by Ieee80211 to signal that a msg has been acked (must use an ID to identify msg!!!), and that channel was switched (msg==NULL then)
class TxNotifDetails : public cObject
{
  protected:
    cPacket *msg;
    InterfaceEntry *ie;

  public:
    TxNotifDetails() {msg = NULL; ie = NULL;}

    cPacket *getPacket() const {return msg;}
    InterfaceEntry *getInterfaceEntry() const {return ie;}
    void setPacket(cPacket *m) {msg = m;}
    void setInterfaceEntry(InterfaceEntry *e) {ie = e;}
};

#endif

