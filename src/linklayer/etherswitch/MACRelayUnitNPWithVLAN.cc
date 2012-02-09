/*
 * Copyright (C) 2011 Kyeong Soo (Joseph) Kim
 * Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "MACRelayUnitNPWithVLAN.h"
#include "EtherFrame_m.h"
#include "Ethernet.h"
#include "MACAddress.h"
#include "VLANTagger.h"


#define MAX_LINE 100


Define_Module( MACRelayUnitNPWithVLAN);

/* unused for now
 static std::ostream& operator<< (std::ostream& os, cMessage *msg)
 {
 os << "(" << msg->getClassName() << ")" << msg->getFullName();
 return os;
 }
 */

/**
 * Function reads from a file stream pointed to by 'fp' and stores characters
 * until the '\n' or EOF character is found, the resultant string is returned.
 * Note that neither '\n' nor EOF character is stored to the resultant string,
 * also note that if on a line containing useful data that EOF occurs, then
 * that line will not be read in, hence must terminate file with unused line.
 */
static char *fgetline(FILE *fp)
{
    // alloc buffer and read a line
    char *line = new char[MAX_LINE];
    if (fgets(line, MAX_LINE, fp) == NULL)
        return NULL;

    // chop CR/LF
    line[MAX_LINE - 1] = '\0';
    int len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
        line[--len] = '\0';

    return line;
}

//MACRelayUnitNPWithVLAN::MACRelayUnitNPWithVLAN()
//{
//    endProcEvents = NULL;
//    numCPUs = 0;
//}
//
//MACRelayUnitNPWithVLAN::~MACRelayUnitNPWithVLAN()
//{
//    for (int i=0; i<numCPUs; i++)
//    {
//        cMessage *endProcEvent = endProcEvents[i];
//        EthernetIIFrameWithVLAN *etherFrame = (EthernetIIFrameWithVLAN *)endProcEvent->getContextPointer();
//        if (etherFrame)
//        {
//            endProcEvent->setContextPointer(NULL);
//            delete etherFrame;
//        }
//        cancelAndDelete(endProcEvent);
//    }
//    delete [] endProcEvents;
//}

void MACRelayUnitNPWithVLAN::initialize(int stage)
{
    if (stage == 0)
    {
        MACRelayUnitNP::initialize();
    }
    else if (stage == 1) // to make sure the following is executed after the normal initialization of other modules (esp. VLANTagger)
    {
        int gateSize = this->gateSize("lowerLayerIn");
        for (int i = 0; i < gateSize; i++)
        {
            // access to the tagger module to initialize a VLAN registration table
            cModule *taggerModule = getParentModule()->getSubmodule("tagger", i);
            VLANTagger *tagger = check_and_cast<VLANTagger *>(taggerModule);

            std::vector<VID> vids = tagger->getVidSet();;
//            if (tagger->isTagged() == false)
//            {
//                vids.push_back(tagger->getPvid());
//            }
//            else
//            {
//                vids = tagger->getVidSet();
//            }
            for (unsigned int j = 0; j < vids.size(); j++)
            {
                PortMap portMap;
                PortStatus *portStatus = new PortStatus();
                portStatus->portno = i;
                portStatus->registration = Fixed;
                portStatus->tagged = tagger->isTagged();

                VLANRegistrationTable::iterator iter = vlanTable.find(vids[j]);
                if (iter == vlanTable.end())
                {
                    // add new entry
                    EV << "Adding entry to VLAN registration table: " << vids[j] << " --> port" << i << endl;
                    portMap.push_back(portStatus);
                    vlanTable[vids[j]] = portMap;
                }
                else
                {
                    // update existing entry in VLAN registration table
                    EV << "Updating entry in VLAN registration table: " << vids[j] << " --> port" << i << endl;
                    PortMap& entry = iter->second;
                    entry.push_back(portStatus);
                }
            }
        }
    }
}

