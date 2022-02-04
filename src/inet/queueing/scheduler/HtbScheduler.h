// Hierarchical Token Bucket Implementation for OMNeT++ & INET Framework
// Copyright (C) 2021 Marija Gajić (NTNU), Marcin Bosk (TUM), Susanna Schwarzmann (TU Berlin), Stanislav Lange (NTNU), and Thomas Zinner (NTNU)
// 
// SPDX-License-Identifier: GPL-2.0-or-later
//
//
//
// This implementation is heavily based on the implementation of Linux HTB qdisc by Martin Devera (https://github.com/torvalds/linux/blob/master/net/sched/sch_htb.c)
// Code base taken from the "PriorityScheduler"
//

#ifndef __INET_HTBSCHEDULER_H
#define __INET_HTBSCHEDULER_H

#include <string>
#include <set>
#include <vector>
#include <array>
#include <cstdlib>
#include <algorithm>
#include <functional>

#include "inet/common/XMLUtils.h"
#include "inet/queueing/base/PacketSchedulerBase.h"
#include "inet/queueing/contract/IPacketCollection.h"

namespace inet {
namespace queueing {

class INET_API HtbScheduler : public PacketSchedulerBase, public IPacketCollection, public cListener
{
  protected:
    simsignal_t dequeueIndexSignal;
    std::vector<IPacketCollection *> collections; // The actual queues

    static const int maxHtbDepth = 8; // The maximal amount of levels of htb tree
    static const int maxHtbNumPrio = 8; // The maximal number of priorities
    static const int can_send = 0; // The class is in can send mode (green)
    static const int may_borrow = 1; // The class is in may borrow mode (yellow)
    static const int cant_send = 2; // The class is in can't send mode (red)

    bool htb_hysteresis; // Use hysteresis determine class mode

    double linkDatarate; // The datarate of connected link

    long long mtu;
    int phyHeaderSize;
    bool valueCorectnessCheck;
    bool valueCorectnessAdj;

    int lastGlobalIdUsed = 0;

    // Structure for the class
    struct htbClass {
        const char *name = "";
        long long assignedRate = 0; // assured rate
        long long ceilingRate = 0; // ceiling rate
        long long burstSize = 0;
        long long cburstSize = 0;

        int quantum = 0;
        long long mbuffer = 0;

        simtime_t checkpointTime = 0; // Time of last token update
        int level = 0; // Level within the tree. !!!!!! ALWAYS 0 = LEAF !!!!!!
        int numChildren = 0; // Number of connected children

        htbClass *parent = NULL; // Pointer to parent node. NULL for root
        // We are only keeping information about the parent. There is usually no need to traverse the tree from the root.
        // We only need to traverse it from the top when there are classes in the inner feeds, which then store the pointers to children

        // The token buckets
        long long tokens = 0; // For assured rate
        long long ctokens = 0; // For ceiling rate

        // The mode of the class
        int mode = can_send;

        // Specifies for which priorities the class is now active and in green/yellow mode
        bool activePriority[maxHtbNumPrio] = { false };

        // If class not in green mode, it specifies when the next mode change is expected
        simtime_t nextEventTime = 0;

        struct htbClassLeaf { // Special class information for leaf
            int priority;
            int deficit[maxHtbDepth]; // Used for deficit round robin (DRR)
            simsignal_t deficitSig[maxHtbDepth]; // Signal for queue level statistics collection
            int queueId; // Id of the corresponding queue
        } leaf;
        struct htbClassSetComp { // Comparator to sort the waiting classes according to their expected mode change time
            bool operator()(htbClass* const & a, htbClass* const & b) const {
                // if (a->name == b->name) {
                //     return false;
                // }
                // return a->name < b->name;
                return a->classId < b->classId;
            }
        };
        struct htbClassInner { // Special class information for inner/root
            std::set<htbClass*, htbClassSetComp> innerFeeds[maxHtbNumPrio]; // Inner feeds of inner/root class
            htbClass* nextToDequeue[maxHtbNumPrio]; // DRR -> which class to dequeue next from inner feed
        } inner;

        // Statistics collection signals
        simsignal_t tokenBucket;
        simsignal_t ctokenBucket;
        simsignal_t classMode;

        int classId; // Uniquie, deterministic, ID for class sorting
    };

    struct waitComp { // Comparator to sort the waiting classes according to their expected mode change time
        bool operator()(htbClass* const & a, htbClass* const & b) const {
            if (a->nextEventTime == b->nextEventTime) {
                // if (a->name == b->name) {
                //     return false;
                // }
                // return a->name < b->name;
                return a->classId < b->classId;
                // return std::less<htbClass*>{}(a, b); // Care for DRR ordering
            }
            return a->nextEventTime < b->nextEventTime;
        }
    };

    // Struct for the tree level
    struct htbLevel {
        unsigned int levelId; // Level number. 0 = Leaf
        std::set<htbClass*, htbClass::htbClassSetComp> selfFeeds[maxHtbNumPrio]; // Self feeds for each priority. Contain active green classes
        htbClass* nextToDequeue[maxHtbNumPrio]; // Next green class to dequeue on level
        std::multiset<htbClass*, waitComp> waitingClasses; // Red classes waiting to become non-red.
    };

    htbLevel* levels[maxHtbDepth]; // Levels saved here

    cXMLElement *htbConfig = nullptr; // XML config for the htb tree

    htbClass* rootClass; // Root class saved here
    std::vector<htbClass*> innerClasses; // Inner classes saved here for ease of access
    std::vector<htbClass*> leafClasses; // Leaf classes saved here for ease of access

    cMessage *classModeChangeEvent = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual int schedulePacket() override;
    virtual void handleMessage(cMessage *msg) override;

  public:
    int classMode(htbClass *cl, long long *diff);
    virtual int getMaxNumPackets() const override { return -1; }
    virtual int getNumPackets() const override;
    virtual bool canPullSomePacket(cGate *gate) const override;
    virtual Packet *canPullPacket(cGate *gate) const override { throw cRuntimeError("The canPullPacket() not implemented yet."); }

    virtual b getMaxTotalLength() const override { return b(-1); }
    virtual b getTotalLength() const override;

    virtual bool isEmpty() const override { return getNumPackets() == 0; }
    virtual Packet *getPacket(int index) const override;
    virtual void removePacket(Packet *packet) override;
    virtual void removeAllPackets() override;

    void htbEnqueue(int index);
    int htbDequeue(int priority, int level);
    void printClass(htbClass *cl);
    void printLevel(htbLevel *level, int index);
    void printInner(htbClass *cl);
    htbClass *createAndAddNewClass(cXMLElement* oneClass, int queueId);
    void activateClass(htbClass *cl, int priority);
    void deactivateClass(htbClass *cl, int priority);

    void activateClassPrios(htbClass *cl);
    void deactivateClassPrios(htbClass *cl);

    inline long htb_hiwater(htbClass *cl);
    inline long htb_lowater(htbClass *cl);

    void updateClassMode(htbClass *cl, long long *diff);
    void accountTokens(htbClass *cl, long long bytes, long long diff);
    void accountCTokens(htbClass *cl, long long bytes, long long diff);
    void chargeClass(htbClass *leafCl, int borrowLevel, Packet *packetToDequeue);

    void htbAddToWaitTree(htbClass *cl, long long delay);
    void htbRemoveFromWaitTree(htbClass *cl);
    htbClass *getLeaf(int priority, int level);
    simtime_t doEvents(int level);

    virtual void scheduleAt(simtime_t t, cMessage *msg) override;
    virtual cMessage *cancelEvent(cMessage *msg) override;

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace queueing
} // namespace inet

#endif

