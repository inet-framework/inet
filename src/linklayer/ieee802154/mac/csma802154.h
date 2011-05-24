/* -*- mode:c++ -*- ********************************************************
 * file:        csma.h
 *
  * author:     Jerome Rousselot, Marcel Steine, Amre El-Hoiydi,
 *              Marc Loebbers, Yosia Hadisusanto
 *
 * copyright:   (C) 2007-2009 CSEM SA
 *              (C) 2009 T.U. Eindhoven
 *              (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 *
 * Funding: This work was partially financed by the European Commission under the
 * Framework 6 IST Project "Wirelessly Accessible Sensor Populations"
 * (WASP) under contract IST-034963.
 ***************************************************************************
 * part of:    Modifications to the MF-2 framework by CSEM
 **************************************************************************/


#ifndef csma_H
#define csma_H


#include <string>
#include <sstream>
#include <vector>
#include <list>
#include "Ieee802154Mac.h"

/**
 * @brief Generic CSMA Mac-Layer.
 *
 * Supports constant, linear and exponential backoffs as well as
 * MAC ACKs.
 *
 * @class csma
 * @ingroup csma
 * @ingroup macLayer
 * @author Jerome Rousselot, Amre El-Hoiydi, Marc Loebbers, Yosia Hadisusanto, Andreas Koepke
 * @author Karl Wessel (port for MiXiM)
 *
 * \image html csmaFSM.png "CSMA Mac-Layer - finite state machine"
 */
class  csma802154 : public Ieee802154Mac
{

