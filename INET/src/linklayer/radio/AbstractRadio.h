//
// Copyright (C) 2006 Andras Varga, Levente Meszaros
// Based on the Mobility Framework's SnrEval by Marc Loebbers
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

#ifndef ABSTRACTRADIO_H
#define ABSTRACTRADIO_H

#include "ChannelAccess.h"
#include "RadioState.h"
#include "AirFrame_m.h"
#include "IRadioModel.h"
#include "IReceptionModel.h"
#include "SnrList.h"



/**
 * Abstract base class for radio modules. Radio modules deal with the
 * transmission of frames over a wireless medium (the radio channel).
 * See the Radio module's NED documentation for an overview of radio modules.
 *
 * This class implements common functionality of the radio, and abstracts
 * out specific parts into two interfaces, IReceptionModel and IRadioModel.
 * The reception model is responsible for modelling path loss, interference
 * and antenna gain. The radio model is responsible for calculating frame
 * duration, and modelling modulation scheme and possible forward error
 * correction. Subclasses have to redefine the <tt>createReceptionModel()</tt>
 * and <tt>createRadioModel()</tt> methods to create and return appropriate
 * reception model and radio model objects.
 *
 * <b>History</b>
 *
 * The implementation is largely based on the Mobility Framework's
 * SnrEval and Decider modules. They have been merged into a single
 * module, multi-channel support, runtime channel and bitrate switching
 * capability added, and all code specific to the physical channel
 * and radio characteristics have been factored out into the IReceptionModel
 * and IRadioModel classes.
 *
 * @author Andras Varga, Levente Meszaros
 */
class INET_API AbstractRadio : public ChannelAccess
{
  public:
    AbstractRadio();
    virtual ~AbstractRadio();

  protected:
    virtual void initialize(int stage);
    virtual void finish();

    virtual void handleMessage(cMessage *msg);

    virtual void handleUpperMsg(AirFrame*);

    virtual void handleSelfMsg(cMessage*);

    virtual void handleCommand(int msgkind, cPolymorphic *ctrl);

    /** @brief Buffer the frame and update noise levels and snr information */
    virtual void handleLowerMsgStart(AirFrame *airframe);

    /** @brief Unbuffer the frame and update noise levels and snr information */
    virtual void handleLowerMsgEnd(AirFrame *airframe);

    /** @brief Buffers message for 'transmission time' */
    virtual void bufferMsg(AirFrame *airframe);

    /** @brief Unbuffers a message after 'transmission time' */
    virtual AirFrame *unbufferMsg(cMessage *msg);

    /** Sends a message to the upper layer */
    virtual void sendUp(AirFrame *airframe);

    /** Sends a message to the channel */
    virtual void sendDown(AirFrame *airframe);

    /** Encapsulates a MAC frame into an Air Frame */
    virtual AirFrame *encapsulatePacket(cPacket *msg);

    /** Sets the radio state, and also fires change notification */
    virtual void setRadioState(RadioState::State newState);

    /** Returns the current channel the radio is tuned to */
    virtual int getChannelNumber() const {return rs.getChannelNumber();}

    /** Updates the SNR information of the relevant AirFrame */
    virtual void addNewSnr();

    /** Create a new AirFrame */
    virtual AirFrame *createAirFrame() {return new AirFrame();}

    /**
     * Change transmitter and receiver to a new channel.
     * This method throws an error if the radio state is transmit.
     * Messages that are already sent to the new channel and would
     * reach us in the future - thus they are on the air - will be
     * received correctly.
     */
    virtual void changeChannel(int channel);

    /**
     * Change the bitrate to the given value. This method throws an error
     * if the radio state is transmit.
     */
    virtual void setBitrate(double bitrate);

    /**
     * To be defined to provide a reception model. The reception model
     * is responsible for modelling path loss, interference and antenna
     * gain.
     */
    virtual IReceptionModel *createReceptionModel() = 0;

    /**
     * To be defined to provide a radio model. The radio model is
     * responsible for calculating frame duration, and modelling modulation
     * scheme and possible forward error correction.
     */
    virtual IRadioModel *createRadioModel() = 0;

  protected:
    IRadioModel *radioModel;
    IReceptionModel *receptionModel;

    /** Power used to transmit messages */
    double transmitterPower;

    /** @name Gate Ids */
    //@{
    int uppergateOut;
    int uppergateIn;
    //@}

    /**
     * Struct to store a pointer to the message, rcvdPower AND a SnrList,
     * needed in addNewSnr().
     */
    struct SnrStruct
    {
        AirFrame *ptr;    ///< pointer to the message this information belongs to
        double rcvdPower; ///< received power of the message
        SnrList sList;    ///< stores SNR over time
    };

    /**
     * State: SnrInfo stores the snrList and the the recvdPower for the
     * message currently being received, together with a pointer to the
     * message.
     */
    SnrStruct snrInfo;

    /**
     * Typedef used to store received messages together with
     * receive power.
     */
    typedef std::map<AirFrame*,double> RecvBuff;

    /**
     * State: A buffer to store a pointer to a message and the related
     * receive power.
     */
    RecvBuff recvBuff;

    /** State: the current RadioState of the NIC; includes channel number */
    RadioState rs;

    /** State: if not -1, we have to switch to that channel once we finished transmitting */
    int newChannel;

    /** State: if not -1, we have to switch to that bitrate once we finished transmitting */
    double newBitrate;

    /** State: the current noise level of the channel.*/
    double noiseLevel;

    /**
     * Configuration: The carrier frequency used. It is read from the ChannelControl module.
     */
    double carrierFrequency;

    /**
     * Configuration: Thermal noise on the channel. Can be specified in
     * omnetpp.ini. Default: -100 dBm
     */
    double thermalNoise;

    /**
     * Configuration: Defines up to what Power level (in dBm) a message can be
     * understood. If the level of a received packet is lower, it is
     * only treated as noise. Can be specified in omnetpp.ini. Default:
     * -85 dBm
     */
    double sensitivity;
};

#endif

