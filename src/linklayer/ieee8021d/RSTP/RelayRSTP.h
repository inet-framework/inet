//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __INET_RELAYRSTP_H_
#define __INET_RELAYRSTP_H_

#include <omnetpp.h>

#include "MACAddress.h"
#include "EtherFrame.h"
#include "BPDU.h"
#include "RSTP.h"
#include "MACAddressTable.h"
#include "Delivery_m.h"

class RelayRSTP : public cSimpleModule
{
  protected:
    MACAddressTable *AddressTable;   /// SwTable module pointer. Updated every time the admcarelay module is accessed.
    RSTP * rstpModule; /// RSTP module pointer.
    bool verbose;  /// It sets module verbosity
  protected:
    MACAddress address;
    virtual void initialize(int stage);
    virtual int numInitStages() const {return 3;}
    virtual void handleMessage(cMessage *msg);
    virtual void handleIncomingFrame(BPDUieee8021D *frame);             /// BPDU handler. Delivers BPDUs to every Designated port or to the RSTP module.
    virtual void handleIncomingFrame(Delivery *frame);
    virtual void handleIncomingFrame(cMessage *msg);
    virtual void handleEtherFrame(EtherFrame *frame);
    virtual void relayMsg(cMessage * msg,int outputPort);
    virtual void broadcastMsg(cMessage * msg);
};

#endif
