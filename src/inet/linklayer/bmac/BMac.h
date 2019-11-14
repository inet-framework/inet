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

#ifndef __INET_BMAC_H
#define __INET_BMAC_H

#include "inet/queueing/contract/IPacketQueue.h"
#include "inet/linklayer/base/MacProtocolBase.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/contract/IMacProtocol.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"

namespace inet {

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
 * @class BMac
 * @ingroup macLayer
 * @author Anna Foerster
 *
 */
class INET_API BMac : public MacProtocolBase, public IMacProtocol
{
  private:
    /** @brief Copy constructor is not allowed.
     */
    BMac(const BMac&);
    /** @brief Assignment operator is not allowed.
     */
    BMac& operator=(const BMac&);

  public:
    BMac() {}
    virtual ~BMac();

    /** @brief Initialization of the module and some variables*/
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int) override;
    virtual void refreshDisplay() const override;

    /** @brief Delete all dynamically allocated objects of the module*/
    virtual void finish() override;

    /** @brief Handle messages from lower layer */
    virtual void handleLowerPacket(Packet *packet) override;

    /** @brief Handle messages from upper layer */
    virtual void handleUpperPacket(Packet *packet) override;

    /** @brief Handle self messages such as timers */
    virtual void handleSelfMessage(cMessage *) override;

    /** @brief Handle control messages from lower layer */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details) override;

  protected:
    /** @brief The radio. */
    physicallayer::IRadio *radio = nullptr;
    physicallayer::IRadio::TransmissionState transmissionState = physicallayer::IRadio::TRANSMISSION_STATE_UNDEFINED;

    /** @name Different tracked statistics.*/
    /*@{*/
    long nbTxDataPackets = 0;
    long nbTxPreambles = 0;
    long nbRxDataPackets = 0;
    long nbRxPreambles = 0;
    long nbMissedAcks = 0;
    long nbRecvdAcks = 0;
    long nbDroppedDataPackets = 0;
    long nbTxAcks = 0;
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
    States macState = static_cast<States>(-1);

    // messages used in the FSM
    cMessage *resend_data = nullptr;
    cMessage *ack_timeout = nullptr;
    cMessage *start_bmac = nullptr;
    cMessage *wakeup = nullptr;
    cMessage *send_ack = nullptr;
    cMessage *cca_timeout = nullptr;
    cMessage *ack_tx_over = nullptr;
    cMessage *send_preamble = nullptr;
    cMessage *stop_preambles = nullptr;
    cMessage *data_tx_over = nullptr;
    cMessage *data_timeout = nullptr;

    /** @name Help variables for the acknowledgment process. */
    /*@{*/
    MacAddress lastDataPktSrcAddr;
    MacAddress lastDataPktDestAddr;
    int txAttempts = 0;
    /*@}*/

    /** @brief Animate (colorize) the nodes.
     *
     * The color of the node reflects its basic status (not the exact state!)
     * BLACK - node is sleeping
     * GREEN - node is receiving
     * YELLOW - node is sending
     */
    bool animation = false;
    /** @brief The duration of the slot in secs. */
    double slotDuration = 0;
    /** @brief Length of the header*/
    b headerLength = b(0);
    b ctrlFrameLength = b(0);
    /** @brief The bitrate of transmission */
    double bitrate = 0;
    /** @brief The duration of CCA */
    double checkInterval = 0;
    /** @brief Use MAC level acks or not */
    bool useMacAcks = false;
    /** @brief Maximum transmission attempts per data packet, when ACKs are
     * used */
    int maxTxAttempts = 0;
    /** @brief Gather stats at the end of the simulation */
    bool stats = false;

    /** @brief Generate new interface address*/
    virtual void configureInterfaceEntry() override;
    virtual void handleCommand(cMessage *msg) {}

    /** @brief Internal function to send the first packet in the queue */
    void sendDataPacket();

    /** @brief Internal function to send an ACK */
    void sendMacAck();

    /** @brief Internal function to send one preamble */
    void sendPreamble();

    /** @brief Internal function to attach a signal to the packet */
    void attachSignal(Packet *macPkt);

    void decapsulate(Packet *packet);
    void encapsulate(Packet *packet);
};

} // namespace inet

#endif // ifndef __INET_BMAC_H

