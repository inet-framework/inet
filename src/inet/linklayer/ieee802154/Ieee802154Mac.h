//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//
/* -*- mode:c++ -*- ********************************************************
 * file:        Ieee802154Mac.h
 *
 * author:     Jerome Rousselot, Marcel Steine, Amre El-Hoiydi,
 *                Marc Loebbers, Yosia Hadisusanto
 *
 * copyright:    (C) 2007-2009 CSEM SA
 *              (C) 2009 T.U. Eindhoven
 *                (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *
 * Funding: This work was partially financed by the European Commission under the
 * Framework 6 IST Project "Wirelessly Accessible Sensor Populations"
 * (WASP) under contract IST-034963.
 ***************************************************************************
 * part of:    Modifications to the MF-2 framework by CSEM
 **************************************************************************/

#ifndef __INET_IEEE802154MAC_H
#define __INET_IEEE802154MAC_H

#include "inet/linklayer/base/MacProtocolBase.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/contract/IMacProtocol.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/queueing/contract/IActivePacketSink.h"
#include "inet/queueing/contract/IPacketQueue.h"

namespace inet {

/**
 * @brief Generic CSMA Mac-Layer.
 *
 * Supports constant, linear and exponential backoffs as well as
 * MAC ACKs.
 *
 * @author Jerome Rousselot, Amre El-Hoiydi, Marc Loebbers, Yosia Hadisusanto, Andreas Koepke
 * @author Karl Wessel (port for MiXiM)
 *
 * \image html csmaFSM.png "CSMA Mac-Layer - finite state machine"
 */
class INET_API Ieee802154Mac : public MacProtocolBase, public IMacProtocol, public queueing::IActivePacketSink
{
  public:
    Ieee802154Mac() : MacProtocolBase() {}

    virtual ~Ieee802154Mac();

    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int) override;

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

