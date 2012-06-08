//
// Copyright (C) 2008 Alfonso Ariza
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

#include "InterfaceTableAccess.h"
#include "Ieee80211Etx.h"
#include "Ieee80211Frame_m.h"
#include "Radio80211aControlInfo_m.h"

Define_Module(Ieee80211Etx);

void Ieee80211Etx::initialize(int stage)
{
    if (stage==2)
    {
        etxTimer = new cMessage("etxTimer");
        ettTimer = new cMessage("etxTimer");
        etxInterval = par("ETXInterval");
        ettInterval = par("ETTInterval");
        etxMeasureInterval = par("ETXMeasureInterval");
        etxSize = par("ETXSize");
        ettSize1 = par("ETTSize1");
        ettSize2 = par("ETTSize2");
        maxLive = par("TimeToLive");
        powerWindow = par("powerWindow");
        powerWindowTime = par("powerWindowTime");
        NotificationBoard *nb = NotificationBoardAccess().get();
        nb->subscribe(this, NF_LINK_BREAK);
        nb->subscribe(this, NF_LINK_FULL_PROMISCUOUS);
        IInterfaceTable *inet_ift = InterfaceTableAccess().get();
        for (int i = 0; i < inet_ift->getNumInterfaces(); i++)
        {
            InterfaceEntry * ie = inet_ift->getInterface(i);
            if (ie->getMacAddress()==myAddress)
                ie->setEstimateCostProcess(par("Index").longValue(), this);
        }
        if (etxSize>0 && etxInterval>0)
            scheduleAt(simTime()+par("jitter")+etxInterval, etxTimer);
        if (ettInterval>0 && ettSize1>0 && ettSize2>0)
            scheduleAt(simTime()+par("jitter")+ettInterval, ettTimer);
    }
}


Ieee80211Etx::~Ieee80211Etx()
{
    for (unsigned int i = 0; i<neighbors.size(); i++)
    {
        while (!neighbors[i].empty())
        {
            delete neighbors[i].begin()->second;
            neighbors[i].erase(neighbors[i].begin());
        }
    }
    neighbors.clear();
    cancelAndDelete(etxTimer);
    cancelAndDelete(ettTimer);
}

void Ieee80211Etx::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        // process timers
        EV << "Timer expired: " << msg << "\n";
        handleTimer(msg);
        return;
    }
    if (dynamic_cast<MACETXPacket *>(msg))
    {
        handleEtxMessage(dynamic_cast<MACETXPacket *>(msg));
    }
    else if (dynamic_cast<MACBwPacket *>(msg))
    {
        handleBwMessage(dynamic_cast<MACBwPacket *>(msg));
    }
}

