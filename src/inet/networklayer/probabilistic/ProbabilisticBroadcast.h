/*
 * ProbabilisticBroadcast.h
 *
 *  Created on: Nov 4, 2008
 *      Author: Damien Piguet
 */

#ifndef __INET_PROBABILISTICBROADCAST_H
#define __INET_PROBABILISTICBROADCAST_H

#include <map>
#include <set>

#include "inet/common/packet/Packet.h"
#include "inet/networklayer/base/NetworkProtocolBase.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/contract/INetworkProtocol.h"
#include "inet/networklayer/probabilistic/ProbabilisticBroadcastHeader_m.h"

namespace inet {

/**
 * @brief This class offers a data dissemination service using
 *        probabilistic broadcast. Each packet which arrives from
 *        upper layer or from the network is (re-)transmitted n
 *        times with n = floor(TTL/bcperiod) with probability beta.
 *
 * @ingroup netwLayer
 * @author Damien Piguet
 **/
class INET_API ProbabilisticBroadcast : public NetworkProtocolBase, public INetworkProtocol
{
  private:
    /** @brief Copy constructor is not allowed.
     */
    ProbabilisticBroadcast(const ProbabilisticBroadcast&);
    /** @brief Assignment operator is not allowed.
     */
    ProbabilisticBroadcast& operator=(const ProbabilisticBroadcast&);

  public:
    ProbabilisticBroadcast()
        : NetworkProtocolBase()
        , broadcastPeriod()
        , beta(0)
        , timeToLive()
        , maxNbBcast(0)
        , maxFirstBcastBackoff(0)
        , timeInQueueAfterDeath(0)
        , headerLength(0)
        , broadcastTimer(nullptr)
        , knownMsgIds()
        , msgQueue()
        , debugMsgIdSet()
        , nbDataPacketsReceived(0)
        , nbDataPacketsSent(0)
        , nbHops(0)
        , debugNbMessageKnown(0)
        , nbDataPacketsForwarded(0)
        , oneHopLatencies()
    {}

    /** @brief Initialization of the module and some variables*/
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    virtual void initialize(int) override;

    virtual void finish() override;

    const Protocol& getProtocol() const override { return Protocol::probabilistic; }

  protected:
    enum messagesTypes {
        UNKNOWN = 0,
        BROADCAST_TIMER,
        NEIGHBOR_TIMER,
        BETA_TIMER,
    };

    /** @brief Store messages in a structure so that we can keep some
     *         information needed by the protocol
     **/
    typedef struct tMsgDesc
    {
        Packet *pkt;
        int nbBcast;    // number of times the present node has passed the
                        // message through a broadcast attempt.
        bool initialSend;    // true if message to be sent for first
                             // time by its creator.
    } tMsgDesc;

    typedef std::set<unsigned int> MsgIdSet;
    typedef std::multimap<simtime_t, tMsgDesc *> TimeMsgMap;

    /** @brief Handle messages from upper layer */
    virtual void handleUpperPacket(Packet *packet) override;

    /** @brief Handle messages from lower layer */
    virtual void handleLowerPacket(Packet *packet) override;

    /** @brief Handle self messages */
    virtual void handleSelfMessage(cMessage *msg) override;

    /** @brief Checks whether a message is known (= kept in memory) or not */
    virtual bool messageKnown(unsigned int msgId);

    /** @brief Checks whether a message is known (= kept in memory) or not */
    virtual bool debugMessageKnown(unsigned int msgId);

    /** @brief Insert a message in both known ID list and message queue.
     *         If the message comes in front of the queue (i.e. it will be the
     *         next one to be broadcasted, the broadcastTimer is reset accordingly.
     *  @param bcastDelay relative (to now) simulator time of next broadcast attempt.
     *  @param msg descriptor of the message to insert in the queue.
     **/
    virtual void insertMessage(simtime_t_cref bcastDelay, tMsgDesc *msgDesc);

    /** @brief Returns the descriptor of the first message in the queue,
     *         then remove its pointer from the queue and its id from the
     *         known IDs list. Then re-schedule the broadcastTimer to the
     *         broadcast instant of the new first element in the list.
     **/
    virtual tMsgDesc *popFirstMessageUpdateQueue(void);

