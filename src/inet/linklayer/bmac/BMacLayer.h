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

#ifndef __INET_BMACLAYER_H
#define __INET_BMACLAYER_H

#include "inet/physicallayer/contract/IRadio.h"
#include "inet/linklayer/contract/IMACProtocol.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/linklayer/common/MACProtocolBase.h"
#include "inet/linklayer/bmac/BMacFrame_m.h"

namespace inet {

using namespace physicallayer;

/**
 * @brief Implementation of B-MAC (called also Berkeley MAC, Low Power
 * Listening or LPL).
 *
 * The protocol works as follows: each node is allowed to sleep for
 * slotDuration. After waking up, it first checks the channel for ongoing
 * transmissions.
 * If a transmission is catched (a preamble is received), the node stays awake
 * for at most slotDuration and waits for the actual data packet.
 * If a node wants to send a packet, it first sends preambles for at least
 * slotDuration, thus waking up all nodes in its transmission radius and
 * then sends out the data packet. If a mac-level ack is required, then the
 * receiver sends the ack immediately after receiving the packet (no preambles)
 * and the sender waits for some time more before going back to sleep.
 *
 * B-MAC is designed for low traffic, low power communication in WSN and is one
 * of the most widely used protocols (e.g. it is part of TinyOS).
 * The finite state machine of the protocol is given in the below figure:
 *
 * \image html BMACFSM.png "B-MAC Layer - finite state machine"
 *
 * A paper describing this implementation can be found at:
 * http://www.omnet-workshop.org/2011/uploads/slides/OMNeT_WS2011_S5_C1_Foerster.pdf
 *
 * @class BMacLayer
 * @ingroup macLayer
 * @author Anna Foerster
 *
 */
class INET_API BMacLayer : public MACProtocolBase, public IMACProtocol
{
  private:
    /** @brief Copy constructor is not allowed.
     */
    BMacLayer(const BMacLayer&);
    /** @brief Assignment operator is not allowed.
     */
    BMacLayer& operator=(const BMacLayer&);

  public:
    BMacLayer()
        : MACProtocolBase()
        , macQueue()
        , nbTxDataPackets(0), nbTxPreambles(0), nbRxDataPackets(0), nbRxPreambles(0)
        , nbMissedAcks(0), nbRecvdAcks(0), nbDroppedDataPackets(0), nbTxAcks(0)
        , macState(INIT)
        , resend_data(NULL), ack_timeout(NULL), start_bmac(NULL), wakeup(NULL)
        , send_ack(NULL), cca_timeout(NULL), ack_tx_over(NULL), send_preamble(NULL), stop_preambles(NULL)
        , data_tx_over(NULL), data_timeout(NULL)
        , lastDataPktSrcAddr()
        , lastDataPktDestAddr()
        , txAttempts(0)
        , queueLength(0)
        , animation(false)
        , slotDuration(0), headerLength(0), bitrate(0), checkInterval(0)
        , useMacAcks(0)
        , maxTxAttempts(0)
        , stats(false)
    {}
    virtual ~BMacLayer();

    /** @brief Initialization of the module and some variables*/
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int);

    /** @brief Delete all dynamically allocated objects of the module*/
    virtual void finish();

    /** @brief Handle messages from lower layer */
    virtual void handleLowerPacket(cPacket *);

    /** @brief Handle messages from upper layer */
    virtual void handleUpperPacket(cPacket *);

    /** @brief Handle self messages such as timers */
    virtual void handleSelfMessage(cMessage *);

    /** @brief Handle control messages from lower layer */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, long value);

  protected:
    typedef std::list<BMacFrame *> MacQueue;

    /** @brief The MAC address of the interface. */
    MACAddress address;

    /** @brief A queue to store packets from upper layer in case another
       packet is still waiting for transmission.*/
    MacQueue macQueue;

    /** @brief The radio. */
    IRadio *radio;
    IRadio::TransmissionState transmissionState;

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
     *  WAIT_DATA -- node has received at least one preamble from another node
     *                  and wiats for the actual data packet
     *  SEND_DATA -- node has sent enough preambles and sends the actual data
     *                  packet
     *  WAIT_TX_DATA_OVER -- node waits until the data packet sending is ready
     *  WAIT_ACK -- node has sent the data packet and waits for ack from the
     *                 receiving node
     *  SEND_ACK -- node send an ACK back to the sender
     *  WAIT_ACK_TX -- node waits until the transmission of the ack packet is
     *                    over
     */
    enum States {
        INIT,    //0
        SLEEP,    //1
        CCA,    //2
        SEND_PREAMBLE,    //3
        WAIT_DATA,    //4
        SEND_DATA,    //5
        WAIT_TX_DATA_OVER,    //6
        WAIT_ACK,    //7
        SEND_ACK,    //8
        WAIT_ACK_TX    //9
    };
    /** @brief The current state of the protocol */
    States macState;

    /** @brief Types of messages (self messages and packets) the node can
     * process **/
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

    /** @name Help variables for the acknowledgment process. */
    /*@{*/
    MACAddress lastDataPktSrcAddr;
    MACAddress lastDataPktDestAddr;
    int txAttempts;
    /*@}*/

    /** @brief The maximum length of the queue */
    unsigned int queueLength;
    /** @brief Animate (colorize) the nodes.
     *
     * The color of the node reflects its basic status (not the exact state!)
     * BLACK - node is sleeping
     * GREEN - node is receiving
     * YELLOW - node is sending
     */
    bool animation;
    /** @brief The duration of the slot in secs. */
    double slotDuration;
    /** @brief Length of the header*/
    int headerLength;
    /** @brief The bitrate of transmission */
    double bitrate;
    /** @brief The duration of CCA */
    double checkInterval;
    /** @brief Use MAC level acks or not */
    bool useMacAcks;
    /** @brief Maximum transmission attempts per data packet, when ACKs are
     * used */
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

    /** @brief Generate new interface address*/
    virtual void initializeMACAddress();
    virtual InterfaceEntry *createInterfaceEntry();
    virtual void handleCommand(cMessage *msg) {}

    /** @brief Internal function to change the color of the node */
    void changeDisplayColor(BMAC_COLORS color);

    /** @brief Internal function to send the first packet in the queue */
    void sendDataPacket();

    /** @brief Internal function to send an ACK */
    void sendMacAck();

    /** @brief Internal function to send one preamble */
    void sendPreamble();

    /** @brief Internal function to attach a signal to the packet */
    void attachSignal(BMacFrame *macPkt);

    /** @brief Internal function to add a new packet from upper to the queue */
    bool addToQueue(cMessage *msg);

    virtual void flushQueue();

    virtual void clearQueue();

    cPacket *decapsMsg(BMacFrame *msg);
    BMacFrame *encapsMsg(cPacket *netwPkt);
    cObject *setUpControlInfo(cMessage *const pMsg, const MACAddress& pSrcAddr);
};

} // namespace inet

#endif // ifndef __INET_BMACLAYER_H

