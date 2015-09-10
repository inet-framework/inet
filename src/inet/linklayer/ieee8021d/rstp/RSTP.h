//
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

#ifndef __INET_RSTP_H
#define __INET_RSTP_H

#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/linklayer/ieee8021d/common/Ieee8021dBPDU_m.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/linklayer/ethernet/EtherFrame.h"
#include "inet/linklayer/ethernet/switch/MACAddressTable.h"
#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/linklayer/configurator/Ieee8021dInterfaceData.h"
#include "inet/linklayer/ieee8021d/common/STPBase.h"

namespace inet {

/**
 * Implements the Rapid Spanning Tree Protocol. See the NED file for details.
 */
class INET_API RSTP : public STPBase
{
  protected:
    // kind codes for self messages
    enum SelfKinds { SELF_HELLOTIME = 1, SELF_UPGRADE };

    enum CompareResult {
        WORSE_PORT = -4, WORSE_SRC = -3, WORSE_RPC = -2, WORSE_ROOT = -1, SIMILAR = 0,
        BETTER_ROOT = 1, BETTER_RPC = 2, BETTER_SRC = 3, BETTER_PORT = 4
    };

    // Set by management: see the ned file for more info
    simtime_t migrateTime;
    simtime_t tcWhileTime;
    bool autoEdge = false;

    cMessage *helloTimer = nullptr;
    cMessage *upgradeTimer = nullptr;

  public:
    RSTP();
    virtual ~RSTP();
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj) override;

  protected:
    virtual void initialize(int stage) override;
    virtual void finish() override {}
    virtual void initInterfacedata(unsigned int portNum);

    virtual void start() override;
    virtual void stop() override;

    /**
     * @brief initialize RSTP dynamic information
     */
    virtual void initPorts();

    /**
     * @brief Gets the best alternate port
     * @return Best alternate gate index
     */
    virtual int getBestAlternate();

    /**
     * @brief Sends BPDUs through all ports, if they are required
     */
    virtual void sendBPDUs();

    /**
     * @brief Sends BPDU through a port
     */
    virtual void sendBPDU(int port);

    /**
     * @brief General processing
     */
    virtual void handleMessage(cMessage *msg) override;

    /**
     * @brief BPDU processing.
     * Updates port information. Handles port role changes.
     */
    virtual void handleIncomingFrame(BPDU *frame);
    virtual void processBPDU(BPDU *frame, unsigned int arrival);
    virtual bool processBetterSource(BPDU *frame, unsigned int arrival);
    virtual bool processSameSource(BPDU *frame, unsigned int arrival);

    /**
     * @brief Prints current data base info
     */
    virtual void printState();

    /**
     * @brief Update Interfacedata with the content of the BDPU frame
     */
    virtual void updateInterfacedata(BPDU *frame, unsigned int portNum);

    /**
     * @brief Compares the BPDU frame with the BPDU this module would send through that port
     * @return (<0 if the root BPDU is better than BPDU)
     */
    virtual CompareResult contestInterfacedata(BPDU *msg, unsigned int portNum);

    /**
     * @brief Compares the port's best BPDU with the BPDU this module would send through that port
     * @return (<0 if the root BPDU is better than port's best BPDU)
     */
    virtual CompareResult contestInterfacedata(unsigned int portNum);

    /**
     * @brief Compares a port's best BPDU with a BPDU frame
     * @return (<0 if vector better than frame)
     */
    virtual CompareResult compareInterfacedata(unsigned int portNum, BPDU *msg, int linkCost);

    /**
     * @brief Compares two RSTP data
     * @return (<0 if vector better than frame)
     */
    virtual CompareResult compareRSTPData(int rootPriority1, int rootPriority2,
            MACAddress rootAddress1, MACAddress rootAddress2,
            int rootPathCost1, int rootPathCost2,
            int bridgePriority1, int bridgePriority2,
            MACAddress bridgeAddress1, MACAddress bridgeAddress2,
            int portPriority1, int portPriority2,
            int portNum1, int portNum2);

    /**
     * @brief If root TCWhile has not expired, sends a BPDU to the Root with TCFlag=true.
     */
    virtual void sendTCNtoRoot();

    /**
     * @brief HelloTime event handling.
     */
    virtual void handleHelloTime(cMessage *);

    /**
     * @brief Upgrade event handling.
     */
    virtual void handleUpgrade(cMessage *);

    /**
     * @brief Checks the frame TC flag.
     * Sets TCWhile if the port was forwarding and the flag is true.
     */
    virtual void checkTC(BPDU *frame, int arrival);

    /**
     * @brief Handles the switch to backup in one of the ports
     */
    virtual void handleBackup(BPDU *frame, unsigned int arrival);

    /**
     * @brief schedule next upgrade self-message
     */
    virtual void scheduleNextUpgrade();

    /**
     * @brief flush all port expect one
     */
    virtual void flushOtherPorts(unsigned int portNum);
};

} // namespace inet

#endif // ifndef __INET_RSTP_H

