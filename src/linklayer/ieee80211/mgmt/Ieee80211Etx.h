//
// Copyright (C) 2006 Andras Varga
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

#ifndef IEEE80211_ETX_ADHOC_H
#define IEEE80211_ETX_ADHOC_H

#include <omnetpp.h>
#include "uint128.h"
#include "IInterfaceTable.h"
#include "ETXPacket_m.h"
#include "NotificationBoard.h"

/**
 *
 * @author Alfonso Ariza
 */
 struct SNRDataTime {
    double snrData;
    double signalPower;
    simtime_t snrTime;
    SNRDataTime& operator=(const SNRDataTime& other)
    {
        if (this==&other) return *this;
        this->snrData = other.snrData;
        this->snrTime = other.snrTime;
        this->signalPower = other.signalPower;
        return *this;
    }
}; // Store information about the SNR and the time that that measure was store

class MacEtxNeighbor
{
  private:
    MACAddress address;
    simtime_t  time;
    simtime_t  ettTime;
    simtime_t  ett1Time;
    simtime_t  ett2Time;
    int     packets;
    int     numFailures;
  public:
    std::vector<simtime_t> timeVector;
    std::vector<simtime_t> timeETT;
    std::vector<SNRDataTime> signalToNoiseAndSignal; // S/N received
  public:
    MacEtxNeighbor() {packets = 0; time = 0; numFailures = 0;}
    ~MacEtxNeighbor()
    {
        timeVector.clear();
        timeETT.clear();
        signalToNoiseAndSignal.clear();
    }
    // this vector store a window of values
    void setAddress(const MACAddress &addr) {address = addr;}
    MACAddress getAddress() const {return address;}
    void setTime(const simtime_t &t) {time = t;}
    simtime_t getTime() const {return time;}

    void setEttTime(const simtime_t &t) {ettTime = t;}
    simtime_t getEttTime() const {return ettTime;}
    void setEtt1Time(const simtime_t &t) {ett1Time = t;}
    simtime_t getEtt1Time() const {return ett1Time;}
    void setEtt2Time(const simtime_t &t) {ett2Time = t;}
    simtime_t getEtt2Time() const {return ett2Time;}

    void setNumFailures(int num) {numFailures = num;}
    int getNumFailures() {return numFailures;}

    void setPackets(const int &p) {packets = p;}
    int getPackets() const {return packets;}
};

typedef std::map<MACAddress,MacEtxNeighbor*> NeighborsMap;

class INET_API Ieee80211Etx : public cSimpleModule, public MacEstimateCostProcess, public INotifiable
{
  protected:
    MACAddress myAddress;
    NeighborsMap neighbors;
    cMessage * etxTimer;
    cMessage * ettTimer;
    simtime_t etxInterval;
    simtime_t ettInterval;
    simtime_t etxMeasureInterval;
    int ettWindow;
    int etxSize;
    int ettSize1;
    int ettSize2;
    simtime_t maxLive;
    MACAddress prevAddress;
    simtime_t  prevTime;
    int powerWindow;
    simtime_t powerWindowTime;

  protected:
    virtual int numInitStages() const {return 3;}
    virtual void initialize(int);
    ~Ieee80211Etx();
    virtual void handleMessage(cMessage*);
    /** Implements abstract to use routing protocols in the mac layer */
    virtual void handleEtxMessage(MACETXPacket *);
    /** Implements abstract Ieee80211MgmtBase method */
    virtual void handleTimer(cMessage *msg);
    /** Implements abstract Ieee80211MgmtBase method */
    virtual void handleBwMessage(MACBwPacket *);
    virtual void receiveChangeNotification(int category, const cPolymorphic *details);
  public:
    virtual double getEtt(const MACAddress &add);
    virtual double getEtx(const MACAddress &add);
    virtual double getPrec(const MACAddress &add);
    virtual double getSignalToNoise(const MACAddress &add);
    virtual double getPacketErrorToNeigh(const MACAddress &add);
    virtual double getPacketErrorFromNeigh(const MACAddress &add);
    virtual void getNeighbors(std::vector<MACAddress> &);

  public:
    Ieee80211Etx() {};
    void setAddress(const MACAddress &add) {myAddress = add;}
    virtual double getCost(int i, MACAddress &add)
    {
        switch (i)
        {
        case 0:
            return getEtt(add);
            break;
        case 1:
            return getEtx(add);
            break;
        case 2:
            return getPrec(add);
            break;
        case 3:
            return getSignalToNoise(add);
            break;
        default:
            return -1;
            break;
        }
    }

    virtual double getNumCost() {return 4;}
    virtual int getNumNeighbors() {return neighbors.size();}
    virtual int getNeighbors(MACAddress add[])
    {
        int i = 0;
        for (NeighborsMap::iterator it = neighbors.begin(); it != neighbors.end(); it++)
        {
            add[i] = it->second->getAddress();
            i++;
        }
        return i;
    }

};

#endif


