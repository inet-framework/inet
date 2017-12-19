//
// Copyright (C) 2017 Jan Peter Drees
// Copyright (C) 2015 Joaquim Oller
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

#ifndef __INET_XMACLAYER_H_
#define __INET_XMACLAYER_H_

#include <string>
#include <sstream>
#include <vector>
#include <list>

#include "inet/linklayer/xmac/XMacFrame_m.h"
#include "inet/common/INETDefs.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/linklayer/contract/IMACProtocol.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/linklayer/base/MACProtocolBase.h"

namespace inet {

using namespace physicallayer;

class MacPkt;

/**
 * @brief Implementation of X-MAC.
 * 
 * This implementation was created for the MiXiM framework by Joaquim Oller.
 * It was ported to the INET framework by Jan Peter Drees.
 *
 * A paper describing the X-MAC protocol can be found at:
 * http://www.cs.cmu.edu/~andersoe/papers/xmac-sensys.pdf
 * 
 * A paper analyzing this MiXiM implementation can be found at:
 * http://ieeexplore.ieee.org/document/7024195/
 *
 * @class XMacLayer
 * @ingroup macLayer
 * @author Joaquim Oller and Jan Peter Drees
 *
 */
class INET_API XMacLayer : public MACProtocolBase, public IMACProtocol
{
  private:
	/** @brief Copy constructor is not allowed.
	 */
	XMacLayer(const XMacLayer&);
	/** @brief Assignment operator is not allowed.
	 */
	XMacLayer& operator=(const XMacLayer&);

  public:
	XMacLayer()
		: MACProtocolBase()
		, macQueue()
		, nbTxDataPackets(0), nbTxPreambles(0), nbRxDataPackets(0), nbRxPreambles(0)
		, nbMissedAcks(0), nbRecvdAcks(0), nbDroppedDataPackets(0), nbTxAcks(0)
		, macState(INIT)
		, resend_data(NULL), ack_timeout(NULL), start_xmac(NULL), wakeup(NULL)
		, send_ack(NULL), cca_timeout(NULL), ack_tx_over(NULL), send_preamble(NULL), stop_preambles(NULL)
		, data_tx_over(NULL), data_timeout(NULL)
		, lastDataPktSrcAddr()
		, lastDataPktDestAddr()
		, txAttempts(0)
		, queueLength(0)
		, animation(false)
		, slotDuration(0), bitrate(0), checkInterval(0), txPower(0)
		, useMacAcks(0)
		, maxTxAttempts(0)
		, stats(false)
	{}
	virtual ~XMacLayer();

    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int) override;

    /** @brief Delete all dynamically allocated objects of the module*/
    virtual void finish() override;

    /** @brief Handle messages from lower layer */
    virtual void handleLowerPacket(cPacket *) override;

    /** @brief Handle messages from upper layer */
    virtual void handleUpperPacket(cPacket *) override;

    /** @brief Handle self messages such as timers */
    virtual void handleSelfMessage(cMessage *) override;

    void receiveSignal(cComponent *source, simsignal_t signalID, long value, cObject *details) override;

  protected:
    typedef std::list<XMacFrame *> MacQueue;

    /** implements MacBase functions */
    //@{
    virtual void flushQueue();
    virtual void clearQueue();
    virtual InterfaceEntry *createInterfaceEntry() override;
    //@}
    virtual void initializeMACAddress();

    /** @brief A queue to store packets from upper layer in case another
	packet is still waiting for transmission.*/
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
	long nbRxBrokenDataPackets;
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
	*  				and wiats for the actual data packet
	*  SEND_DATA -- node has sent enough preambles and sends the actual data
	*  				packet
	*  WAIT_TX_DATA_OVER -- node waits until the data packet sending is ready
	*  WAIT_ACK -- node has sent the data packet and waits for ack from the
	*  			   receiving node
	*  SEND_ACK -- node send an ACK back to the sender
	*  WAIT_ACK_TX -- node waits until the transmission of the ack packet is
	*  				  over
	*/
	enum States {
		INIT,	//0
		SLEEP,	//1
		CCA,	//2
		SEND_PREAMBLE, 	//3
		WAIT_DATA,		//4
		SEND_DATA,		//5
		WAIT_TX_DATA_OVER,	//6
		WAIT_ACK,       //~7: think this is not used
		SEND_ACK,		//8
		WAIT_ACK_TX		//9
	  };
	/** @brief The current state of the protocol */
	States macState;

