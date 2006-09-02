//
// Copyright (C) Andras Varga, Levente Meszaros
// Based on the Mobility Framework's SnrEval by Marc Loebbers
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef ABSTRACTRADIO_H
#define ABSTRACTRADIO_H

#include "ChannelAccess.h"
#include "RadioState.h"
#include "AirFrame_m.h"
#include "PhyControlInfo_m.h"

//FIXME docu

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
class INET_API AbstractRadio : public ChannelAccess
{
  public:
    AbstractRadio();

    /** @brief change transmitter and receiver to a new channel.
      * This method throws an error if the radio state is transmit.
      * Messages that are already sent to the new channel and would
      * reach us in the future - thus they are on the air - will be
      * received correctly.
      */
    void changeChannel(int channel);

  protected:
    /** @brief Initialize variables and publish the radio status*/
    virtual void initialize(int);

    virtual void finish();

    virtual ~AbstractRadio();

  protected:
    void handleMessage(cMessage *msg);

    virtual void handleUpperMsg(AirFrame*);

    virtual void handleSelfMsg(cMessage*);

    virtual void handleCommand(int msgkind, cPolymorphic *ctrl);

    /** @brief Buffer the frame and update noise levels and snr information */
    virtual void handleLowerMsgStart(AirFrame*);

    /** @brief Unbuffer the frame and update noise levels and snr information*/
    virtual void handleLowerMsgEnd(AirFrame*);

    /** @brief Buffers message for 'transmission time'*/
    void bufferMsg(AirFrame *frame);

    /** @brief Unbuffers a message after 'transmission time'*/
    AirFrame* unbufferMsg(cMessage *msg);

    /** @brief Sends a message to the upper layer*/
    void sendUp(AirFrame*, SnrList&);

    /** @brief Sends a message to the channel*/
    void sendDown(AirFrame *msg);

    /** @brief Encapsulates a MAC frame into an Air Frame*/
    virtual AirFrame *encapsMsg(cMessage *msg);

    /**
     * Should be defined to calculate the duration of the AirFrame.
     * Usually the duration is just the frame length divided by the
     * bitrate. However there may be cases (like 802.11) where the header
     * has a different modulation (and thus a different bitrate) than the
     * rest of the message.
     */
    virtual double calcDuration(AirFrame *) = 0;

    /**
     * Should be defined to calculates the power with which a packet is received.
     */
    virtual double calcRcvdPower(double pSend, double distance) = 0;

    /** Redefined from BasicRadio */
    virtual int channelNumber() const  {return rs.getChannel();}

    /** @brief updates the snr information of the relevant AirFrames*/
    void addNewSnr();

    /** @brief Create a new AirFrame */
    virtual AirFrame* createCapsulePkt() {return new AirFrame();};

  protected:
    double bitrate;
    int headerLength;

    /** @brief power used to transmit messages */
    double transmitterPower;

    /** @brief gate id*/
    /*@{*/
    int uppergateOut;
    int uppergateIn;
    /*@}*/

    /**
     * @brief Struct to store a pointer to the message, rcvdPower AND a
     * SnrList, needed in AbstractRadio::addNewSnr
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