void Ieee80211Etx::handleTimer(cMessage *msg)
{
    if (msg == etxTimer)
    {
        for (unsigned int i = 0; i< neighbors.size(); i++)
        {
            MACETXPacket *pkt = new MACETXPacket();
            pkt->setBitLength(etxSize);
            pkt->setSource(myAddress);
            pkt->setDest(MACAddress::BROADCAST_ADDRESS);
            for (NeighborsMap::iterator it = neighbors[i].begin(); it != neighbors[i].end();)
            {
                if (simTime() - it->second->getTime() > maxLive)
                {
                    NeighborsMap::iterator itAux = it;
                    it++;
                    delete itAux->second;
                    neighbors[i].erase(itAux);
                    continue;
                }
                it++;
            }
            pkt->setNeighborsArraySize(neighbors[i].size());
            pkt->setRecPacketsArraySize(neighbors[i].size());
            int j = 0;
            for (NeighborsMap::iterator it = neighbors[i].begin(); it != neighbors[i].end(); it++)
            {
                pkt->setNeighbors(j, it->second->getAddress());
                pkt->setRecPackets(j, it->second->timeVector.size());
                j++;
            }
            pkt->setKind(i);
            send(pkt, "toMac");
        }
        scheduleAt(simTime()+par("jitter")+etxInterval, etxTimer);
    }
    else if (msg == ettTimer)
    {
        for (unsigned int i = 0; i < neighbors.size(); i++)
        {
            for (NeighborsMap::iterator it = neighbors[i].begin(); it != neighbors[i].end();)
            {
                if (simTime() - it->second->getTime() > maxLive)
                {
                    NeighborsMap::iterator itAux = it;
                    it++;
                    neighbors[i].erase(itAux);
                    continue;
                }
                MACBwPacket *pkt1 = new MACBwPacket();
                MACBwPacket *pkt2 = new MACBwPacket();
                pkt1->setSource(myAddress);
                pkt2->setSource(myAddress);
                pkt1->setBitLength(ettSize1);
                pkt2->setBitLength(ettSize2);
                pkt1->setDest(it->second->getAddress());
                pkt2->setDest(it->second->getAddress());
                it->second->setEttTime(simTime());
                pkt1->setType(0);
                pkt2->setType(0);
                pkt1->setKind(i);
                pkt2->setKind(i);
                double ranVal = uniform(0, 0.1);
                sendDelayed(pkt1, ranVal, "toMac");
                sendDelayed(pkt2, ranVal, "toMac");
                it++;
            }
        }
        scheduleAt(simTime() + par("jitter") + ettInterval, ettTimer);
    }
}

int Ieee80211Etx::getEtx(const MACAddress &add, double &val)
{
    MacEtxNeighbor *neig;
    val = 1e300;
    int interface = -1;
    for (unsigned int i = 0; i < neighbors.size(); i++)
    {
        NeighborsMap::iterator it = neighbors[i].find(add);
        if (it == neighbors[i].end())
        {
            continue;
        }
        else
        {
            neig = it->second;
            int expectedPk = etxMeasureInterval / etxInterval;
            while (!neig->timeVector.empty() && (simTime() - neig->timeVector.front() > etxMeasureInterval))
                neig->timeVector.erase(neig->timeVector.begin());
            int pkRec = neig->timeVector.size();
            double pr = pkRec / expectedPk;
            double ps = neig->getPackets() / expectedPk;
            if (pr > 1)
                pr = 1;
            if (ps > 1)
                ps = 1;
            double results;
            if (ps == 0 || pr == 0)
                results = 1e100;
            results = 1 / (ps * pr);
            if (results<val)
            {
                val = results;
                interface = (int)i;
            }
        }
    }
    return interface;
}

double Ieee80211Etx::getEtx(const MACAddress &add, const int &iface)
{
    NeighborsMap::iterator it = neighbors[iface].find(add);
    MacEtxNeighbor *neig;
    if (it==neighbors[iface].end())
    {
        return -1;
    }
    else
    {
        neig = it->second;
        int expectedPk = etxMeasureInterval/etxInterval;
        while (!neig->timeVector.empty() && (simTime()-neig->timeVector.front() >  etxMeasureInterval))
            neig->timeVector.erase(neig->timeVector.begin());
        int pkRec = neig->timeVector.size();
        double pr = pkRec/expectedPk;
        double ps = neig->getPackets()/expectedPk;
        if (pr>1) pr = 1;
        if (ps>1) ps = 1;
        if (ps == 0 || pr==0)
            return 1e300;
        return 1/(ps*pr);
    }
}

