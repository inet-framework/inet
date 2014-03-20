// Copyright (C) 2013 OpenSim Ltd.
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
// Author: Benjamin Martin Seregi

#ifndef IEEE8021DRelay_H_
#define IEEE8021DRelay_H_

#include "INETDefs.h"
#include "InterfaceTable.h"
#include "IMACAddressTable.h"
#include "EtherFrame_m.h"
#include "NodeOperations.h"
#include "NodeStatus.h"
#include "Ieee8021dBPDU_m.h"

//
// This module forward frames (~EtherFrame) based on their destination MAC addresses to appropriate ports.
// See the NED definition for details.
//
class Ieee8021dRelay : public cSimpleModule, public ILifecycle
{
    public:
        Ieee8021dRelay();

    protected:
        MACAddress bridgeAddress;
        IInterfaceTable * ifTable;
        IMACAddressTable * macTable;
        InterfaceEntry * ie;
        bool isOperational;
        bool isStpAware;
        unsigned int portCount; // number of ports in the switch

        // statistics: see finish() for details.
        int numReceivedNetworkFrames;
        int numDroppedFrames;
        int numReceivedBPDUsFromSTP;
        int numDeliveredBDPUsToSTP;
        int numDispatchedNonBPDUFrames;
        int numDispatchedBDPUFrames;

        virtual void initialize(int stage);
        virtual int numInitStages() const { return NUM_INIT_STAGES; }
        virtual void handleMessage(cMessage * msg);

        /**
         * Updates address table (if the port is in learning state)
         * with source address, determines output port
         * and sends out (or broadcasts) frame on ports
         * (if the ports are in forwarding state).
         * Includes calls to updateTableWithAddress() and getPortForAddress().
         *
         */
        void handleAndDispatchFrame(EtherFrame * frame);
        void dispatch(EtherFrame * frame, unsigned int portNum);
        void learn(EtherFrame * frame);
        void broadcast(EtherFrame * frame);

        /**
         * Receives BPDU from the STP/RSTP module and dispatch it to network.
         * Sets EherFrame destination, source, etc. according to the BPDU's Ieee802Ctrl info.
         */
        void dispatchBPDU(BPDU * bpdu);

        /**
         * Deliver BPDU to the STP/RSTP module.
         * Sets the BPDU's Ieee802Ctrl info according to the arriving EtherFrame.
         */
        void deliverBPDU(EtherFrame * frame);

        // For lifecycle
        virtual void start();
        virtual void stop();
        bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);

        /*
         * Gets port data from the InterfaceTable
         */
        Ieee8021dInterfaceData * getPortInterfaceData(unsigned int portNum);

        /*
         * Returns the first non-loopback interface.
         */
        virtual InterfaceEntry * chooseInterface();
        virtual void finish();
};

#endif