    // IActivePacketSink:
    virtual queueing::IPassivePacketSource *getProvider(cGate *gate) override;
    virtual void handleCanPullPacketChanged(cGate *gate) override;
    virtual void handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful) override;

  protected:
    /** @name Different tracked statistics.*/
    /*@{*/
    long nbTxFrames = 0;
    long nbRxFrames = 0;
    long nbMissedAcks = 0;
    long nbRecvdAcks = 0;
    long nbDroppedFrames = 0;
    long nbTxAcks = 0;
    long nbDuplicates = 0;
    long nbBackoffs = 0;
    double backoffValues = 0.0;
    /*@}*/

    /** @brief MAC states
     * see states diagram.
     */
    enum t_mac_states {
        IDLE_1 = 1,
        BACKOFF_2,
        CCA_3,
        TRANSMITFRAME_4,
        WAITACK_5,
        WAITSIFS_6,
        TRANSMITACK_7
    };

    /*************************************************************/
    /****************** TYPES ************************************/
    /*************************************************************/

    /** @brief Kinds for timer messages.*/
    enum t_mac_timer {
        TIMER_NULL = 0,
        TIMER_BACKOFF,
        TIMER_CCA,
        TIMER_SIFS,
        TIMER_RX_ACK,
    };

    /** @name Pointer for timer messages.*/
    /*@{*/
    cMessage *backoffTimer = nullptr;
    cMessage *ccaTimer = nullptr;
    cMessage *sifsTimer = nullptr;
    cMessage *rxAckTimer = nullptr;
    /*@}*/

    /** @brief MAC state machine events.
     * See state diagram.*/
    enum t_mac_event {
        EV_SEND_REQUEST = 1, // 1, 11, 20, 21, 22
        EV_TIMER_BACKOFF, // 2, 7, 14, 15
        EV_FRAME_TRANSMITTED, // 4, 19
        EV_ACK_RECEIVED, // 5
        EV_ACK_TIMEOUT, // 12
        EV_FRAME_RECEIVED, // 15, 26
        EV_DUPLICATE_RECEIVED,
        EV_TIMER_SIFS, // 17
        EV_BROADCAST_RECEIVED, // 23, 24
        EV_TIMER_CCA
    };

    /** @brief Types for frames sent by the CSMA.*/
    enum t_csma_frame_types {
        DATA,
        ACK
    };

    enum t_mac_carrier_sensed {
        CHANNEL_BUSY = 1,
        CHANNEL_FREE
    };

    enum t_mac_status {
        STATUS_OK = 1,
        STATUS_ERROR,
        STATUS_RX_ERROR,
        STATUS_RX_TIMEOUT,
        STATUS_FRAME_TO_PROCESS,
        STATUS_NO_FRAME_TO_PROCESS,
        STATUS_FRAME_TRANSMITTED
    };

    /** @brief The different back-off methods.*/
    enum backoff_methods {
        /** @brief Constant back-off time.*/
        CONSTANT = 0,
        /** @brief Linear increasing back-off time.*/
        LINEAR,
        /** @brief Exponentially increasing back-off time.*/
        EXPONENTIAL,
    };

    /** @brief keep track of MAC state */
    t_mac_states macState = IDLE_1;
    t_mac_status status = STATUS_OK;

    /** @brief The radio. */
    ModuleRefByPar<physicallayer::IRadio> radio;
    physicallayer::IRadio::TransmissionState transmissionState = physicallayer::IRadio::TRANSMISSION_STATE_UNDEFINED;

    /** @brief Maximum time between a packet and its ACK
     *
     * Usually this is slightly more then the tx-rx turnaround time
     * The channel should stay clear within this period of time.
     */
    simtime_t sifs;

    /** @brief The amount of time the MAC waits for the ACK of a packet.*/
    simtime_t macAckWaitDuration;

    /** @brief Length of the header*/
    int headerLength = 0;

    bool transmissionAttemptInterruptedByRx = false;
    /** @brief CCA detection time */
    simtime_t ccaDetectionTime;
    /** @brief Time to setup radio from sleep to Rx state */
    simtime_t rxSetupTime;
    /** @brief Time to switch radio from Rx to Tx state */
    simtime_t aTurnaroundTime;
    /** @brief maximum number of extra backoffs (excluding the first unconditional one) before frame drop */
    int macMaxCSMABackoffs = 0;
    /** @brief maximum number of frame retransmissions without ack */
    unsigned int macMaxFrameRetries = 0;
    /** @brief base time unit for calculating backoff durations */
    simtime_t aUnitBackoffPeriod;
    /** @brief Stores if the MAC expects Acks for Unicast packets.*/
    bool useMACAcks = false;

    /** @brief Defines the backoff method to be used.*/
    backoff_methods backoffMethod = CONSTANT;

    /**
     * @brief Minimum backoff exponent.
     * Only used for exponential backoff method.
     */
    int macMinBE = 0;
    /**
     * @brief Maximum backoff exponent.
     * Only used for exponential backoff method.
     */
    int macMaxBE = 0;

    /** @brief initial contention window size
     * Only used for linear and constant backoff method.*/
    int initialCW = 0;

    /** @brief The power (in mW) to transmit with.*/
    double txPower = 0.0;

    /** @brief number of backoff performed until now for current frame */
    int NB = 0;

    /** @brief count the number of tx attempts
     *
     * This holds the number of transmission attempts for the current frame.
     */
    unsigned int txAttempts = 0;

    /** @brief the bit rate at which we transmit */
    double bitrate = 0.0;

    /** @brief The bit length of the ACK packet.*/
    int ackLength = 0;

  protected:
    /** @brief Generate new interface address*/
    virtual void configureNetworkInterface() override;
    virtual void handleCommand(cMessage *msg) {}

    // FSM functions
    void fsmError(t_mac_event event, cMessage *msg);
    void executeMac(t_mac_event event, cMessage *msg);
    void updateStatusIdle(t_mac_event event, cMessage *msg);
    void updateStatusBackoff(t_mac_event event, cMessage *msg);
    void updateStatusCCA(t_mac_event event, cMessage *msg);
    void updateStatusTransmitFrame(t_mac_event event, cMessage *msg);
    void updateStatusWaitAck(t_mac_event event, cMessage *msg);
    void updateStatusSIFS(t_mac_event event, cMessage *msg);
    void updateStatusTransmitAck(t_mac_event event, cMessage *msg);
    void updateStatusNotIdle(cMessage *msg);
    void manageQueue();
    void updateMacState(t_mac_states newMacState);

    void attachSignal(Packet *mac, simtime_t_cref startTime);
    void manageMissingAck(t_mac_event event, cMessage *msg);
    void startTimer(t_mac_timer timer);

    virtual simtime_t scheduleBackoff();

    virtual void encapsulate(Packet *packet);
    virtual void decapsulate(Packet *packet);

    // OperationalBase:
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
    virtual void refreshDisplay() const override;

    Packet *ackMessage = nullptr;

    // sequence number for sending, map for the general case with more senders
    // also in initialisation phase multiple potential parents
    std::map<MacAddress, unsigned long> SeqNrParent; // parent -> sequence number

    // sequence numbers for receiving
    std::map<MacAddress, unsigned long> SeqNrChild; // child -> sequence number
};

} // namespace inet

#endif

