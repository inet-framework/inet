//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#ifndef __TXNOTIFDETAILS_H
#define __TXNOTIFDETAILS_H

#include "INETDefs.h"

/**
 * Details class for the NF_PP_TX_BEGIN, NF_PP_TX_END and NF_PP_RX_END
 * notifications (normally triggered from PPP).
 *
 * @see NotificationBoard
 */
//XXX also used by Ieee80211 to signal that a msg has been acked (must use an ID to identify msg!!!), and that channel was switched (msg==NULL then) 
class TxNotifDetails : public cPolymorphic
{
  protected:
    cMessage *msg;
    InterfaceEntry *ie;

  public:
    TxNotifDetails() {msg=NULL; ie=NULL;}

    cMessage *message() const {return msg;}
    InterfaceEntry *interfaceEntry() const {return ie;}
    void setMessage(cMessage *m) {msg = m;}
    void setInterfaceEntry(InterfaceEntry *e) {ie = e;}
};

#endif