    Ieee802154Frame *sendPacket;
    PHYenum phystatus;
  public:
    virtual int    numInitStages    () const { return 3; }
    csma802154() {sendPacket = NULL;}
    ~csma802154();

    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);

    /** @brief Delete all dynamically allocated objects of the module*/
    virtual void finish();

    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(cMessage*);

    /** @brief Handle messages from upper layer */
    virtual void handleUpperMsg(cMessage*);

    /** @brief Handle self messages such as timers */
    virtual void handleSelfMsg(cMessage*);

    /** @brief Handle control messages from lower layer */
    virtual void handleLowerControl(cMessage *msg);

  protected:
    virtual void receiveChangeNotification(int category, const cPolymorphic *details);
    typedef std::list<Ieee802154Frame*> MacQueue;

    /** @name Different tracked statistics.*/
    /*@{*/
    long nbTxFrames;
    long nbRxFrames;
    long nbMissedAcks;
    long nbRecvdAcks;
    long nbDroppedFrames;
    long nbTxAcks;
    long nbDuplicates;
    long nbBackoffs;
    double backoffValues;
    bool useIeee802Ctrl;
    /*@}*/

    /** @brief Records general statistics?*/
    bool stats;
    /** @brief Record out put vectors?*/
    bool trace;

    /** @brief MAC states
     * see states diagram.
     */
    enum t_mac_states
    {
        IDLE_1=1,
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
    enum t_mac_timer
    {
        TIMER_NULL=0,
        TIMER_BACKOFF,
        TIMER_CCA,
        TIMER_SIFS,
        TIMER_RX_ACK,
    };

    /** @name Pointer for timer messages.*/
    /*@{*/
    cMessage * backoffTimer, * ccaTimer, * txTimer, * sifsTimer, * rxAckTimer;
    /*@}*/

    /** @brief MAC state machine events.
     * See state diagram.*/
    enum t_mac_event
    {
        EV_SEND_REQUEST=1,                   // 1, 11, 20, 21, 22
        EV_TIMER_BACKOFF,                    // 2, 7, 14, 15
        EV_FRAME_TRANSMITTED,                // 4, 19
        EV_ACK_RECEIVED,                     // 5
        EV_ACK_TIMEOUT,                      // 12
        EV_FRAME_RECEIVED,                   // 15, 26
        EV_DUPLICATE_RECEIVED,
        EV_TIMER_SIFS,                       // 17
        EV_BROADCAST_RECEIVED,           // 23, 24
        EV_TIMER_CCA
    };

    /** @brief Types for frames sent by the CSMA.*/
    enum t_csma_frame_types
    {
        DATA,
        ACK
    };

    enum t_mac_carrier_sensed
    {
        CHANNEL_BUSY=1,
        CHANNEL_FREE
    } ;

    enum t_mac_status
    {
        STATUS_OK=1,
        STATUS_ERROR,
        STATUS_RX_ERROR,
        STATUS_RX_TIMEOUT,
        STATUS_FRAME_TO_PROCESS,
        STATUS_NO_FRAME_TO_PROCESS,
        STATUS_FRAME_TRANSMITTED
    };

    /** @brief The different back-off methods.*/
    enum backoff_methods
    {
        /** @brief Constant back-off time.*/
        CONSTANT = 0,
        /** @brief Linear increasing back-off time.*/
        LINEAR,
        /** @brief Exponentially increasing back-off time.*/
        EXPONENTIAL,
    };

    /** @brief keep track of MAC state */
    t_mac_states macState;
    t_mac_status status;

    /** @brief Maximum time between a packet and its ACK
     *
     * Usually this is slightly more then the tx-rx turnaround time
     * The channel should stay clear within this period of time.
     */
    simtime_t sifs;
    int headerLength;
    RadioState::State radioState;

    /** @brief The amount of time the MAC waits for the ACK of a packet.*/
    simtime_t macAckWaitDuration;

    bool transmissionAttemptInterruptedByRx;
    /** @brief CCA detection time */
    simtime_t ccaDetectionTime;
    /** @brief Time to setup radio from sleep to Rx state */
    simtime_t rxSetupTime;
    /** @brief Time to switch radio from Rx to Tx state */
    simtime_t aTurnaroundTime;
    /** @brief maximum number of backoffs before frame drop */
    int macMaxCSMABackoffs;
    /** @brief maximum number of frame retransmissions without ack */
    unsigned int macMaxFrameRetries;
    /** @brief base time unit for calculating backoff durations */
    simtime_t aUnitBackoffPeriod;
    /** @brief Stores if the MAC expects Acks for Unicast packets.*/
    bool useMACAcks;

    /** @brief Defines the backoff method to be used.*/
    backoff_methods backoffMethod;

    /**
     * @brief Minimum backoff exponent.
     * Only used for exponential backoff method.
     */
    int macMinBE;
    /**
     * @brief Maximum backoff exponent.
     * Only used for exponential backoff method.
     */
    int macMaxBE;

    /** @brief initial contention window size
     * Only used for linear and constant backoff method.*/
    double initialCW;

    /** @brief The power (in mW) to transmit with.*/
    double txPower;

    /** @brief number of backoff performed until now for current frame */
    int NB;

    /** @brief A queue to store packets from upper layer in case another
    packet is still waiting for transmission..*/
    MacQueue macQueue;

    /** @brief length of the queue*/
    unsigned int queueLength;

    /** @brief count the number of tx attempts
     *
     * This holds the number of transmission attempts for the current frame.
     */
    unsigned int txAttempts;


    /** @brief plus category from BB */
    int catDroppedPacket;

    /** @brief publish dropped packets nic wide */
    int nicId;

    /** @brief The bit length of the ACK packet.*/
    int ackLength;



    void sendNewPacketInTx(Ieee802154Frame *p)
    {
        if (sendPacket)
            error("the previous packet is not send yet");
        if (phystatus==phy_TX_ON)
            sendDown(p);
        else
            sendPacket = p;
    }

  protected:
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

    //void attachSignal(Ieee802154Frame* mac, simtime_t startTime);
    void manageMissingAck(t_mac_event event, cMessage *msg);
    void startTimer(t_mac_timer timer);

    virtual double scheduleBackoff();
    virtual void handleMessage(cMessage* msg);
    virtual void sendUp(cMessage *msg);

    virtual cPacket *decapsMsg(Ieee802154Frame * macPkt);
    Ieee802154Frame * ackMessage;

    //sequence number for sending, map for the general case with more senders
    //also in initialisation phase multiple potential parents
    //std::map<int, unsigned long> SeqNrParent; //parent -> sequence number
    std::map<int, uint8_t> SeqNrParent; //parent -> sequence number

    //sequence numbers for receiving
    //std::map<int, unsigned long> SeqNrChild; //child -> sequence number
    std::map<int, uint8_t> SeqNrChild; //child -> sequence number

};

#endif

