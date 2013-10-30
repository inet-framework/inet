//
// Copyright (C) 2011 Juan Luis Garrote Molinero
// Copyright (C) 2013 Zsolt Prontvai
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

#ifndef __INET_RELAYRSTP_H_
#define __INET_RELAYRSTP_H_

#include <omnetpp.h>

#include "ILifecycle.h"
#include "MACAddress.h"
#include "EtherFrame.h"
#include "IEEE8021DBPDU_m.h"
#include "RSTP.h"
#include "MACAddressTable.h"
#include "InterfaceTable.h"
#include "IEEE8021DInterfaceData.h"

class RelayRSTP : public cSimpleModule, public ILifecycle
{
  protected:
    MACAddressTable *AddressTable;   /// SwTable module pointer. Updated every time the admcarelay module is accessed.
    RSTP * rstpModule; /// RSTP module pointer.
    bool verbose;  /// It sets module verbosity
    bool isOperational;         // for lifecycle
    IInterfaceTable * ifTable;
  protected:
    MACAddress address;
    virtual void initialize(int stage);
    virtual int numInitStages() const {return 3;}
    virtual void handleMessage(cMessage *msg);
    virtual void handleBPDUFrame(EtherFrame *frame);             /// BPDU handler. Delivers BPDUs to every Designated port or to the RSTP module.
    virtual void handleFrameFromRSTP(BPDU *frame);
    virtual void handleEtherFrame(EtherFrame *frame);
    virtual void relayMsg(cMessage * msg,int outputPort);
    virtual void broadcastMsg(cMessage * msg);

  // for lifecycle:
  public:
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);
  protected:
    virtual void start();
    virtual void stop();
    IEEE8021DInterfaceData * getPortInterfaceData(unsigned int portNum);
};

#endif