void MACRelayUnitNPWithVLAN::handleAndDispatchFrame(EtherFrame *frame, int inputport)
{
    // make sure that the received frame has a VLAN tag
    EthernetIIFrameWithVLAN *vlanFrame = check_and_cast<EthernetIIFrameWithVLAN *>(frame);

    // update the VLAN address table (i.e., the 'Filtering Database' in the IEEE 802.1Q standard)
    updateVLANTableWithAddress(vlanFrame->getSrc(), vlanFrame->getVid(), inputport); // now with VID as well

    // handle broadcast frames first
    if (vlanFrame->getDest().isBroadcast())
    {
        EV << "Broadcasting broadcast frame " << vlanFrame << endl;
        broadcastFrame(vlanFrame, inputport);
        return;
    }

    // Finds output port of destination address and sends to output port
    // if not found then broadcasts to all other ports instead
    int outputport = getPortForVLANAddress(vlanFrame->getDest(), vlanFrame->getVid()); // now with VID as well
    if (inputport == outputport)
    {
        EV << "Output port is same as input port, " << vlanFrame->getFullName()
                << " dest " << vlanFrame->getDest() << ", discarding frame\n";
        delete vlanFrame;
        return;
    }
    if (outputport >= 0)
    {
        EV << "Sending frame " << vlanFrame << " with dest address " << vlanFrame->getDest() << " to port "
           << outputport << endl;
        send(vlanFrame, "lowerLayerOut", outputport);
    }
    else
    {
        EV << "Dest address " << vlanFrame->getDest() << " unknown, broadcasting frame " << vlanFrame << endl;
        broadcastFrame(vlanFrame, inputport);
    }
}

void MACRelayUnitNPWithVLAN::broadcastFrame(EtherFrame *frame, int inputport)
{
    // make sure that the received frame has a VLAN tag
    EthernetIIFrameWithVLAN *vlanFrame = check_and_cast<EthernetIIFrameWithVLAN *>(frame);

    VLANRegistrationTable::iterator iter = vlanTable.find(vlanFrame->getVid());

    if (iter == vlanTable.end())
    {
        EV << getFullPath() << ": The VID of the received frame:" << vlanFrame->getVid() << endl;
        EV << "The contents of the vlanTable:" << endl;
        for (VLANRegistrationTable::iterator vid_it = vlanTable.begin(); vid_it != vlanTable.end(); vid_it++)
        {
            EV << "VID: " << vid_it->first << endl;
            for (unsigned int i = 0; i < vid_it->second.size(); i++)
            {
                EV << "- Port " << i << ": " << vid_it->second[i]->registration << ", " << (vid_it->second[i]->tagged ? "tagged" : "untagged") << endl;
            }
        }
        error("There is an error in VLAN configuration.");
    }
    else
    {
        PortMap &portMap = iter->second;
        for (unsigned int i = 0; i < portMap.size(); i++)
        {
            if ((portMap[i]->portno != inputport) && (portMap[i]->registration == Fixed))
            {
                send((EthernetIIFrameWithVLAN*) vlanFrame->dup(), "lowerLayerOut", portMap[i]->portno);
            }
        }
    }
    delete frame;
}

void MACRelayUnitNPWithVLAN::printAddressTable()
{
    VLANAddressTable::iterator iter;
    EV << "Address Table (" << addresstable.size() << " entries):\n";
    for (iter = addresstable.begin(); iter != addresstable.end(); ++iter)
    {
        EV << "  " << iter->first << " --> vid" << iter->second.vid << " --> port" << iter->second.portno
           << (iter->second.insertionTime + agingTime <= simTime() ? " (aged)" : "") << endl;
    }
}

void MACRelayUnitNPWithVLAN::removeAgedEntriesFromTable()
{
    for (VLANAddressTable::iterator iter = addresstable.begin(); iter != addresstable.end();)
    {
        VLANAddressTable::iterator cur = iter++; // iter will get invalidated after erase()
        VLANAddressEntry& entry = cur->second;
        if (entry.insertionTime + agingTime <= simTime())
        {
            EV << "Removing aged entry from Address Table: " << cur->first << " --> vid" << cur->second.vid
               << " --> port" << cur->second.portno << "\n";
            addresstable.erase(cur);
        }
    }
}