double Ieee80211Etx::getEtt(const MACAddress &add, const int &iface)
{
    if (ettInterval<=0 || ettSize1 <= 0 || ettSize2<=0)
	    return -1;

    NeighborsMap::iterator it = neighbors[iface].find(add);
    MacEtxNeighbor *neig;
    if (it==neighbors[iface].end())
    {
        return -1;
    }
    else
    {
        neig = it->second;
        if (neig->timeETT.empty())
            return -1;
        int expectedPk = etxMeasureInterval/etxInterval;
        while (!neig->timeVector.empty() && (simTime()-neig->timeVector.front() >  etxMeasureInterval))
            neig->timeVector.erase(neig->timeVector.begin());
        int pkRec = neig->timeVector.size();
        double pr = pkRec/expectedPk;
        double ps = neig->getPackets()/expectedPk;
        if (pr>1) pr = 1;
        if (ps>1) ps = 1;
        if (ps == 0 || pr==0)
            return 1e300;
        double etx =  1/(ps*pr);
        simtime_t minTime = 10000.0;
        for (unsigned int i = 0; i<neig->timeETT.size(); i++)
            if (minTime>neig->timeETT[i])
                minTime = neig->timeETT[i];
        double bw = ettSize2/minTime;
        return etx*(etxSize/bw);
    }
}


int Ieee80211Etx::getEtt(const MACAddress &add, double &val)
{
    if (ettInterval <= 0 || ettSize1 <= 0 || ettSize2 <= 0)
        return -1;
    val = 1e300;
    int interface = -1;
    for (unsigned int i = 0; i < neighbors.size(); i++)
    {
        NeighborsMap::iterator it = neighbors[i].find(add);
        MacEtxNeighbor *neig;
        if (it == neighbors[i].end())
        {
            continue;
        }
        else
        {
            neig = it->second;
            if (neig->timeETT.empty())
                return -1;
            int expectedPk = etxMeasureInterval / etxInterval;
            while (!neig->timeVector.empty() && (simTime() - neig->timeVector.front() > etxMeasureInterval))
                neig->timeVector.erase(neig->timeVector.begin());
            int pkRec = neig->timeVector.size();
            double pr = pkRec / expectedPk;
            double ps = neig->getPackets() / expectedPk;
            if (pr > 1)
                pr = 1;
            if (ps > 1)
                ps = 1;
            double result;
            if (ps == 0 || pr == 0)
                result = 1e100;
            else
            {
                double etx = 1 / (ps * pr);
                simtime_t minTime = 100.0;
                for (unsigned int i = 0; i < neig->timeETT.size(); i++)
                    if (minTime > neig->timeETT[i])
                        minTime = neig->timeETT[i];
                double bw = ettSize2 / minTime;
                result = etx * (etxSize / bw);
            }
            if (result<val)
            {
                val = result;
                interface = (int)i;
            }
        }
    }
    return interface;
}

double Ieee80211Etx::getPrec(const MACAddress &add, const int &iface)
{
    NeighborsMap::iterator it = neighbors[iface].find(add);
    MacEtxNeighbor *neig;
    if (it==neighbors[iface].end())
    {
        return 0;
    }
    else
    {
        neig = it->second;
        if (neig->signalToNoiseAndSignal.empty())
            return 0;

        double sum = 0;
        std::vector<SNRDataTime>::iterator itNeig;

        for (itNeig = neig->signalToNoiseAndSignal.begin(); itNeig!=neig->signalToNoiseAndSignal.end();)
        {
            if ((simTime()- itNeig->snrTime)>powerWindowTime)
            {
                std::vector<SNRDataTime>::iterator itAux = itNeig+1;
                neig->signalToNoiseAndSignal.erase(itNeig);
                itNeig = itAux;
                continue;
            }
            sum += itNeig->signalPower;
            itNeig++;
        }
        return sum/neig->signalToNoiseAndSignal.size();
    }
}


int Ieee80211Etx::getPrec(const MACAddress &add, double &val)
{
    val = 0;
    int interface = -1;
    for (unsigned int i = 0; i < neighbors.size(); i++)
    {
        NeighborsMap::iterator it = neighbors[i].find(add);
        MacEtxNeighbor *neig;
        if (it == neighbors[i].end())
        {
            continue;
        }
        else
        {
            neig = it->second;
            if (neig->signalToNoiseAndSignal.empty())
                continue;

            double sum = 0;
            std::vector<SNRDataTime>::iterator itNeig;

            for (itNeig = neig->signalToNoiseAndSignal.begin(); itNeig != neig->signalToNoiseAndSignal.end();)
            {
                if ((simTime() - itNeig->snrTime) > powerWindowTime)
                {
                    std::vector<SNRDataTime>::iterator itAux = itNeig + 1;
                    neig->signalToNoiseAndSignal.erase(itNeig);
                    itNeig = itAux;
                    continue;
                }
                sum += itNeig->signalPower;
                itNeig++;
            }

            double result = sum / neig->signalToNoiseAndSignal.size();
            if (result>val)
            {
                val = result;
                interface = (int)i;
            }
        }
    }
    return interface;
}

