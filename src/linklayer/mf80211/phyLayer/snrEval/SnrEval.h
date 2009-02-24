/* -*- mode:c++ -*- ********************************************************
 * file:        SnrEval.h
 *
 * author:      Marc Loebbers
 *              Multi-channel support: Levente Meszaros, Andras Varga
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 ***************************************************************************/

#ifndef SNR_EVAL_H
#define SNR_EVAL_H

#include "BasicSnrEval.h"
#include "RadioState.h"
#include "PhyControlInfo_m.h"

/**
 * @brief Keeps track of the different snir levels when receiving a
 * packet
 *
 * This module keeps track of the noise level of the channel.
 *
 * When receiving a packet this module updates the noise level of the
 * channel. Based on the receive power of the packet it is processed
 * and handed to upper layers or just treated as noise.
 *
 * After the packet is completely received the snir information is
 * attached and it is handed to the decider module.
 *
 * The snir information is a SnrList that lists all different snr
 * levels together with the point of time (simTime()) when they
 * started.
 *
 * On top of that this module manages the RadioState, and posts notifications
 * on NotificationBoard whenever it changes. The radio state gives information
 * about whether this module is sending a packet, receiving a packet or idle.
 * This information can be accessed via the NotificationBoard by other modules,
 * e.g. a CSMAMacLayer.
 *
 * @author Marc Loebbers
 * @ingroup snrEval
 */
class INET_API SnrEval : public BasicSnrEval
{
  public:
    SnrEval();

    /** @brief change transmitter and receiver to a new channel.
      * This method throws an error if the radio state is transmit.
      * Messages that are already sent to the new channel and would
      * reach us in the future - thus they are on the air - will be
      * received correctly.
      */
    virtual void changeChannel(int channel);

    /** @brief change the bitrate to the given value.
      * This method throws an error if the radio state is transmit.
      */
    virtual void setBitrate(double bitrate);

  protected:
    /** @brief Initialize variables and publish the radio status*/
    virtual void initialize(int);

    virtual void finish();

    virtual ~SnrEval();

  protected:
    virtual void handleMessage(cMessage *msg);

    virtual void handleUpperMsg(AirFrame*);

    virtual void handleSelfMsg(cMessage*);

    virtual void handleCommand(int msgkind, cPolymorphic *ctrl);

    /** @brief Buffer the frame and update noise levels and snr information */
    virtual void handleLowerMsgStart(AirFrame*);

    /** @brief Unbuffer the frame and update noise levels and snr information*/
    virtual void handleLowerMsgEnd(AirFrame*);

    /** @brief Calculates the power with which a packet is received.*/
    virtual double calcRcvdPower(double pSend, double distance);

    /** Redefined from BasicSnrEval */
    virtual int getChannelNumber() const  {return rs.getChannelNumber();}

    /** @brief updates the snr information of the relevant AirFrames*/
    virtual void addNewSnr();

  protected:
    /** @brief Enum to store self message getKind()s*/
    enum
      {
        /** @brief timer to indicate that a message is completely sent now*/
        TRANSM_OVER
      };

    /**
     * @brief Struct to store a pointer to the message, rcvdPower AND a
     * SnrList, needed in SnrEval::addNewSnr
     */
    struct SnrStruct {
      /** @brief Pointer to the message this information belongs to*/
      AirFrame* ptr;
      /** @brief Received power of the message*/
      double rcvdPower;
      /** @brief Snr list to store the SNR values*/
      SnrList sList;
    };

    /**
     * @brief State: SnrInfo stores the snrList and the the recvdPower for the
     * message currently being received, together with a pointer to the
     * message.
     */
    SnrStruct snrInfo;

    /**
     * @brief Typedef used to store received messages together with
     * receive power.
     */
    typedef std::map<AirFrame*,double> RecvBuff;

    /**
     * @brief State: A buffer to store a pointer to a message and the related
     * receive power.
     */
    RecvBuff recvBuff;

    /** @brief State: the current RadioState of the NIC; includes channel number */
    RadioState rs;

    /** @brief State: if not -1, we have to switch to that channel once we finished transmitting */
    int newChannel;

    /** @brief State: if not -1, we have to switch to that bitrate once we finished transmitting */
    double newBitrate;

    /** @brief State: the current noise level of the channel.*/
    double noiseLevel;

    /**
     * @brief Configuration: The carrier frequency used. It is read from the ChannelControl module.
     */
    double carrierFrequency;

    /**
     * @brief Configuration: Thermal noise on the channel. Can be specified in
     * omnetpp.ini. Default: -100 dBm
     */
    double thermalNoise;

    /**
     * @brief Configuration: Defines up to what Power level (in dBm) a message can be
     * understood. If the level of a received packet is lower, it is
     * only treated as noise. Can be specified in omnetpp.ini. Default:
     * -85 dBm
     */
    double sensitivity;

    /**
     * @brief Configuration: Path loss coefficient. Can be specified in omnetpp.ini. If
     * not it is read from the ChannelControl module. This value CANNOT
     * be smaller than the one specified in the ChannelControl
     * module, or the simulation will exit with an error!
     */
    double pathLossAlpha;
};

#endif