    /** @brief Returns a network layer packet which encapsulates the upper layer
     *         packet passed to the function.
     **/
    virtual void encapsulate(Packet *packet);

    /** @brief extracts and returns the application layer packet which is encapsulated
     *         in the network layer packet given in argument, delete network layer packet.
     **/
    virtual void decapsulate(Packet *packet);

    /** @brief Insert a new message in both known ID list and message queue.
     *         The message comes either from upper layer or from lower layer.
     *         In both cases, it will be inserted in the queue with a broadcast
     *         attempt delay taken uniformly between 0 and min(broadcast period, TTL)
     *         in order to implement a random backoff for the first broadcast of
     *         a message.
     *  @param msgDesc descriptor of the message to insert in the queue.
     *  @param iAmInitialSender message comes from upper layer, I am its creator
     *                          and initial sender.
     **/
    virtual void insertNewMessage(Packet *packet, bool iAmInitialSender = false);

    /**
     * @brief Attaches a "control info" (NetwToMac) structure (object) to the message pMsg.
     *
     * This is most useful when passing packets between protocol layers
     * of a protocol stack, the control info will contain the destination MAC address.
     *
     * The "control info" object will be deleted when the message is deleted.
     * Only one "control info" structure can be attached (the second
     * setL3ToL2ControlInfo() call throws an error).
     *
     * @param pMsg      The message where the "control info" shall be attached.
     * @param pDestAddr The MAC address of the message receiver.
     */
    virtual void setDownControlInfo(Packet *const pMsg, const MacAddress& pDestAddr);

    // OperationalBase:
    virtual void handleStartOperation(LifecycleOperation *operation) override {}    //TODO implementation
    virtual void handleStopOperation(LifecycleOperation *operation) override {}    //TODO implementation
    virtual void handleCrashOperation(LifecycleOperation *operation) override {}    //TODO implementation

    /**
     * @brief Period (in sim time) between two broadcast attempts.
     * Read from omnetpp.ini
     **/
    simtime_t broadcastPeriod;

    /**
     * @brief Probability of each broadcast attempt.
     * Read from omnetpp.ini
     **/
    double beta;

    /*
     * @brief Default time to live for packets to send.
     */
    simtime_t timeToLive;

    static long id_counter;

    static long getNextID()
    {
        long nextID = id_counter;
        id_counter++;
        return nextID;
    }

    /**
     * @brief Maximal number of broadcast attempts for each packet.
     * Read from omnetpp.ini
     **/
    int maxNbBcast;

    /**
     * @brief Maximal back-off before first broadcast attempt [seconds].
     * Read from omnetpp.ini
     **/
    simtime_t maxFirstBcastBackoff;

    /**
     * @brief How long the message should be kept in queue after its died.
     *        That way the message is known if the node receives one of its
     *        copy that isn't dead because of TTL de-synchronization due to
     *        MAC backoff, propagation delay and clock drift.
     * Read from omnetpp.ini
     **/
    simtime_t timeInQueueAfterDeath;

    /**
     * @brief Length of the NetwPkt header
     * Read from omnetpp.ini
     **/
    int headerLength;

    // perform broadcast attempt for the first message in the list each time it expires
    cMessage *broadcastTimer;

    // we use two containers: a set which stores the ID's of the messages which are kept
    // in memory and a multimap which stores a pair <Key, Value> where Key is the next
    // broadcasting attempt time of the message and Value is a pointer to the message
    // (see typedef's above).
    MsgIdSet knownMsgIds;
    TimeMsgMap msgQueue;
    MsgIdSet debugMsgIdSet;

    // variables for statistics
    long nbDataPacketsReceived;    // total number of received packets from lower layer
    long nbDataPacketsSent;
    long nbHops;
    int debugNbMessageKnown;
    long nbDataPacketsForwarded;
    // records the time packets take between submission to MAC layer and reception at
    // networking layer (over one hop).
    cOutVector oneHopLatencies;

    L3Address myNetwAddr;
};

} // namespace inet

#endif // ifndef __INET_PROBABILISTICBROADCAST_H