double Ieee80211Etx::getSignalToNoise(const MACAddress &add, const int &iface)
{
    NeighborsMap::iterator it = neighbors[iface].find(add);
    MacEtxNeighbor *neig;
    if (it==neighbors[iface].end())
    {
        return 0;
    }
    else
    {
        neig = it->second;
        if (neig->signalToNoiseAndSignal.empty())
            return 0;

        double sum = 0;
        std::vector<SNRDataTime>::iterator itNeig;

        for (itNeig = neig->signalToNoiseAndSignal.begin(); itNeig!=neig->signalToNoiseAndSignal.end();)
        {
            if ((simTime()- itNeig->snrTime)>powerWindowTime)
            {
                std::vector<SNRDataTime>::iterator itAux = itNeig+1;
                neig->signalToNoiseAndSignal.erase(itNeig);
                itNeig = itAux;
                continue;
            }
            sum += itNeig->snrData;
            itNeig++;
        }
        return sum/neig->signalToNoiseAndSignal.size();
    }
}

int Ieee80211Etx::getSignalToNoise(const MACAddress &add, double &val)
{
    MacEtxNeighbor *neig;
    val = 0;
    int interface = -1;
    for (unsigned int i = 0; i < neighbors.size(); i++)
    {
        NeighborsMap::iterator it = neighbors[i].find(add);
        if (it == neighbors[i].end())
        {
            continue;
        }
        else
        {
            neig = it->second;
            if (neig->signalToNoiseAndSignal.empty())
                continue;

            double sum = 0;
            std::vector<SNRDataTime>::iterator itNeig;

            for (itNeig = neig->signalToNoiseAndSignal.begin(); itNeig != neig->signalToNoiseAndSignal.end();)
            {
                if ((simTime() - itNeig->snrTime) > powerWindowTime)
                {
                    std::vector<SNRDataTime>::iterator itAux = itNeig + 1;
                    neig->signalToNoiseAndSignal.erase(itNeig);
                    itNeig = itAux;
                    continue;
                }
                sum += itNeig->snrData;
                itNeig++;
            }
            double result = sum / neig->signalToNoiseAndSignal.size();
            if (result>val)
            {
                val = result;
                interface = (int)i;
            }
        }
    }
    return interface;
}

double Ieee80211Etx::getPacketErrorToNeigh(const MACAddress &add, const int &iface)
{
    NeighborsMap::iterator it = neighbors[iface].find(add);
    MacEtxNeighbor *neig;

    if (it == neighbors[iface].end())
    {
        return -1;
    }
    else
    {
        neig = it->second;
        int expectedPk = etxMeasureInterval / etxInterval;
        while (!neig->timeVector.empty() && (simTime() - neig->timeVector.front() > etxMeasureInterval))
            neig->timeVector.erase(neig->timeVector.begin());
        double ps = neig->getPackets() / expectedPk;
        if (ps > 1)
            ps = 1;
        return 1 - ps;
    }
}