	/** @brief Types of messages (self messages and packets) the node can
	 * process **/
	enum TYPES {
		// packet types
		XMAC_PREAMBLE = 191,
		XMAC_DATA,
		XMAC_ACK,
		// self message types
		XMAC_RESEND_DATA,
		XMAC_ACK_TIMEOUT,
		XMAC_START_XMAC,
		XMAC_WAKE_UP,
		XMAC_SEND_ACK,
		XMAC_CCA_TIMEOUT,   // 199
		XMAC_ACK_TX_OVER,
		XMAC_SEND_PREAMBLE,
		XMAC_STOP_PREAMBLES, // 202
		XMAC_DATA_TX_OVER,
		XMAC_DATA_TIMEOUT,
		SWITCH_PREAMBLE_PHASE, // 205
		DELAY_FOR_ACK_WITHIN_REMOTE_RX,
		XMAC_SWITCHING_FINISHED,
	};

	// messages used in the FSM
	cMessage *resend_data;
	cMessage *ack_timeout;
	cMessage *start_xmac;
	cMessage *wakeup;
	cMessage *send_ack;
	cMessage *cca_timeout;
	cMessage *ack_tx_over;
	cMessage *send_preamble;
	cMessage *switch_preamble_phase;
	cMessage *delay_for_ack_within_remote_rx;
	cMessage *stop_preambles;
	cMessage *data_tx_over;
	cMessage *data_timeout;
	cMessage *switching_done;

	/** @name Help variables for the acknowledgment process. */
	/*@{*/
	MACAddress lastDataPktSrcAddr;
	MACAddress lastDataPktDestAddr;
	MACAddress lastPreamblePktSrcAddr;
	int headerLength = 0;    // XMacFrame header length in bytes
	MACAddress address;    // MAC address

	/** @brief The radio. */
    IRadio *radio;
    IRadio::TransmissionState transmissionState = IRadio::TRANSMISSION_STATE_UNDEFINED;

	int              txAttempts;
	/*@}*/


	/** @brief Inspect reasons for dropped packets */
	static simsignal_t packetFromUpperDroppedSignal;

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
	/** @brief The bitrate of transmission */
	double bitrate;
	/** @brief The duration of CCA */
	double checkInterval;
	/** @brief Transmission power of the node */
	double txPower;
	/** @brief Use MAC level acks or not */
	bool useMacAcks;
	/** @brief Maximum transmission attempts per data packet, when ACKs are
	 * used */
	int maxTxAttempts;
	/** @brief Gather stats at the end of the simulation */
	bool stats;

	/** @brief Possible colors of the node for animation */
	enum XMAC_COLORS {
		GREEN = 1,
		BLUE = 2,
		RED = 3,
		BLACK = 4,
		YELLOW = 5
	};

	/** @brief Internal function to change the color of the node */
	void changeDisplayColor(XMAC_COLORS color);

	/** @brief Internal function to send the first packet in the queue */
	void sendDataPacket();

	/** @brief Internal function to send an ACK */
	void sendMacAck();

	/** @brief Internal function to send one preamble */
	void sendPreamble(MACAddress destination);

	/** @brief Internal function to attach a signal to the packet */
	void attachSignal(XMacFrame *mac, simtime_t_cref startTime);

	/** @brief Internal function to add a new packet from upper to the queue */
	bool addToQueue(cMessage * msg);

	cPacket *decapsMsg(XMacFrame *macPkt);
	cObject *setUpControlInfo(cMessage *const pMsg, const MACAddress& pSrcAddr);
};

} // namespace inet

#endif /* XMACLAYER_H_ */
