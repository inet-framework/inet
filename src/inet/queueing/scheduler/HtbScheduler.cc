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

#include <stdlib.h>
#include <string.h>

#include "inet/queueing/scheduler/HtbScheduler.h"

#include "inet/common/XMLUtils.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {
namespace queueing {

Define_Module(HtbScheduler);

void HtbScheduler::printClass(htbClass *cl)
{
    EV_INFO << "Class: " << cl->name << endl;
//    EV_INFO << "   - assigned rate: " << cl->assignedRate << endl;
//    EV_INFO << "   - ceiling rate: " << cl->ceilingRate << endl;
    EV_INFO << "   - burst size: " << cl->burstSize << endl;
    EV_INFO << "   - cburst size: " << cl->cburstSize << endl;
//    EV_INFO << "   - quantum: " << cl->quantum << endl;
//    EV_INFO << "   - mbuffer: " << cl->mbuffer << endl;
    EV_INFO << "   - checkpoint time: " << cl->checkpointTime << endl;
    EV_INFO << "   - level: " << cl->level << endl;
    EV_INFO << "   - number of children: " << cl->numChildren << endl;
    if (cl->parent != NULL) {
        EV_INFO << "   - parent class name: " << cl->parent->name << endl;
    }
    EV_INFO << "   - current tokens: " << cl->tokens << endl;
    EV_INFO << "   - current ctokens: " << cl->ctokens << endl;
    EV_INFO << "   - current class mode: " << cl->mode << endl;

    char actPrios[2 * maxHtbNumPrio + 1];

    for (int i = 0; i < maxHtbNumPrio; i++) {
        actPrios[2 * i] = cl->activePriority[i] ? '1' : '0';
        actPrios[2 * i + 1] = ';';
    }
    actPrios[2*maxHtbNumPrio] = '\0';

    EV_INFO << "   - active priorities: " << actPrios << endl;

    if (strstr(cl->name, "leaf")) {
        EV_INFO << "   - leaf priority: " << cl->leaf.priority << endl;
        int leafNumPackets = collections[cl->leaf.queueId]->getNumPackets();
        EV_INFO << "   - current queue level: " << leafNumPackets << endl;
        EV_INFO << "   - queue num: " << cl->leaf.queueId << endl;
    }
}

// Gets information from XML config, creates a class (leaf/inner/root) and correctly adds it to tree structure
HtbScheduler::htbClass *HtbScheduler::createAndAddNewClass(cXMLElement* oneClass, int queueId)
{
    // Create class, set its name and parents' name
    htbClass* newClass = new htbClass();
    lastGlobalIdUsed += 1;
    newClass->classId = lastGlobalIdUsed;
    newClass->name = oneClass->getAttribute("id");
    const char* parentName = oneClass->getFirstChildWithTag("parentId")->getNodeValue();

    // Configure class settings - rate, ceil, burst, cburst, quantum, etc.
    long long rate = atoi(oneClass->getFirstChildWithTag("rate")->getNodeValue()) * 1e3;
    newClass->assignedRate = rate;
    long long ceil = atoi(oneClass->getFirstChildWithTag("ceil")->getNodeValue()) * 1e3;
    newClass->ceilingRate = ceil;

    // The user has an option to configure burst/cburst manually, or it can be configured automatically
    long long burstTemp = 0;
    long long cburstTemp = 0;
    if (oneClass->getFirstChildWithTag("burst") != nullptr) {
        burstTemp = atoi(oneClass->getFirstChildWithTag("burst")->getNodeValue()); // Burst specified in Bytes
        if (burstTemp < mtu) {
            throw cRuntimeError("Class %s burst of %llu Bytes cannot be smaller than MTU of %llu Bytes! Error regardless of the check corecctness flag!", newClass->name, burstTemp, mtu);
        }
        else if (burstTemp < rate / 8000) {
            EV_WARN << "Class " << newClass->name << " burst might be too small for optimal performance! Recommend setting to at least rate/8000 to allow for 1ms burst time." << endl;
            if (valueCorectnessCheck == true) {
                throw cRuntimeError("Class %s burst of %llu Bytes is smaller than minimum recommended for optimal performance of %llu Bytes!", newClass->name, burstTemp, rate / 8000);
            }
        }
    }
    else {
        burstTemp = std::max(rate / 8000, mtu);
        EV_INFO << "User did not specify burst. Configuring automatically to: " << burstTemp << " Bytes." << endl;
    }
    if (oneClass->getFirstChildWithTag("cburst") != nullptr) {
        cburstTemp = atoi(oneClass->getFirstChildWithTag("cburst")->getNodeValue()); // Cburst specified in Bytes
        if (cburstTemp < mtu) {
            throw cRuntimeError("Class %s cburst of %llu Bytes cannot be smaller than MTU of %llu Bytes! Error regardless of the check corecctness flag!", newClass->name, cburstTemp, mtu);
        }
        else if (cburstTemp < ceil / 8000) {
            EV_WARN << "Class " << newClass->name << " cburst might be too small for optimal performance! Recommend setting to at least ceil/8000 to allow for 1ms burst time." << endl;
            if (valueCorectnessCheck == true) {
                throw cRuntimeError("Class %s cburst of %llu Bytes is smaller than minimum recommended for optimal performance of %llu Bytes!", newClass->name, cburstTemp, ceil / 8000);
            }
        }
    }
    else {
        cburstTemp = std::max(ceil / 8000, mtu);
        EV_INFO << "User did not specify cburst. Configuring automatically to: " << cburstTemp << " Bytes." << endl;
    }

    if (linkDatarate == -1) {
        throw cRuntimeError("Link datarate was -1!");
    }

    // https://patchwork.ozlabs.org/project/netdev/patch/200901221145.57856.denys@visp.net.lb/ -> Defines that burst/cburst should be at least mtu + 1ms worth of sending at rate/ceil
    // Determine and calculate the burstSize and cburstSize
    long long burstChosen = burstTemp;
    long long cburstChosen = cburstTemp;
    if (valueCorectnessAdj == true) {
        if (burstTemp < rate / 8000) {
            burstChosen = std::max(burstTemp, rate / 8000); // Burst should not be smaller than mtu + 1ms worth of sending at rate
            EV_WARN << "Burst adjusted to " << burstChosen << "Bytes" << endl;
        }
        if (cburstTemp < ceil / 8000) {
            cburstChosen = std::max(cburstTemp, ceil / 8000); // Cburst should not be smaller than mtu + 1ms worth of sending at ceil
            EV_WARN << "Cburst adjusted to " << cburstChosen << "Bytes" << endl;
        }
    }

    long long burst = (((long long) burstChosen * 8 * 1e+9)/(long long)rate);
    long long cburst = (((long long) cburstChosen * 8 * 1e+9)/(long long)ceil);

    newClass->burstSize = burst;
    newClass->cburstSize = cburst;
    int level = atoi(oneClass->getFirstChildWithTag("level")->getNodeValue()); // Level in the tree structure. 0 = LEAF!!!
    newClass->level = level;
    int quantum = atoi(oneClass->getFirstChildWithTag("quantum")->getNodeValue());
    if (valueCorectnessCheck == true && quantum < mtu) {
        throw cRuntimeError("Class %s quantum of %d Bytes is smaller than minimum recommended  of %llu Bytes!", newClass->name, quantum, mtu);
    }
    if (valueCorectnessAdj == true && quantum < mtu) {
        quantum = mtu;
    }
    newClass->quantum = quantum;
    long long mbuffer = atoi(oneClass->getFirstChildWithTag("mbuffer")->getNodeValue()) * 1e9;
    newClass->mbuffer = mbuffer;
    newClass->checkpointTime = simTime(); // Initialize to now. It says when was the last time the tokens were updated.
    newClass->tokens = burst;
    newClass->ctokens = cburst;

    // Handle different types of classes:
    if (strstr(newClass->name, "inner")) { // INNER
        if (strstr(parentName, "root")) {
            newClass->parent = rootClass;
            rootClass->numChildren += 1;
            innerClasses.push_back(newClass);
        }
        else if (strstr(parentName, "inner")) {
            for (auto innerCl : innerClasses) {
                if (!strcmp(innerCl->name, parentName)) {
                    newClass->parent = innerCl;
                    innerCl->numChildren += 1;
                    innerClasses.push_back(newClass);
                }
            }
        }
    }
    else if (strstr(newClass->name, "leaf")) { // LEAF
        if (strstr(parentName, "root")) {
            newClass->parent = rootClass;
            rootClass->numChildren += 1;
            leafClasses.push_back(newClass);
        }
        else if (strstr(parentName, "inner")) {
            for (auto innerCl : innerClasses) {
                if (!strcmp(innerCl->name, parentName)) {
                    newClass->parent = innerCl;
                    innerCl->numChildren += 1;
                    leafClasses.push_back(newClass);
                }
            }
        }
        memset(newClass->leaf.deficit, 0, sizeof(newClass->leaf.deficit));
//        newClass->leaf.queueLevel = 0;
        newClass->leaf.queueId = atoi(oneClass->getFirstChildWithTag("queueNum")->getNodeValue());
        newClass->leaf.priority = atoi(oneClass->getFirstChildWithTag("priority")->getNodeValue());

        // Statistics collection for deficit
        for (int i = 0; i < maxHtbDepth; i++) {
            char deficitSignalName[50];
            sprintf(deficitSignalName, "class-%s-deficit%d", newClass->name, i);
            newClass->leaf.deficitSig[i] = registerSignal(deficitSignalName);
            char deficitStatisticName[50];
            sprintf(deficitStatisticName, "class-%s-deficit%d", newClass->name, i);
            cProperty *deficitStatisticTemplate = getProperties()->get("statisticTemplate", "deficit");
            getEnvir()->addResultRecorders(this, newClass->leaf.deficitSig[i], deficitStatisticName, deficitStatisticTemplate);
            emit(newClass->leaf.deficitSig[i], newClass->leaf.deficit[i]);
        }
    }
    else if (strstr(newClass->name, "root")) { // ROOT
        newClass->parent = NULL;
        rootClass = newClass;
    }
    else {
        newClass->parent = NULL;
    }

    // Statistics collection is created dynamically for each class:
    // Statistics collection for token levels
    char tokenLevelSignalName[50];
    sprintf(tokenLevelSignalName, "class-%s-tokenLevel", newClass->name);
    newClass->tokenBucket = registerSignal(tokenLevelSignalName);
    char tokenLevelStatisticName[50];
    sprintf(tokenLevelStatisticName, "class-%s-tokenLevel", newClass->name);
    cProperty *tokenLevelStatisticTemplate = getProperties()->get("statisticTemplate", "tokenLevel");
    getEnvir()->addResultRecorders(this, newClass->tokenBucket, tokenLevelStatisticName, tokenLevelStatisticTemplate);
    emit(newClass->tokenBucket, (long) newClass->tokens);

    // Statistics collection for ctoken levels
    char ctokenLevelSignalName[50];
    sprintf(ctokenLevelSignalName, "class-%s-ctokenLevel", newClass->name);
    newClass->ctokenBucket = registerSignal(ctokenLevelSignalName);
    char ctokenLevelStatisticName[50];
    sprintf(ctokenLevelStatisticName, "class-%s-ctokenLevel", newClass->name);
    cProperty *ctokenLevelStatisticTemplate = getProperties()->get("statisticTemplate", "ctokenLevel");
    getEnvir()->addResultRecorders(this, newClass->ctokenBucket, ctokenLevelStatisticName, ctokenLevelStatisticTemplate);
    emit(newClass->ctokenBucket, (long) newClass->ctokens);

    // Statistics collection for class mode
    char classModeSignalName[50];
    sprintf(classModeSignalName, "class-%s-mode", newClass->name);
    newClass->classMode = registerSignal(classModeSignalName);
    char classModeStatisticName[50];
    sprintf(classModeStatisticName, "class-%s-mode", newClass->name);
    cProperty *classModeStatisticTemplate = getProperties()->get("statisticTemplate", "classMode");
    getEnvir()->addResultRecorders(this, newClass->classMode, classModeStatisticName, classModeStatisticTemplate);
    emit(newClass->classMode, newClass->mode);

    return newClass;
}

void HtbScheduler::initialize(int stage)
{
    PacketSchedulerBase::initialize(stage); // Initialize the packet scheduler module
    if (stage == INITSTAGE_LOCAL) {
        mtu = par("mtu");
        phyHeaderSize = par("phyLayerHeaderLength");
        valueCorectnessCheck = par("checkHTBTreeValuesForCorectness");
        valueCorectnessAdj = par("adjustHTBTreeValuesForCorectness");
        getParentModule()->subscribe(packetPushEndedSignal, this);
        EV_INFO << "HtbScheduler: parent = " << getParentModule()->getFullPath() << endl;
        // Get the datarate of the link connected to interface
        //register signal for dequeue index
        dequeueIndexSignal = registerSignal("dequeueIndex");
        // Get all leaf queues. IMPORTANT: Leaf queue id MUST correspond to leaf class id!!!!!
        for (auto provider : providers) {
            collections.push_back(dynamic_cast<IPacketCollection *>(provider)); // Get pointers to queues
        }

        // Load configs
        htbConfig = par("htbTreeConfig");
        htb_hysteresis = par("htbHysterisis");

        // Create all classes and build the tree structure
        cXMLElementList classes = htbConfig->getChildren();
        for (auto & oneClass : classes) {
            htbClass* newClass = createAndAddNewClass(oneClass, 0);
            printClass(newClass);
        }
        printClass(rootClass);

        // Create all levels
        for (int i = 0; i < maxHtbDepth; i++) {
            levels[i] = new htbLevel();
            levels[i]->levelId = i;
        }

        classModeChangeEvent = new cMessage("probablyClassNotRedEvent"); // Omnet++ event to take action when new elements to dequeue are available
    }
    else if (stage == INITSTAGE_NETWORK_INTERFACE_CONFIGURATION) {
        EV_INFO << "Get link datarate" << endl;
        auto iface = getContainingNicModule(this);
        linkDatarate = iface->getTxTransmissionChannel()->getNominalDatarate();
        // linkDatarate = -1;
        EV_INFO << "SchedInit: Link datarate = " << linkDatarate << endl;
    }
}

/*
 * METHODS FOR OMNET++ EVENTS - BEGIN
 */

// Wrapper for scheduleAt method
void HtbScheduler::scheduleAt(simtime_t t, cMessage *msg)
{
    Enter_Method("scheduleAt");
    cSimpleModule::scheduleAt(t, msg);
}

// Wrapper for cancelEvent method
cMessage *HtbScheduler::cancelEvent(cMessage *msg)
{
    Enter_Method("cancelEvent");
    return cSimpleModule::cancelEvent(msg);
}

// Handle internal events
void HtbScheduler::handleMessage(cMessage *message)
{
    Enter_Method("handleMessage");
    if (message == classModeChangeEvent) {
        CHK(collector)->handleCanPullPacketChanged(CHK(outputGate)->getPathEndGate());
    }
    else
        throw cRuntimeError("Unknown self message");
}

/*
 * METHODS FOR OMNET++ EVENTS - END
 */

// Refreshes information on class mode when a change in the mode was expected.
// Not a real omnet event. Called only on dequeue when the next queue to pop is determined.
// Does all "events" on a level
simtime_t HtbScheduler::doEvents(int level)
{
    // No empty events = nothing to do
    if (levels[level]->waitingClasses.empty()) {
        EV_INFO << "doEvents: There were no events for level " << level << endl;
        return 0;
    }

    for (auto it = levels[level]->waitingClasses.cbegin(); it != levels[level]->waitingClasses.cend(); ) {
        htbClass *cl = *it; // Class to update
        it++;

        // If the event for a class is still in the future then return the event time
        if (cl->nextEventTime > simTime()) {
            EV_INFO << "doEvents: Considering class " << cl->name << " event lies in the future..." << endl;
            EV_INFO << "doEvents: Next event scheduled at " << cl->nextEventTime << " but current time is " << simTime() << endl;
            // printSet = "";
            for (auto & elem : levels[cl->level]->waitingClasses){
                if (elem->nextEventTime <= simTime()) {
                    std::string printSet = "";
                    for (auto & elem : levels[level]->waitingClasses){
                        printSet.append(elem->name);
                        printSet.append("(");
                        printSet.append(elem->nextEventTime.str());
                        printSet.append("); ");
                    }
                    EV_INFO << "doEvents: Wait queue for level " << level << " after updates: " << printSet << endl;

                    throw cRuntimeError("Class %s has an event that's not in the future but was not upadted! Wait queue for level %d after updates: %s", elem->name, level, printSet.c_str());
                }
            }
            return cl->nextEventTime;
        }

        // Take the class out of waiting queue and update its mode
        htbRemoveFromWaitTree(cl);
        EV_INFO << "doEvents: Considering class " << cl->name << " in old mode " << std::to_string(cl->mode) << endl;
        long long diff = (long long) std::min((simTime() - cl->checkpointTime).inUnit(SIMTIME_NS), (int64_t)cl->mbuffer);
        updateClassMode(cl, &diff);
        EV_INFO << "doEvents: Considering class " << cl->name << " in new mode " << std::to_string(cl->mode) << endl;
        // If still not green (cen_send) readd to wait queue with new next event time.
        if (cl->mode != can_send) {
            htbAddToWaitTree(cl, diff);
        }
    }

    return simTime(); // If we managed all events, then just return current time.
}

// Returns the number of packets available to dequeue. If there are packets in the queue,
// but none are available to dequeue will schedule an event to update the interface and
// "force it" to dequeue again.
int HtbScheduler::getNumPackets() const
{
    const_cast<HtbScheduler *>(this)->cancelEvent(classModeChangeEvent);
    int fullSize = 0;  // Number of all packets in the queue
    int dequeueSize = 0; // Number of packets available for dequeueing
    int leafId = 0; // Id of the leaf/queue
    simtime_t changeTime = simTime() + SimTime(100000, SIMTIME_NS); // Time at which we expect the next change of class mode
    for (auto collection : collections) { // Iterate over all leaf queues
        int collectionNumPackets = collection->getNumPackets();
        if (leafClasses.at(leafId) == nullptr) { // Leafs need to exist. If they don't, we've done sth. wrong :)
            throw cRuntimeError("There is no leaf at index %i!", leafId);
        }
        long long diff = (long long) std::min((simTime() - leafClasses.at(leafId)->checkpointTime).inUnit(SIMTIME_NS), (int64_t)leafClasses.at(leafId)->mbuffer);
        int currClassMode = const_cast<HtbScheduler *>(this)->classMode(leafClasses.at(leafId), &diff);
        if (currClassMode != cant_send) { // The case when there are packets available at leaf for dequeuing
            bool parentOk = false;
            htbClass *parent = leafClasses.at(leafId)->parent;
            if (currClassMode != can_send) { //&& leafClasses.at(leafId)->nextEventTime <= simTime()
                while (parent != NULL) { //&& parent->nextEventTime <= simTime()
                    long long diff2 = (long long) std::min((simTime() - parent->checkpointTime).inUnit(SIMTIME_NS), (int64_t)parent->mbuffer);
                    int parentMode = const_cast<HtbScheduler *>(this)->classMode(parent, &diff2);
                    if (parentMode == can_send && parent->nextEventTime <= simTime()) {
                        parentOk = true;
                        break;
                    }
                    else if (parentMode == cant_send) {
                        break;
                    }
                    else if (parentMode == may_borrow && (parentMode == parent->mode || parent->nextEventTime <= simTime())) {
                        parent = parent->parent;
                    }
                    else {
                        break;
                    }
                }
            }
            else {
                parentOk = true;
            }
            if (parentOk == true) {
                if (currClassMode == can_send && leafClasses.at(leafId)->nextEventTime <= simTime()) {
                    dequeueSize += collectionNumPackets;
                }
                else if (currClassMode == may_borrow && (currClassMode == leafClasses.at(leafId)->mode || leafClasses.at(leafId)->nextEventTime <= simTime())) {
                    dequeueSize += collectionNumPackets;
                }
            }
        }
        fullSize += collectionNumPackets; // Keep track of all packets in leaf queues
        EV_INFO << "getNumPackets: Queue " << leafId << " -> Num packets: " << collectionNumPackets << endl;
        leafId++;
    }

    EV_INFO << "Curr num packets to dequeue: " << dequeueSize << "; All packets: " << fullSize << endl;
    if (dequeueSize == 0 && fullSize > 0 && changeTime > simTime()) { // We have packets in queue, just none are available for dequeue!
        EV_INFO << "Next Event will be scheduled at " << changeTime.str() << endl;
        const_cast<HtbScheduler *>(this)->scheduleAt(changeTime, classModeChangeEvent);  // schedule an Omnet event on when we expect things to change.
    }
    return dequeueSize;
}

bool HtbScheduler::canPullSomePacket(cGate *gate) const
{
    // TODO write a more optimal code
    return getNumPackets() > 0;
}

b HtbScheduler::getTotalLength() const
{
    b totalLength(0);
    for (auto collection : collections)
        totalLength += collection->getTotalLength();
    return totalLength;
}

Packet *HtbScheduler::getPacket(int index) const
{
    int origIndex = index;
    for (auto collection : collections) {
        auto numPackets = collection->getNumPackets();
        if (index < numPackets)
            return collection->getPacket(index);
        else
            index -= numPackets;
    }
    throw cRuntimeError("Index %i out of range", origIndex);
}

void HtbScheduler::removePacket(Packet *packet)
{
    Enter_Method("removePacket");
    for (auto collection : collections) {
        int numPackets = collection->getNumPackets();
        for (int j = 0; j < numPackets; j++) {
            if (collection->getPacket(j) == packet) {
                collection->removePacket(packet);
                return;
            }
        }
    }
    throw cRuntimeError("Cannot find packet");
}

void HtbScheduler::removeAllPackets()
{
    Enter_Method("removeAllPacket");
    for (auto collection : collections)
        collection->removeAllPackets();
}

// This is where the HTB scheduling comes into play
int HtbScheduler::schedulePacket() {
    int level;
    simtime_t nextEvent;
    int dequeueIndex = -1;

    nextEvent = simTime();

    // Go over all levels until we find something to dequeue
    for (level = 0; level < maxHtbDepth; level++) {
        nextEvent = doEvents(level); // Do all events for level
        // Go through all priorities on level until we find something to dequeue
        EV << "schedulePacket: Level " << level << endl;
        printLevel(levels[level], level);
        for (int prio = 0; prio < maxHtbNumPrio; prio++) {
            EV_INFO << "Dequeue - testing: level = " << level << "; priority = " << prio << endl;
            // if (levels[level]->nextToDequeue[prio] != NULL) { // Next to dequeue is always right. If it's not null, then we can dequeue something there. If it's null, we know there is nothing to dequeue.
            if (levels[level]->selfFeeds[prio].size() > 0) { // Next to dequeue is always right. If it's not null, then we can dequeue something there. If it's null, we know there is nothing to dequeue.
                EV_INFO << "Dequeue - Found class to dequeue on level " << level << " and priority " << prio << endl;
                dequeueIndex = htbDequeue(prio, level); // Do the dequeue in the HTB tree. Actual dequeue is done by the interface (I think)
            }
            if (dequeueIndex != -1) { // We found the first valid thing, just break!
                break;
            }
        }
        if (dequeueIndex != -1) { // We found the first valid thing, just break!
            break;
        }
    }
    emit(dequeueIndexSignal, dequeueIndex);
    return dequeueIndex; //index returned to the interface
}

// Activates a class for a priority. Only really called for leafs
void HtbScheduler::activateClass(htbClass *cl, int priority)
{
    EV_INFO << "activateClass called for class " << cl->name << " on priority " << priority << endl;
    if (!cl->activePriority[priority]) {
        cl->activePriority[priority] = true; // Class is now active for priority
        activateClassPrios(cl); // Take care of all other things associated with the active class (like putting it into appropriate feeds)
        if (cl->mode != can_send) {
            htbAddToWaitTree(cl, (long long) 0);
        }
    }
}

// Deactivates a class for a priority. Only really called for leafs. Leafs may only be active for one priority.
void HtbScheduler::deactivateClass(htbClass *cl, int priority)
{
    EV_INFO << "deactivateClass called for class " << cl->name << " on priority " << priority << endl;
    if (cl->activePriority[priority]) {
        deactivateClassPrios(cl); // Take care of all other things associated with deactivating a class
        levels[cl->level]->selfFeeds[priority].erase(cl);
        if (cl->parent != NULL) {
            cl->parent->inner.innerFeeds[priority].erase(cl);
        }
        htbRemoveFromWaitTree(cl);
        cl->activePriority[priority] = false; // Class is now inactive for priority
    }
}

// Inform the htb about a newly enqueued packet. Enqueueing is actually done in the classifier.
void HtbScheduler::htbEnqueue(int index)
{
    htbClass *currLeaf = leafClasses.at(index);
    activateClass(currLeaf, currLeaf->leaf.priority);
    return;
}

// Gets a leaf that can be dequeued from a priority and level. Level does not have to be 0 here
HtbScheduler::htbClass* HtbScheduler::getLeaf(int priority, int level)
{
    htbClass *cl = levels[level]->nextToDequeue[priority];
    printClass(cl);
    if (levels[level]->selfFeeds[priority].find(cl) == levels[level]->selfFeeds[priority].cend()) {
        levels[cl->level]->selfFeeds[priority].insert(cl);
        levels[cl->level]->nextToDequeue[priority] = *std::next(levels[cl->level]->selfFeeds[priority].find(cl));
        if (levels[cl->level]->nextToDequeue[priority] == *levels[cl->level]->selfFeeds[priority].cend()) {
            levels[cl->level]->nextToDequeue[priority] = *levels[cl->level]->selfFeeds[priority].cbegin();
        }
        levels[cl->level]->selfFeeds[priority].erase(cl);
        cl = levels[cl->level]->nextToDequeue[priority];
        printClass(cl);
    }

    // If there is nothing to dequeue just return NULL (actually)
    if (cl == NULL) {
        return cl;
    }

    // Otherwise, follow the tree to the leaf
    while (cl->level > 0) {
        htbClass *parent = cl;
        cl = parent->inner.nextToDequeue[priority];
        printClass(cl);
        if (parent->inner.innerFeeds[priority].find(cl) == parent->inner.innerFeeds[priority].cend()) {
            parent->inner.innerFeeds[priority].insert(cl);
            parent->inner.nextToDequeue[priority] = *std::next(parent->inner.innerFeeds[priority].find(cl));
            if (parent->inner.nextToDequeue[priority] == *parent->inner.innerFeeds[priority].cend()) {
                parent->inner.nextToDequeue[priority] = *parent->inner.innerFeeds[priority].cbegin();
            }
            parent->inner.innerFeeds[priority].erase(cl);
            cl = parent->inner.nextToDequeue[priority];
            printClass(cl);
        }
    }
    printClass(cl);
    // And return the found leaf. Thanks to the rest of code, guaranteed to return leaf or NULL.
    return cl;
}

// Determine which leaf queue to dequeue based on the htb tree. Based heavily on Linux HTB source code
int HtbScheduler::htbDequeue(int priority, int level)
{
    int retIndex = -1;
    htbClass *cl = getLeaf(priority, level); // Get first leaf we could find
    htbClass *start = cl;
    Packet *thePacketToPop = nullptr;
    htbClass *delClass = nullptr;

    do {
        if (delClass == start) {   // Fix start if we just deleted it
            start = cl;
            delClass = nullptr;
        }
        if (!cl) {
            return -1;
        }
        // Take care if the queue is empty, but was not deactivated
        if (collections[cl->leaf.queueId]->isEmpty()) {
            htbClass *next;
            deactivateClass(cl, priority); // Also takes care of DRR if we deleted the nextToDequeue. Only needs to be called once, as leafs can only be active for one prio
            delClass = cl;
            next = getLeaf(priority, level); // Get next leaf available

            cl = next;
            continue;
        }

        thePacketToPop = providers[cl->leaf.queueId]->canPullPacket(inputGates[cl->leaf.queueId]);

        if (thePacketToPop != nullptr) {
            retIndex = cl->leaf.queueId;
            break;
        }

        cl = getLeaf(priority, level);

    } while (cl != start);
    EV_INFO << "htbDequeue: Queue of leaf " << cl->name << " will be dequeued in mode " << cl->mode << endl;
    if (thePacketToPop != nullptr) {
        // Take care of the deficit round robin
        if (cl->leaf.deficit[level] < 0) {
            throw cRuntimeError("The class %s deficit on level %d is negative!", cl->name, level);
        }
        cl->leaf.deficit[level] -= (thePacketToPop->getByteLength() + phyHeaderSize);
        emit(cl->leaf.deficitSig[level], cl->leaf.deficit[level]);
        if (cl->leaf.deficit[level] < 0) {
            cl->leaf.deficit[level] += cl->quantum;
            emit(cl->leaf.deficitSig[level], cl->leaf.deficit[level]);
            htbClass *tempNode = cl;
            while (tempNode != rootClass) {
                if (tempNode->parent->inner.innerFeeds[priority].size() > 1 && tempNode->mode == may_borrow) {
                    if (tempNode->parent->inner.nextToDequeue[priority] == tempNode) {
                        tempNode->parent->inner.nextToDequeue[priority] = *std::next(tempNode->parent->inner.innerFeeds[priority].find(tempNode));
                        if (tempNode->parent->inner.nextToDequeue[priority] == *tempNode->parent->inner.innerFeeds[priority].cend()) {
                            tempNode->parent->inner.nextToDequeue[priority] = *tempNode->parent->inner.innerFeeds[priority].cbegin();
                        }
                    }
                }
                else if (levels[tempNode->level]->selfFeeds[priority].size() > 1 && tempNode->mode == can_send) {
                    if (levels[tempNode->level]->nextToDequeue[priority] == tempNode) {
                        levels[tempNode->level]->nextToDequeue[priority] = *std::next(levels[tempNode->level]->selfFeeds[priority].find(tempNode));
                        if (levels[tempNode->level]->nextToDequeue[priority] == *levels[tempNode->level]->selfFeeds[priority].cend()) {
                            levels[tempNode->level]->nextToDequeue[priority] = *levels[tempNode->level]->selfFeeds[priority].cbegin();
                        }
                        if (levels[tempNode->level]->nextToDequeue[priority] == nullptr) {
                            throw cRuntimeError("Next to dequeue would be null even though it shouldn't be!");
                        }
                    }
                    break;
                }
                else if (levels[tempNode->level]->selfFeeds[priority].size() == 1 && tempNode->mode == can_send) {
                    if (levels[tempNode->level]->nextToDequeue[priority] != tempNode) {
                        throw cRuntimeError("We were green and got selected but were not next to dequeue!");
                    }
                    break;
                }
                tempNode = tempNode->parent;
            }
        }
        chargeClass(cl, level, thePacketToPop);
        if (collections[cl->leaf.queueId]->getNumPackets() <= 1) {
            deactivateClass(cl, priority); // Called for leaf. Leaf can be active for only one prio
        }
    }
    return retIndex;
}

void HtbScheduler::printLevel(htbLevel *level, int index)
{
    EV_INFO << "Self feeds for Level:  " << level->levelId << endl;
    for (int i = 0; i < maxHtbNumPrio; i++) {
        std::string printSet = "";
        for (auto & elem : level->selfFeeds[i]){
            printSet.append(elem->name);
            printSet.append(" ;");
        }
        printSet.append(" Next to dequeue: ");
        if (level->nextToDequeue[i] != NULL) {
            printSet.append(level->nextToDequeue[i]->name);
        }
        else {
            printSet.append("None");
        }
        EV_INFO << "   - Self feed for priority " << i  << ": " << printSet << endl;
    }
    EV_INFO << "Wait queue for level " << level->levelId << " contains " << level->waitingClasses.size() << " elements" << endl;
}

void HtbScheduler::printInner(htbClass *cl)
{
    EV_INFO << "Inner feeds for Inner Class:  " << cl->name << endl;
    for (int i = 0; i < maxHtbNumPrio; i++) {
        std::string printSet = "";
        for (auto & elem : cl->inner.innerFeeds[i]){
            printSet.append(elem->name);
            printSet.append(" ;");
        }
        printSet.append(" Next to dequeue: ");
        if (cl->inner.nextToDequeue[i] != NULL) {
            printSet.append(cl->inner.nextToDequeue[i]->name);
        }
        else {
            printSet.append("None");
        }
        EV_INFO << "   - Inner feed for priority " << i  << ": " << printSet << endl;
    }
}

inline long HtbScheduler::htb_lowater(htbClass *cl)
{
    if (htb_hysteresis)
        return cl->mode != cant_send ? -cl->cburstSize : 0;
    else
        return 0;
}

inline long HtbScheduler::htb_hiwater(htbClass *cl)
{
    if (htb_hysteresis)
        return cl->mode == can_send ? -cl->burstSize : 0;
    else
        return 0;
}

int HtbScheduler::classMode(htbClass *cl, long long *diff)
{
    long long toks;
    if ((toks = (cl->ctokens + *diff)) < htb_lowater(cl)) {
        *diff = -toks;
        return cant_send;
    }
    if ((toks = (cl->tokens + *diff)) >= htb_hiwater(cl)) {
        return can_send;
    }
    *diff = -toks;
    return may_borrow;
}

void HtbScheduler::activateClassPrios(htbClass *cl)
{
    EV_INFO << "activateClassPrios called for class " << cl->name << endl;
    htbClass *parent = cl->parent;

    bool newActivity[8];
    bool tempActivity[8];
    std::copy(std::begin(cl->activePriority), std::end(cl->activePriority), std::begin(newActivity));

    while (cl->mode == may_borrow && parent != NULL && std::any_of(std::begin(newActivity), std::end(newActivity), [](bool b) {return b;})) { // Iterate over as long as the class is in may_borrow, the class is not root and There is an activity the class should be activated on
        std::copy(std::begin(newActivity), std::end(newActivity), std::begin(tempActivity));
        for (int i = 0; i < maxHtbNumPrio; i++) { // i = priority level
            if (tempActivity[i]) { // Check if parent shoudl be actie for priority
                parent->activePriority[i] = true; // Set parent to active on priority
                if (parent->inner.innerFeeds[i].find(cl) == parent->inner.innerFeeds[i].cend()) { // Check if we are in parent's inner feed
                    if (parent->inner.nextToDequeue[i] == NULL) {
                        parent->inner.nextToDequeue[i] = cl;
                    }
                    parent->inner.innerFeeds[i].insert(cl); // Finally, insert us to the inner feed
                }
            }
        }
        cl = parent;
        parent = cl->parent;
    }

    printClass(cl);

    if (cl->mode == can_send && std::any_of(std::begin(newActivity), std::end(newActivity), [](bool b) {return b;})){
        for (int i = 0; i < maxHtbNumPrio; i++) { // i = priority level
            if (newActivity[i]) {
                if (levels[cl->level]->selfFeeds[i].find(cl) == levels[cl->level]->selfFeeds[i].cend()) {
                    if (levels[cl->level]->nextToDequeue[i] == NULL) {
                        levels[cl->level]->nextToDequeue[i] = cl;
                    }
                    levels[cl->level]->selfFeeds[i].insert(cl);
                }
            }
        }
    }
}

void HtbScheduler::deactivateClassPrios(htbClass *cl)
{
    EV_INFO << "deactivateClassPrios called for class " << cl->name << endl;
    htbClass *parent = cl->parent;

    bool newActivity[8];
    bool tempActivity[8];
    std::copy(std::begin(cl->activePriority), std::end(cl->activePriority), std::begin(newActivity));

    while (cl->mode == may_borrow && parent != NULL && std::any_of(std::begin(newActivity), std::end(newActivity), [](bool b) {return b;})) {
        std::copy(std::begin(newActivity), std::end(newActivity), std::begin(tempActivity));
        for (int i = 0; i < maxHtbNumPrio; i++) {
            newActivity[i] = false;
        }

        for (int i = 0; i < maxHtbNumPrio; i++) { // i = priority level
            if (tempActivity[i]) {
                if (parent->inner.innerFeeds[i].find(cl) != parent->inner.innerFeeds[i].cend()) {
                    parent->inner.innerFeeds[i].erase(cl);
                }
                if (parent->inner.innerFeeds[i].empty()) {
                    parent->activePriority[i] = false;
                    newActivity[i] = true;
                }
            }
        }
        cl = parent;
        parent = cl->parent;
    }

    if (cl->mode == can_send && std::any_of(std::begin(newActivity), std::end(newActivity), [](bool b) {return b;})){
        EV_INFO << "Class " << cl->name << " is with with mode " << cl->mode << " and is active on some priority!" << endl;
        for (int i = 0; i < maxHtbNumPrio; i++) { // i = priority level
            EV_INFO << "Testing priority " << i << endl;
            if (newActivity[i]) {
                EV_INFO << "Class is active on priority " << i << endl;
                EV_INFO << "Erase class " << cl->name << " with with mode " << cl->mode << " from self feed on level " << cl->level << " and priority " << i << endl;
                levels[cl->level]->selfFeeds[i].erase(cl);
            }
        }
    }
}

void HtbScheduler::updateClassMode(htbClass *cl, long long *diff)
{
    int newMode = classMode(cl, diff);
    EV_INFO << "updateClassMode - oldMode = " << cl->mode << "; newMode = " << newMode << endl;
    if (newMode == cl->mode)
            return;
    if (std::any_of(std::begin(cl->activePriority), std::end(cl->activePriority), [](bool b) {return b;})) {
       EV_INFO << "Deactivate then activate!" << endl;
        if (cl->mode != cant_send) {
            EV_INFO << "updateClassMode - Deactivate priorities for class: " << cl->name << " in old mode " << cl->mode << endl;
            deactivateClassPrios(cl);
        }
        cl->mode = newMode;
        emit(cl->classMode, cl->mode);
        if (newMode != cant_send) {
           EV_INFO << "updateClassMode - Activate priorities for class: " << cl->name << " in new mode " << cl->mode << endl;
            activateClassPrios(cl);
        }
    }
    else {
        cl->mode = newMode;
        emit(cl->classMode, cl->mode);
    }
}

void HtbScheduler::accountTokens(htbClass *cl, long long bytes, long long diff) {

    long long toks = diff + cl->tokens;
    if (toks > cl->burstSize) {
        toks = cl->burstSize;
    }
    toks -= bytes * 8 * 1e9 / cl->assignedRate;
    if (toks <= -cl->mbuffer)
        toks = 1 - cl->mbuffer;
    cl->tokens = toks;
    emit(cl->tokenBucket, (long) cl->tokens);
    return;
}

void HtbScheduler::accountCTokens(htbClass *cl, long long bytes, long long diff)
{
    long long toks = diff + cl->ctokens;
    if (toks > cl->cburstSize) {
        toks = cl->cburstSize;
    }
    toks -= bytes * 8 * 1e9 / cl->ceilingRate;
    if (toks <= -cl->mbuffer)
        toks = 1 - cl->mbuffer;
    cl->ctokens = toks;
    emit(cl->ctokenBucket, (long) cl->ctokens);
    return;
}

void HtbScheduler::htbAddToWaitTree(htbClass *cl, long long delay)
{
    cl->nextEventTime = simTime() + SimTime(delay, SIMTIME_NS);
    if (levels[cl->level]->waitingClasses.find(cl) != levels[cl->level]->waitingClasses.cend()) {
        throw cRuntimeError("Class added to waiting classes even though it was already there!");
    }
    levels[cl->level]->waitingClasses.insert(cl);
    printLevel(levels[cl->level], cl->level);
}

void HtbScheduler::htbRemoveFromWaitTree(htbClass *cl)
{
    std::multiset<htbClass*, waitComp>::iterator hit(levels[cl->level]->waitingClasses.find(cl));
    while (hit != levels[cl->level]->waitingClasses.cend()) {
        if (*hit == cl) {
            levels[cl->level]->waitingClasses.erase(hit);
            break;
        }
        else {
            hit++;
        }
    }
}

void HtbScheduler::chargeClass(htbClass *leafCl, int borrowLevel, Packet *packetToDequeue)
{
    long long bytes = (long long) packetToDequeue->getByteLength() + phyHeaderSize;
    int old_mode;
    long long diff;

    htbClass *cl;
    cl = leafCl;

    while (cl) {
        if (cl->checkpointTime == simTime()) {
            throw cRuntimeError("Updating a class that was already updated exactly now!");
        }
        diff = (long long) std::min((simTime() - cl->checkpointTime).inUnit(SIMTIME_NS), (int64_t)cl->mbuffer);
        EV_INFO << "chargeClass: Diff1 is: " << diff << "; Packet Bytes: " << bytes << "Used toks:" << bytes * 8 * 1e9 / cl->assignedRate << "Used ctoks:" << bytes * 8 * 1e9 / cl->ceilingRate << endl;
        if (cl->level >= borrowLevel) {
            accountTokens(cl, bytes, diff);
        }
        else {
            cl->tokens += diff; /* we moved t_c; update tokens */
            emit(cl->tokenBucket, (long) cl->tokens);
        }
        EV_INFO << "chargeClass: Diff2 is: " << diff << "; Packet Bytes: " << bytes << "Used toks:" << bytes * 8 * 1e9 / cl->assignedRate << "Used ctoks:" << bytes * 8 * 1e9 / cl->ceilingRate << endl;
        accountCTokens(cl, bytes, diff);
        cl->checkpointTime = simTime();

        old_mode = cl->mode;
        diff = 0;
        updateClassMode(cl, &diff);

        if (old_mode != cl->mode) {
            if (old_mode != can_send) {
                htbRemoveFromWaitTree(cl);
            }
            if (cl->mode != can_send) {
                htbAddToWaitTree(cl, diff);
            }
        }
        cl = cl->parent;
    }

    return;
}

void HtbScheduler::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));
    if (signal == packetPushEndedSignal) {
        if (std::string(source->getClassName()).find("inet::queueing::PacketQueue") != std::string::npos) { // Might need adjustment so that we can use compound packet queues as queues
            int index = static_cast<cModule*>(source)->getIndex();
            EV_INFO << "HtbScheduler::receiveSignal: PacketQueue " << index << " emitted a packetPushed signal! Call htbEnqueue for leaf " << index << endl;
            htbEnqueue(index);
        }
    }
    else
        throw cRuntimeError("Unknown signal");
}

} // namespace queueing
} // namespace inet