int Ieee80211Etx::getPacketErrorToNeigh(const MACAddress &add, double &val)
{
    MacEtxNeighbor *neig;
    val = 1;
    int interface = -1;
    for (unsigned int i = 0; i < neighbors.size(); i++)
    {
        NeighborsMap::iterator it = neighbors[i].find(add);
        if (it == neighbors[i].end())
        {
            continue;
        }
        else
        {
            neig = it->second;
            int expectedPk = etxMeasureInterval / etxInterval;
            while (!neig->timeVector.empty() && (simTime() - neig->timeVector.front() > etxMeasureInterval))
                neig->timeVector.erase(neig->timeVector.begin());
            double ps = neig->getPackets() / expectedPk;
            if (ps > 1)
                ps = 1;
            double resul = 1 - ps;
            if (val>resul)
            {
                interface = i;
                val = resul;
            }
        }
    }
    return interface;
}

double Ieee80211Etx::getPacketErrorFromNeigh(const MACAddress &add, const int &iface)
{
    NeighborsMap::iterator it = neighbors[iface].find(add);
    MacEtxNeighbor *neig;
    if (it==neighbors[iface].end())
    {
        return -1;
    }
    else
    {
        neig = it->second;
        int expectedPk = etxMeasureInterval/etxInterval;
        while (!neig->timeVector.empty() && (simTime()-neig->timeVector.front() >  etxMeasureInterval))
            neig->timeVector.erase(neig->timeVector.begin());
        int pkRec = neig->timeVector.size();
        double pr = pkRec/expectedPk;
        if (pr>1) pr = 1;
        return 1-pr;
    }
}

int Ieee80211Etx::getPacketErrorFromNeigh(const MACAddress &add, double &val)
{
    val = 1;
    int interface = -1;
    for (unsigned int i = 0; i < neighbors.size(); i++)
    {
        NeighborsMap::iterator it = neighbors[i].find(add);
        MacEtxNeighbor *neig;
        if (it == neighbors[i].end())
        {
            continue;
        }
        else
        {
            neig = it->second;
            int expectedPk = etxMeasureInterval / etxInterval;
            while (!neig->timeVector.empty() && (simTime() - neig->timeVector.front() > etxMeasureInterval))
                neig->timeVector.erase(neig->timeVector.begin());
            int pkRec = neig->timeVector.size();
            double pr = pkRec / expectedPk;
            if (pr > 1)
                pr = 1;
            double resul = 1 - pr;
            if (val > resul)
            {
                interface = i;
                val = resul;
            }
        }
    }
    return interface;
}


void Ieee80211Etx::handleEtxMessage(MACETXPacket *msg)
{
    int interface = msg->getKind();
    if (interface<0 || interface>= (int)numInterfaces)
        interface = 0;
    NeighborsMap::iterator it = neighbors[interface].find(msg->getSource());
    MacEtxNeighbor *neig;
    if (it==neighbors[interface].end())
    {
        neig = new MacEtxNeighbor;
        neig->setAddress(msg->getSource());
        neighbors[interface][msg->getSource()] = neig;
    }
    else
        neig = it->second;
    while (!neig->timeVector.empty() && (simTime()-neig->timeVector.front() >  etxMeasureInterval))
        neig->timeVector.erase(neig->timeVector.begin());

    neig->timeVector.push_back(simTime());
    neig->setTime(simTime());
    neig->setPackets(0);
    neig->setNumFailures(0);
    for (unsigned int i = 0; i<msg->getNeighborsArraySize(); i++)
    {
        if (myAddress==msg->getNeighbors(i))
        {
            neig->setPackets(msg->getRecPackets(i));
            break;
        }
    }
    delete msg;
}