void MACRelayUnitNPWithVLAN::removeOldestTableEntry()
{
    VLANAddressTable::iterator oldest = addresstable.end();
    simtime_t oldestInsertTime = simTime() + 1;
    for (VLANAddressTable::iterator iter = addresstable.begin(); iter != addresstable.end(); iter++)
    {
        if (iter->second.insertionTime < oldestInsertTime)
        {
            oldest = iter;
            oldestInsertTime = iter->second.insertionTime;
        }
    }
    if (oldest != addresstable.end())
    {
        EV << "Table full, removing oldest entry: " << oldest->first << " --> port" << oldest->second.portno << "\n";
        addresstable.erase(oldest);
    }
}

void MACRelayUnitNPWithVLAN::updateVLANTableWithAddress(MACAddress& address, VID vid, int portno)
{
    bool updated = false;
    std::pair<VLANAddressTable::iterator, VLANAddressTable::iterator> range = addresstable.equal_range(address);

    if (range.first != range.second)
    {
        for (VLANAddressTable::iterator iter = range.first; iter != range.second; ++iter)
        {
            if ((vid == iter->second.vid) && (portno == iter->second.portno))
            {
                // update the existing entry
                EV << "Updating entry in Address Table: " << address << "--> vid" << vid << " --> port" << portno
                   << "\n";
                VLANAddressEntry& entry = iter->second;
                entry.vid = vid;
                entry.portno = portno;
                entry.insertionTime = simTime();
                updated = true;
            }
        }
    }

    if (updated == false)
    {
        // it's a new entry

        if (addressTableSize != 0 && addresstable.size() == (unsigned int) addressTableSize)
        {
            // observe finite table size
            // lazy removal of aged entries: only if table gets full (this step is not strictly needed)
            EV << "Making room in Address Table by throwing out aged entries.\n";
            removeAgedEntriesFromTable();

            if (addresstable.size() == (unsigned int) addressTableSize)
                removeOldestTableEntry();
        }

        // add a new entry
        EV << "Adding entry to Address Table: " << address << "--> vid" << vid << " --> port" << portno << "\n";
        VLANAddressEntry entry;
        entry.vid = vid;
        entry.portno = portno;
        entry.insertionTime = simTime();
        addresstable.insert(std::pair<MACAddress, VLANAddressEntry>(address, entry));
    }
}

int MACRelayUnitNPWithVLAN::getPortForVLANAddress(MACAddress& address, VID vid)
{
    VLANAddressTable::iterator iter;
    std::pair<VLANAddressTable::iterator, VLANAddressTable::iterator> range = addresstable.equal_range(address);

    if (range.first != range.second)
    {
        for (iter = range.first; iter != range.second; iter++)
        {
            if (vid == iter->second.vid)
            {
                if (iter->second.insertionTime + agingTime <= simTime())
                {
                    // don't use (and throw out) aged entries
                    EV << "Ignoring and deleting aged entry: " << iter->first
                            << " --> vid" << iter->second.vid << " --> port" << iter->second.portno << endl;
                    addresstable.erase(iter);
                    break;
                }
                else
                {
                    return iter->second.portno;
                }
            }
        }
    }

    // failed to get a port
    return -1;
}

int MACRelayUnitNPWithVLAN::getVIDForMACAddress(MACAddress address)
{
    VLANAddressTable::iterator iter = addresstable.find(address);

    // TODO: Extend to the case of multiple VIDs for a given MAC address

    if (iter != addresstable.end())
    {
    	return iter->second.vid;
    }

    // failed to get a vid
    return -1;
}

void MACRelayUnitNPWithVLAN::readAddressTable(const char* fileName)
{
    FILE *fp = fopen(fileName, "r");
    if (fp == NULL)
        error("cannot open address table file `%s'", fileName);

    //  Syntax of the file goes as:
    //  MAC Address (hex), VID (decimal), Portno
    //  ffffffff    1   1
    //  ffffeed1    1   2
    //  aabcdeff    2   3
    //
    //  etc...
    //
    //  Each iteration of the loop reads in an entire line i.e. up to '\n' or EOF characters
    //  and uses strtok to extract tokens from the resulting string
    char *line;
    int lineno = 0;
    while ((line = fgetline(fp)) != NULL)
    {
        lineno++;

        // lines beginning with '#' are treated as comments
        if (line[0] == '#')
            continue;

        // scan in hexadecimal MAC address
        char *hexaddress = strtok(line, " \t");
        // scan in VID
        char *vid = strtok(line, " \t");
        // scan in port number
        char *portno = strtok(NULL, " \t");

        // empty line?
        if (!hexaddress)
            continue;

        // broken line?
        if (!vid || !portno)
            error("line %d invalid in address table file `%s'", lineno, fileName);

        // Create an entry with VID, portno and insertion time, and insert into table
        VLANAddressEntry entry;
        entry.vid = atoi(vid);
        entry.portno = atoi(portno);
        entry.insertionTime = 0;
        addresstable.insert(std::pair<MACAddress, VLANAddressEntry>(MACAddress(hexaddress), entry));

        // Garbage collection before next iteration
        delete[] line;
    }
    fclose(fp);
}

