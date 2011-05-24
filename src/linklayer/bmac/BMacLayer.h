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

#ifndef BMACLAYER_H_
#define BMACLAYER_H_

#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <bmacpkt_m.h>
#include <WirelessMacBase.h>
#include "NotificationBoard.h"
#include "NotifierConsts.h"
#include "RadioState.h"
#include "MACAddress.h"
#include "IPassiveQueue.h"
#include "Ieee802154MacPhyPrimitives_m.h"
#include "Ieee802154NetworkCtrlInfo_m.h"
#include "Ieee802154Enum.h"
/**
 *\mainpage
 * @brief Implementation of B-MAC (called also Berkeley MAC, Low Power Listening or LPL.
 *
 * The protocol works as follows: each node is allowed to sleep for slotDuration. After waking up, it first checks the channel for ongoing transmisison.
 * If a transmission is catched (a premable is received), the node stays awake for at most slotDuration and waits for the actual data packet.
 * If a node wants to send a packet, it first sends preambles for at least slotDuration, thus waking up all nodes in its transmisison radius and
 * then sends out the data packet. If a mac-level ack is required, then the receiver sends the ack immediately after receiving the packet (no preambles)
 * and the sender waits for some time more before going back to sleep.
 *
 * B-MAC is designed for low traffic, low power communication in WSN and is one of the most widely usde protocols (e.g. it is part of TinyOS).
 * The finite state machine of the protocol is given in the below figure:
 *
 * \image html FSM.png "B-MAC Layer - finite state machine"
 *
 *
 * @class BMacLayer
 * @ingroup bmac
 * @ingroup macLayer
 * @author Anna Foerster
 *
 */
class  BMacLayer : public WirelessMacBase, public INotifiable
{
  public:


	~BMacLayer();
	BMacLayer();

    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);
    virtual int    numInitStages    () const { return 2;}

    /** @brief Delete all dynamically allocated objects of the module*/
    virtual void finish();

    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(cPacket*);

    /** @brief Handle messages from upper layer */
    virtual void handleUpperMsg(cPacket*);

    /** @brief Handle self messages such as timers */
    virtual void handleSelfMsg(cMessage*);

    /** @brief Handle control messages from lower layer */
    virtual void handleCommand(cMessage *msg);

  protected:
    PHYenum phystatus;
    /** @brief  pointer to the NotificationBoard module */
    NotificationBoard* mpNb;
    typedef std::list<BmacPkt*> MacQueue;
    uint64_t L2BROADCAST;
    uint64_t myMacAddr;
    MACAddress macAddress;
    RadioState::State radioState;

    /** @brief A queue to store packets from upper layer in case another
    packet is still waiting for transmission..*/
    MacQueue macQueue;

    /** @name Different tracked statistics.*/
    /*@{*/
    long nbTxDataPackets;
    long nbTxPreambles;
    long nbRxDataPackets;
    long nbRxPreambles;
    long nbMissedAcks;
    long nbRecvdAcks;
    long nbDroppedDataPackets;
    long nbTxAcks;
    /*@}*/

    /** @brief MAC states
       *
       *  The MAC states help to keep track what the MAC is actually
       *  trying to do.
       *  INIT -- node has just started and its status is unclear
       *  SLEEP -- node sleeps, but accepts packets from the network layer
       *  CCA -- Clear Channel Assessment - MAC checks
       *         whether medium is busy
       *  SEND_PREAMBLE -- node sends preambles to wake up all nodes
       *  WAIT_DATA -- node has received at least one preamble from another node and wiats for the actual data packet
       *  SEND_DATA -- node has sent enough preambles and sends the actual data packet
       *  WAIT_TX_DATA_OVER -- node waits until the data packet sending is ready
       *  WAIT_ACK -- node has sent the data packet and waits for ack from the receiving node
       *  SEND_ACK -- node send an ACK back to the sender
       *  WAIT_ACK_TX -- node waits until the transmission of the ack packet is over
       */
    enum States {
        INIT,    //0
        SLEEP,    //1
        CCA,    //2
        SEND_PREAMBLE,     //3
        WAIT_DATA,        //4
        SEND_DATA,        //5
        WAIT_TX_DATA_OVER,    //6
        WAIT_ACK,        //7
        SEND_ACK,        //8
        WAIT_ACK_TX        //9
      };
    /** @brief The current state of the protocol */
    States macState;

    /** @brief Types of messages (self messages and packets) the node can process
           *
           **/
    enum TYPES {
        // packet types
        BMAC_PREAMBLE = 191,
        BMAC_DATA,
        BMAC_ACK,
        // self message types
        BMAC_RESEND_DATA,
        BMAC_ACK_TIMEOUT,
        BMAC_START_BMAC,
        BMAC_WAKE_UP,
        BMAC_SEND_ACK,
        BMAC_CCA_TIMEOUT,
        BMAC_ACK_TX_OVER,
        BMAC_SEND_PREAMBLE,
        BMAC_STOP_PREAMBLES,
        BMAC_DATA_TX_OVER,
        BMAC_DATA_TIMEOUT
    };

    // messages used in the FSM
    cMessage *resend_data;
    cMessage *ack_timeout;
    cMessage *start_bmac;
    cMessage *wakeup;
    cMessage *send_ack;
    cMessage *cca_timeout;
    cMessage *ack_tx_over;
    cMessage *send_preamble;
    cMessage *stop_preambles;
    cMessage *data_tx_over;
    cMessage *data_timeout;

    /** @brief Help variables for the acknowledgement process
           *
           */
    uint64_t lastDataPktSrcAddr;
    uint64_t lastDataPktDestAddr;
    int txAttempts;



    /** @brief Inspect reasons for dropped packets */