void Ieee80211Etx::handleBwMessage(MACBwPacket *msg)
{
    int interface = msg->getKind();
    if (interface<0 || interface>= (int)numInterfaces)
        interface = 0;
    NeighborsMap::iterator it = neighbors[interface].find(msg->getSource());
    MacEtxNeighbor *neig = NULL;
    if (it!=neighbors[interface].end())
    {
        neig = it->second;
        neig->setNumFailures(0);
    }

    if (!msg->getType())
    {
        if (msg->getByteLength()==ettSize1)
        {
            prevAddress = msg->getSource();
            prevTime = simTime();
            delete msg;
        }
        else if (msg->getByteLength()==ettSize2)
        {
            if (prevAddress == msg->getSource())
            {
                msg->setTime(simTime()-prevTime);
                msg->setType(1);
                msg->setDest(msg->getSource());
                msg->setByteLength(ettSize1);
                msg->setSource(myAddress);
                send(msg, "toMac");
            }
            else
                prevAddress = MACAddress::UNSPECIFIED_ADDRESS;

        }
        return;
    }
    if (!neig)
    {
        neig = new MacEtxNeighbor;
        neig->setAddress(msg->getSource());
        neighbors[interface][msg->getSource()] = neig;
    }
    if (msg->getByteLength()==ettSize1)
    {
        if (neig->timeETT.size() > (unsigned int)ettWindow)
            neig->timeETT.erase(neig->timeETT.begin());
        neig->timeETT.push_back(msg->getTime());
    }
}

void Ieee80211Etx::getNeighbors(std::vector<MACAddress> & add,const int &iface)
{
    Enter_Method_Silent();
    add.clear();
    for (NeighborsMap::iterator it = neighbors[iface].begin(); it != neighbors[iface].end(); it++)
    {
        add.push_back(it->second->getAddress());
    }
    return;
}

void Ieee80211Etx::receiveChangeNotification(int category, const cObject *details)
{
    Enter_Method("Ieee80211Etx llf");
    if (details==NULL)
        return;
    Ieee80211TwoAddressFrame *frame = dynamic_cast<Ieee80211TwoAddressFrame *>(const_cast<cObject*> (details));
    if (frame==NULL)
        return;
    if (category == NF_LINK_BREAK)
    {
        NeighborsMap::iterator it = neighbors[0].find(frame->getReceiverAddress());
        if (it!=neighbors[0].end())
        {
            it->second->setNumFailures(it->second->getNumFailures()+1);
            if (it->second->getNumFailures()>1)
            {
                delete it->second;
                neighbors[0].erase(it);
            }
        }
    }
    else if (category == NF_LINK_FULL_PROMISCUOUS)
    {
        NeighborsMap::iterator it = neighbors[0].find(frame->getTransmitterAddress());
        if (it!=neighbors[0].end())
            it->second->setNumFailures(0);
        if (powerWindow>0)
        {
            Radio80211aControlInfo * cinfo = dynamic_cast<Radio80211aControlInfo *> (frame->getControlInfo());
            // use only data frames
            if (!dynamic_cast<Ieee80211DataFrame *>(frame))
                return;
            if (cinfo)
            {
                if (it==neighbors[0].end())
                {
                    // insert new element
                    MacEtxNeighbor *neig = new MacEtxNeighbor;
                    neig->setAddress(frame->getTransmitterAddress());
                    neighbors[0].insert(std::pair<MACAddress, MacEtxNeighbor*>(frame->getTransmitterAddress(),neig));
                    it = neighbors[0].find(frame->getTransmitterAddress());
                }
                if (!it->second->signalToNoiseAndSignal.empty())
                {
                    while ((int)it->second->signalToNoiseAndSignal.size()>powerWindow-1)
                        it->second->signalToNoiseAndSignal.erase(it->second->signalToNoiseAndSignal.begin());
                    while (simTime() - it->second->signalToNoiseAndSignal.front().snrTime>powerWindowTime && !it->second->signalToNoiseAndSignal.empty())
                        it->second->signalToNoiseAndSignal.erase(it->second->signalToNoiseAndSignal.begin());
                }

                SNRDataTime snrDataTime;
                snrDataTime.signalPower = cinfo->getRecPow();
                snrDataTime.snrData = cinfo->getSnr();
                snrDataTime.snrTime = simTime();
                snrDataTime.testFrameDuration = cinfo->getTestFrameDuration();
                snrDataTime.testFrameError = cinfo->getTestFrameError();
                snrDataTime.airtimeMetric = cinfo->getAirtimeMetric();
                if (snrDataTime.airtimeMetric)
                    snrDataTime.airtimeValue = (uint32_t)ceil((snrDataTime.testFrameDuration/10.24e-6)/(1-snrDataTime.testFrameError));
                else
                    snrDataTime.airtimeValue = 0xFFFFFFF;
                it->second->signalToNoiseAndSignal.push_back(snrDataTime);
                if (snrDataTime.airtimeMetric)
                {
                    // found the best
                    uint32_t cost = 0xFFFFFFFF;
                    for (unsigned int i = 0; i < it->second->signalToNoiseAndSignal.size(); i++)
                    {
                        if (it->second->signalToNoiseAndSignal[i].airtimeMetric && cost > it->second->signalToNoiseAndSignal[i].airtimeValue)
                            cost = it->second->signalToNoiseAndSignal[i].airtimeValue;
                    }
                    it->second->setAirtimeMetric(cost);
                }
                else
                    it->second->setAirtimeMetric(0xFFFFFFF);
            }
        }
    }
}


