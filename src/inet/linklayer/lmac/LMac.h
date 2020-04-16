/*
 *  LMac.h
 *  LMAC for MF 2.02, omnetpp 3.4
 *
 *  Created by Anna Förster on 10/10/08.
 *  Copyright 2008 Universita della Svizzera Italiana. All rights reserved.
 *
 *  Converted to MiXiM by Kapourniotis Theodoros
 *
 */

#ifndef __INET_LMAC_H
#define __INET_LMAC_H

#include "inet/queueing/contract/IPacketQueue.h"
#include "inet/linklayer/base/MacProtocolBase.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/contract/IMacProtocol.h"
#include "inet/linklayer/lmac/LMacHeader_m.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"

namespace inet {

/**
 * @brief Implementation of L-MAC (Lightweight Medium Access Protocol for
 * Wireless Sensor Networks [van Hoesel 04] ).
 *
 * Each node has its own unique timeslot. Nodes can use their timeslots to
 * transfer data without having to contend for the medium or having to deal
 * with energy wasting collisions or retransmissions.
 *
 * During the first 5 full frames nodes will be waking up every controlDuration
 * to setup the network first by assigning a different timeslot to each node.
 * Normal packets will be queued, but will be send only after the setup phase.
 *
 * During its timeslot a node wakes up, checks the channel for a short random
 * period (CCA) to check for possible collision in the slot and, if the
 * channel is free, sends a control packet. If there is a collision it tries
 * to change its timeslot to an empty one. After the transmission of the control
 * packet it checks its packet queue and if its non-empty it sends a data
 * packet.
 *
 * During a foreign timeslot a node wakes up, checks the channel for
 * 2*controlDuration period for an incoming control packet and if there in
 * nothing it goes back to sleep and conserves energy for the rest of the
 * timeslot. If it receives a control packet addressed for itself it stays awake
 * for the rest of the timeslot to receive the incoming data packet.
 *
 * The finite state machine of the protocol is given in the below figure:
 *
 * \image html LMACFSM.jpg "State chart for LMAC layer"
 *
 * A paper describing the protocol is:
 *
 * L. van Hoesel and P. Havinga. A lightweight medium access
 * protocol (LMAC) for wireless sensor networks. In Proceedings of the 3rd
 * International Symposium on Information Processing in Sensor Networks (IPSN),
 * pages 55-60, Berkeley, CA, February 2004. April.
 *
 * @ingroup macLayer
 **/
class INET_API LMac : public MacProtocolBase, public IMacProtocol
{
  private:
    /** @brief Copy constructor is not allowed.
     */
    LMac(const LMac&);
    /** @brief Assignment operator is not allowed.
     */
    LMac& operator=(const LMac&);

  public:
    LMac()
        : MacProtocolBase()
        , SETUP_PHASE(true)
        , slotChange()
        , macState()
        , slotDuration(0)
        , headerLength(b(0))
        , ctrlFrameLength(b(0))
        , controlDuration(0)
        , myId(0)
        , mySlot(0)
        , numSlots(0)
        , currSlot()
        , reservedMobileSlots(0)
        , radio(nullptr)
        , transmissionState(physicallayer::IRadio::TRANSMISSION_STATE_UNDEFINED)
        , wakeup(nullptr)
        , timeout(nullptr)
        , sendData(nullptr)
        , initChecker(nullptr)
        , checkChannel(nullptr)
        , start_lmac(nullptr)
        , send_control(nullptr)
        , bitrate(0)
    {}
    /** @brief Clean up messges.*/
    virtual ~LMac();

    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int) override;

    /** @brief Handle messages from lower layer */
    virtual void handleLowerPacket(Packet *packet) override;

    /** @brief Handle messages from upper layer */
    virtual void handleUpperPacket(Packet *packet) override;

    /** @brief Handle self messages such as timers */
    virtual void handleSelfMessage(cMessage *) override;

    /** @brief Handle control messages from lower layer */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details) override;

    /** @brief Encapsulate the NetwPkt into an MacPkt */
    virtual void encapsulate(Packet *);
    virtual void decapsulate(Packet *);

  protected:
    /** @brief Generate new interface address*/
    virtual void configureInterfaceEntry() override;
    virtual void handleCommand(cMessage *msg) {}

    /** @brief MAC states
     *
     *  The MAC states help to keep track what the MAC is actually
     *  trying to do -- this is esp. useful when radio switching takes
     *  some time.
     *  SLEEP -- the node sleeps but accepts packets from the network layer
     *  RX  -- MAC accepts packets from PHY layer
     *  TX  -- MAC transmits a packet
     *  CCA -- Clear Channel Assessment - MAC checks
     *         whether medium is busy
     */

    enum States {
        INIT, SLEEP, CCA, WAIT_CONTROL, WAIT_DATA, SEND_CONTROL, SEND_DATA
    };

    /** @brief dummy receiver address to indicate no pending packets in the control packet */
    static const MacAddress LMAC_NO_RECEIVER;
    static const MacAddress LMAC_FREE_SLOT;

    /** @brief the setup phase is the beginning of the simulation, where only control packets at very small slot durations are exchanged. */
    bool SETUP_PHASE;

    /** @brief indicate how often the node needs to change its slot because of collisions */
    cOutVector *slotChange;

    /** @brief keep track of MAC state */
    States macState;

    /** @brief Duration of a slot */
    double slotDuration;
    /** @brief Length of the header */
    b headerLength;
    /** @brief Length of the control frames */
    b ctrlFrameLength;
    /** @brief Duration of teh control time in each slot */
    double controlDuration;
    /** @brief my ID */
    int myId;
    /** @brief my slot ID */
    int mySlot;
    /** @brief how many slots are there */
    int numSlots;
    /** @brief The current slot of the simulation */
    int currSlot;
    /** @brief Occupied slots from nodes, from which I hear directly */
    MacAddress occSlotsDirect[64];
    /** @brief Occupied slots of two-hop neighbors */
    MacAddress occSlotsAway[64];
    /** @brief The first couple of slots are reserved for nodes with special needs to avoid changing slots for them (mobile nodes) */
    int reservedMobileSlots;

    /** @brief The radio. */
    physicallayer::IRadio *radio;
    physicallayer::IRadio::TransmissionState transmissionState;

    cMessage *wakeup;
    cMessage *timeout;
    cMessage *sendData;
    cMessage *initChecker;
    cMessage *checkChannel;
    cMessage *start_lmac;
    cMessage *send_control;

    /** @brief the bit rate at which we transmit */
    double bitrate;

    /** @brief find a new slot */
    void findNewSlot();

    /** @brief Internal function to attach a signal to the packet */
    void attachSignal(Packet *macPkt);
};

} // namespace inet

#endif // ifndef __INET_LMAC_H