//    DroppedPacket droppedPacket;

    /** @brief plus category from BB */
    int catDroppedPacket;

    /** @brief publish dropped packets nic wide */
    int nicId;

    /** @brief The maximum length of the queue */
    double queueLength;
    /** @brief Animate (colorize) the nodes.
     *
     * The color of the node reflects its basic status (not the exact state!)
     * BLACK - node is sleeping
     * GREEN - node is receiving
     * YELLOW - node is sending
     */
    bool animation;
    // write the status in bubbles
    bool animationBubble;
    /** @brief The duration of the slot in secs. */
    double slotDuration;
    /** @brief The bitrate of transmission */
    double bitrate;
    /** @brief The length of the MAC header */
    double headerLength;
    /** @brief The duration of CCA */
    double checkInterval;
    /** @brief Transmission power of the node */
    double txPower;
    /** @brief Use MAc level acks or not */
    bool useMacAcks;
    /** @brief Maximum trasnmission attempts per data packet, when ACKs are used */
    int maxTxAttempts;
    /** @brief Gather stats at the end of the simulation */
    bool stats;

    /** @brief Possible colors of the node for animation */
    enum BMAC_COLORS {
        GREEN = 1,
        BLUE = 2,
        RED = 3,
        BLACK = 4,
        YELLOW = 5
    };

    enum BMAC_BUBBLE {
        B_RX,
        B_TXPREAMBLE,
        B_TXDATA,
        B_TXACK,
        B_RXPREAMBLE,
        B_RXDATA,
        B_RXACK,
        B_WAITACK,
        B_SLEEP
    };
    /** @brief Internal fucntion to change the color of the node */
    void changeDisplayColor(BMAC_COLORS color);

    /** @brief Internal function to write bubbles with the internal state */
    void showBuble(BMAC_BUBBLE bubble);

    /** @brief Internal function to send the first packet in the queue */
    void sendDataPacket();

    /** @brief Internal fucnction to send an ACK */
    void sendMacAck();

    /** @brief Internal function to send one preamble */
    void sendPreamble();

    /** @brief Internal function to attach a signal to the packet */
    void attachSignal(BmacPkt *macPkt);

    /** @brief Internal function to add a new packet from upper to the queue */
    bool addToQueue(cMessage * msg);

    virtual void receiveChangeNotification(int category, const cPolymorphic *details);
    virtual void registerInterface();

    void PLME_SET_TRX_STATE_request(PHYenum state);
    // Use to distinguish the radio module that send the event
    int radioModule;

    int getRadioModuleId() {return radioModule;}
    bool useIeee802Ctrl;
    cPacket *decapsMsg(BmacPkt * macPkt);
    void reqtMsgFromQueue();
    void initializeQueueModule();
    /** @brief pointer to the passive queue module */
    IPassiveQueue* queueModule;

};

#endif /* BMACLAYER_H_ */