uint32_t Ieee80211Etx::getAirtimeMetric(const MACAddress &addr, const int &iface)
{
    NeighborsMap::iterator it = neighbors[iface].find(addr);
    if (it != neighbors[iface].end())
    {
        while (!it->second->signalToNoiseAndSignal.empty() && (simTime() - it->second->signalToNoiseAndSignal.front().snrTime > powerWindowTime))
            it->second->signalToNoiseAndSignal.erase(it->second->signalToNoiseAndSignal.begin());
        if (it->second->signalToNoiseAndSignal.empty() && (simTime() - it->second->getTime() > maxLive))
        {
            neighbors[iface].erase(it);
            return 0xFFFFFFF;
        }
        else if (it->second->signalToNoiseAndSignal.empty())
            return 0xFFFFFFF;
        else
            return it->second->getAirtimeMetric();
    }
    else
        return 0xFFFFFFF;
}

void Ieee80211Etx::getAirtimeMetricNeighbors(std::vector<MACAddress> &addr, std::vector<uint32_t> &cost, const int &iface)
{
    addr.clear();
    cost.clear();
    for (NeighborsMap::iterator it = neighbors[iface].begin(); it != neighbors[iface].end();)
    {
        while (simTime() - it->second->signalToNoiseAndSignal.front().snrTime > powerWindowTime)
            it->second->signalToNoiseAndSignal.erase(it->second->signalToNoiseAndSignal.begin());
        if (it->second->signalToNoiseAndSignal.empty() && (simTime() - it->second->getTime() > maxLive))
        {
            NeighborsMap::iterator itAux = it;
            it++;
            neighbors[iface].erase(itAux);
        }
        else if (it->second->signalToNoiseAndSignal.empty())
        {
            it++;
        }
        else if (it->second->signalToNoiseAndSignal.empty())
        {
            addr.push_back(it->first);
            cost.push_back(it->second->getAirtimeMetric());
            it++;
        }
    }
}



void Ieee80211Etx::setNumInterfaces(unsigned int iface)
{
    if (iface == 0)
        return;
    numInterfaces = iface;
    if (neighbors.size()<numInterfaces)
    {

        for (unsigned int i = numInterfaces; i< neighbors.size(); i++)
        {
            while (neighbors[i].empty())
            {
                delete neighbors[i].begin()->second;
                neighbors[i].erase(neighbors[i].begin());
            }
        }
    }
    neighbors.resize(numInterfaces);
}

std::string Ieee80211Etx::detailedInfo() const
{
   return info();
}

std::string Ieee80211Etx::info() const
{
    std::stringstream out;
    for (unsigned int i = 0; i < neighbors.size(); ++i)
    {
        out << "interface : " << i << "num neighbors :" << neighbors[i].size() <<"\n";
        for (NeighborsMap::const_iterator it = neighbors[i].begin(); it != neighbors[i].end(); it++)
        {
            out << "address : " << it->second->getAddress().str() <<"\n";;
        }
    }
    return out.str();
}