//void MACRelayUnitNPWithVLAN::handleMessage(cMessage *msg)
//{
//    if (!msg->isSelfMessage())
//    {
//        // Frame received from MAC unit
//        handleIncomingFrame(check_and_cast<EthernetIIFrameWithVLAN *>(msg));
//    }
//    else
//    {
//        // Self message signal used to indicate a frame has finished processing
//        processFrame(msg);
//    }
//}

//void MACRelayUnitNPWithVLAN::handleIncomingFrame(EthernetIIFrameWithVLAN *frame)
//{
//    // If buffer not full, insert payload frame into buffer and process the frame in parallel.
//
//    long length = frame->getByteLength();
//    if (length + bufferUsed < bufferSize)
//    {
//        bufferUsed += length;
//
//        // send PAUSE if above watermark
//        if (pauseUnits>0 && highWatermark>0 && bufferUsed>=highWatermark && simTime()-pauseLastSent>pauseInterval)
//        {
//            // send PAUSE on all ports
//            for (int i=0; i<numPorts; i++)
//                sendPauseFrame(i, pauseUnits);
//            pauseLastSent = simTime();
//        }
//
//        // assign frame to a free CPU (if there is one)
//        int i;
//        for (i=0; i<numCPUs; i++)
//            if (!endProcEvents[i]->isScheduled())
//                break;
//        if (i==numCPUs)
//        {
//            EV << "All CPUs busy, enqueueing incoming frame " << frame << " for later processing\n";
//            queue.insert(frame);
//        }
//        else
//        {
//            EV << "Idle CPU-" << i << " starting processing of incoming frame " << frame << endl;
//            cMessage *msg = endProcEvents[i];
//            ASSERT(msg->getContextPointer()==NULL);
//            msg->setContextPointer(frame);
//            scheduleAt(simTime() + processingTime, msg);
//        }
//    }
//    // Drop the frame and record the number of dropped frames
//    else
//    {
//        EV << "Buffer full, dropping frame " << frame << endl;
//        delete frame;
//        ++numDroppedFrames;
//    }
//
//    // Record statistics of buffer usage levels
//    bufferLevel.record(bufferUsed);
//}

//void MACRelayUnitNPWithVLAN::processFrame(cMessage *msg)
//{
//    int cpu = msg->getKind();
//    EthernetIIFrameWithVLAN *frame = (EthernetIIFrameWithVLAN *) msg->getContextPointer();
//    ASSERT(frame);
//    msg->setContextPointer(NULL);
//    long length = frame->getByteLength();
//    int inputport = frame->getArrivalGate()->getIndex();
//
//    EV << "CPU-" << cpu << " completed processing of frame " << frame << endl;
//
//    handleAndDispatchFrame(frame, inputport);
//    printAddressTable();
//
//    bufferUsed -= length;
//    bufferLevel.record(bufferUsed);
//
//    numProcessedFrames++;
//
//    // Process next frame in queue if they are pending
//    if (!queue.empty())
//    {
//        EthernetIIFrameWithVLAN *newframe = (EthernetIIFrameWithVLAN *) queue.pop();
//        msg->setContextPointer(newframe);
//        EV << "CPU-" << cpu << " starting processing of frame " << newframe << endl;
//        scheduleAt(simTime()+processingTime, msg);
//    }
//    else
//    {
//        EV << "CPU-" << cpu << " idle\n";
//    }
//}

//void MACRelayUnitNPWithVLAN::finish()
//{
//    recordScalar("processed frames", numProcessedFrames);
//    recordScalar("dropped frames", numDroppedFrames);
//}
