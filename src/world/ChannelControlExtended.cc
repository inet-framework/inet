/***************************************************************************
 * file:        ChannelControlExtended.cc
 *
 * copyright:   (C) 2005 Andras Varga
 * copyright:   (C) 2009 Juan-Carlos Maureira
 * copyright:   (C) 2009 Alfonso Ariza
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
 **************************************************************************/


#include "ChannelControlExtended.h"
#include "FWMath.h"
#include <cassert>

using namespace std;
#define coreEV (ev.isDisabled()||!coreDebug) ? ev : ev << "ChannelControl: "

Define_Module(ChannelControlExtended);


//std::ostream& operator<<(std::ostream& os, const ChannelControl::HostEntry& h)
//{
//    os << h.host->getFullPath() << " (x=" << h.pos.x << ",y=" << h.pos.y << "), "
//       << h.neighbors.size() << " neighbor(s)";
//   return os;
//}

// New operator to show the HostEntry now showing each radio channel and gate id
std::ostream& operator<<(std::ostream& os, const ChannelControlExtended::HostEntryExtended& h)
{
    os << h.host->getFullPath() << " (x=" << h.pos.x << ",y=" << h.pos.y << "), " << h.neighbors.size() << " neighbor(s)" << endl;
    int count = 0;
    for (ChannelControlExtended::RadioList::const_iterator it = h.radioList.begin(); it!=h.radioList.end(); it++)
    {
        os << "                 radio " << count << " " << (*it).radioModule << " channel " << (*it).channel << " gate id " << (*it).hostGateId << endl;
        count++;
    }
    return os;
}


std::ostream& operator<<(std::ostream& os, const ChannelControlExtended::TransmissionList& tl)
{
    for (ChannelControlExtended::TransmissionList::const_iterator it = tl.begin(); it != tl.end(); ++it)
        os << endl << *it;
    return os;
}


// NEW METHODS TO SUPPORT MULTIPLES RADIOS

/* Check if the host entry has a radio in the given channel */
bool ChannelControlExtended::HostEntryExtended::isReceivingInChannel(int channel,double freq)
{
    /* if there is some radio on the channel on this host */
    for (ChannelControlExtended::RadioList::const_iterator it = radioList.begin(); it!=radioList.end(); it++)
    {
        if (((*it).channel==channel) && (fabs(((*it).radioCarrier - freq)/freq)<=percentage))
        {
            return(true);
        }
    }
    return(false);
}

/* return a std::list with the radioIn host gates that points to radios on the given channel */
ChannelControlExtended::radioGatesList ChannelControlExtended::HostEntryExtended::getHostGatesOnChannel(int channel,double freq)
{

    ChannelControlExtended::radioGatesList theRadioList;

    if (freq==0.0)
        freq=carrierFrequency;

    for (ChannelControlExtended::RadioList::const_iterator it = radioList.begin(); it!=radioList.end(); it++)
    {
        int channelGate = it->channel;
        double freqGate = it->radioCarrier;
        if ((channelGate==channel) && (fabs((freqGate - freq)/freqGate)<=percentage))
        {
            cGate* radioGate = NULL;
            if ((*it).radioInGate)
            {
                radioGate = it->radioInGate;
            }
            else
            {
                if (!this->radioInGate)
                    this->radioInGate = (this->host)->gate("radioIn");
                radioGate = this->radioInGate;
            }
            if (radioGate != NULL )
            {
                theRadioList.push_back(radioGate);
            }
        }
    }
    return(theRadioList);
}

void ChannelControlExtended::HostEntryExtended::registerRadio(cModule* ar)
{
    ChannelControlExtended::RadioEntry ra;
    ra.radioModule = ar;
    ra.channel = -1;
    ra.radioInGate = NULL;
    ra.radioCarrier=carrierFrequency;

    cGate* radioIn = ar->gate("radioIn");

    for (ChannelControlExtended::RadioList::iterator it = radioList.begin(); it!=radioList.end(); it++)
    {
        if (it->hostGateId == radioIn->getId())
            return; // Is register
    }

    // JcM
    // look for the host gate id for each radio module.

    while (radioIn!=NULL && radioIn->getOwnerModule() != host )
    {
        radioIn = radioIn->getPreviousGate();
    }

    if (radioIn!=NULL && radioIn->getOwnerModule() == host)
    {
        ra.hostGateId = radioIn->getId();
        ra.radioInGate = radioIn;
    }
    else
    {
        ra.hostGateId = -1;
        ra.radioInGate = NULL;
    }

    ChannelControlExtended::HostEntryExtended::radioList.push_back(ra);
}


void ChannelControlExtended::HostEntryExtended::unregisterRadio(cModule* ar)
{

    for (ChannelControlExtended::RadioList::iterator it = radioList.begin(); it!=radioList.end(); it++)
    {
        ChannelControlExtended::RadioEntry ra = (*it);
        if (ra.radioModule == ar)
        {
            ChannelControlExtended::HostEntryExtended::radioList.erase(it);
            return;
        }
    }
}

/*
bool ChannelControl::HostEntry::updateRadioChannel(cModule* ar,int ch) {
   EV << "HostEntry " << this->host->getFullPath() << " updating radio channel to " << ar << " ch " << ch  << endl;
   for(ChannelControl::RadioList::iterator it = radioList.begin();it!=radioList.end();it++) {
      EV << "UpdateRadioChannel ar: " << ar->getFullPath() << " radioModule :" << (*it).radioModule->getFullPath() << endl;
      if ((*it).radioModule==ar) {
        (*it).channel = ch;
        (*it).radioCarrier=carrierFrequency;
        return(true);
      }
   }
   return(false);
}
*/

bool ChannelControlExtended::HostEntryExtended::updateRadioChannel(cModule* ar,int ch,double freq)
{
    EV << "HostEntry " << this->host->getFullPath() << " updating radio channel to " << ar << " ch " << ch  << endl;
    for (ChannelControlExtended::RadioList::iterator it = radioList.begin(); it!=radioList.end(); it++)
    {
        EV << "UpdateRadioChannel ar: " << ar->getFullPath() << " radioModule :" << (*it).radioModule->getFullPath() << endl;
        if ((*it).radioModule==ar)
        {
            (*it).channel = ch;
            (*it).radioCarrier=carrierFrequency;
            if (freq>0.0)
                (*it).radioCarrier=freq;
            return(true);
        }
    }
    return(false);
}

bool ChannelControlExtended::HostEntryExtended::isRadioRegistered(cModule* r)
{
    for (ChannelControlExtended::RadioList::iterator it = radioList.begin(); it!=radioList.end(); it++)
    {
        if ((*it).radioModule==r)
        {
            return(true);
        }
    }
    return(false);
}

cModule* ChannelControlExtended::getHostByRadio(AbstractRadio* ar)
{
    Enter_Method_Silent();
    for (HostList::iterator it = hosts.begin(); it != hosts.end(); it++)
    {
        if (it->isRadioRegistered((cModule*)ar))
        {
            return it->host;
        }
    }
    return NULL;
}

// END NEW METHODS FOR SUPPORT MULTIPLES RADIOS

ChannelControlExtended::ChannelControlExtended()
{
}

ChannelControlExtended::~ChannelControlExtended()
{
    for (unsigned int i = 0; i < transmissions.size(); i++)
        for (TransmissionList::iterator it = transmissions[i].begin(); it != transmissions[i].end(); it++)
            delete *it;
}

ChannelControlExtended *ChannelControlExtended::get()
{
    ChannelControl * cc = ChannelControl::get();
    if (!cc)
    {
        cc = dynamic_cast<ChannelControl *>(simulation.getModuleByPath("channelcontrolextended"));
        if (!cc)
            cc = dynamic_cast<ChannelControl *>(simulation.getModuleByPath("channelcontrolextended"));
        if (!cc)
            throw cRuntimeError("Could not find ChannelControl module");
    }
    ChannelControlExtended *ccExt = dynamic_cast<ChannelControlExtended *>(cc);
    return ccExt;
}

/**
 * Sets up the playgroundSize and calculates the
 * maxInterferenceDistance
 *
 * @ref calcInterfDist
 */
void ChannelControlExtended::initialize()
{
    coreDebug = hasPar("coreDebug") ? (bool) par("coreDebug") : false;

    coreEV << "initializing ChannelControlExtended \n";
    EV << "initializing ChannelControlExtended \n";
    playgroundSize.x = par("playgroundSizeX");
    playgroundSize.y = par("playgroundSizeY");

    numChannels = par("numChannels");
    transmissions.resize(numChannels);
    percentage = par("percentage");

    lastOngoingTransmissionsUpdate = 0;

    carrierFrequency = par("carrierFrequency");
    maxInterferenceDistance = calcInterfDist();

    WATCH(maxInterferenceDistance);
    WATCH_LIST(hosts);
    WATCH_VECTOR(transmissions);
    updateDisplayString(getParentModule());
}


ChannelControl::HostRef ChannelControlExtended::registerHost(cModule * host, const Coord& initialPos,cGate *radioInGate)
{
    Enter_Method_Silent();
    if (lookupHost(host) != NULL)
        error("ChannelControlExtended::registerHost(): host (%s)%s already registered",
              host->getClassName(), host->getFullPath().c_str());

    HostEntryExtended he;
    he.host = host;
    he.pos = initialPos;
    he.radioInGate = radioInGate;
    he.carrierFrequency=par("carrierFrequency");
    he.percentage=par ("percentage");
    he.isNeighborListValid = false;
    // TODO: get it from caller
    he.channel = 0;
    hosts.push_back(he);
    return &hosts.back(); // last element
}




void ChannelControlExtended::unregisterHost(cModule *host)
{
    Enter_Method_Silent();
    for (HostList::iterator it = hosts.begin(); it != hosts.end(); it++)
    {
        if (it->host == host)
        {
            HostRef h = &*it;

            // erase host from all registered hosts' neighbor list
            for (HostList::iterator i2 = hosts.begin(); i2 != hosts.end(); ++i2)
            {
                HostRef h2 = &*i2;
                h2->neighbors.erase(h);
                h2->isNeighborListValid = false;
                h->isNeighborListValid = false;
            }

            // erase host from registered hosts
            hosts.erase(it);
            return;
        }
    }
    error("unregisterHost failed: no such host");
}

ChannelControl::HostRef ChannelControlExtended::lookupHost(cModule *host)
{
    Enter_Method_Silent();
    for (HostList::iterator it = hosts.begin(); it != hosts.end(); it++)
        if (it->host == host)
            return &(*it);
    return 0;
}

const ChannelControl::HostRefVector& ChannelControlExtended::getNeighbors(HostRef haux)
{
    Enter_Method_Silent();
    HostRefExtended h = dynamic_cast <ChannelControlExtended::HostRefExtended>(haux);
    if (!h)
        error(" not HostRefExtended");
    if (!h->isNeighborListValid)
    {
        h->neighborList.clear();
        for (std::set<HostRefExtended>::const_iterator it = h->neighbors.begin(); it != h->neighbors.end(); it++)
            h->neighborList.push_back(*it);
        h->isNeighborListValid = true;
    }
    return h->neighborList;
}

const ChannelControlExtended::TransmissionList& ChannelControlExtended::getOngoingTransmissions(const int channel)
{
    Enter_Method_Silent();

    checkChannel(channel);
    purgeOngoingTransmissions();
    return transmissions[channel];
}

void ChannelControlExtended::addOngoingTransmission(HostRef h, AirFrame *frame)
{
    Enter_Method_Silent();

    // we only keep track of ongoing transmissions so that we can support
    // NICs switching channels -- so there's no point doing it if there's only
    // one channel
    if (numChannels==1)
    {
        delete frame;
        return;
    }

    // purge old transmissions from time to time
    if (simTime() - lastOngoingTransmissionsUpdate > TRANSMISSION_PURGE_INTERVAL)
    {
        purgeOngoingTransmissions();
        lastOngoingTransmissionsUpdate = simTime();
    }

    // register ongoing transmission
    take(frame);
    frame->setTimestamp(); // store time of transmission start
    AirFrameExtended * frameExtended = dynamic_cast<AirFrameExtended*>(frame);
    if (!frameExtended)
    {
        frameExtended = new AirFrameExtended();
        AirFrame * faux = frameExtended;
        *faux = (*frame);
        delete frame;
    }

    transmissions[frame->getChannelNumber()].push_back(frameExtended);
}

void ChannelControlExtended::purgeOngoingTransmissions()
{
    for (int i = 0; i < numChannels; i++)
    {
        for (TransmissionList::iterator it = transmissions[i].begin(); it != transmissions[i].end();)
        {
            TransmissionList::iterator curr = it;
            AirFrame *frame = *it;
            it++;

            if (frame->getTimestamp() + frame->getDuration() + TRANSMISSION_PURGE_INTERVAL < simTime())
            {
                delete frame;
                transmissions[i].erase(curr);
            }
        }
    }
}


/**
 * Calculation of the interference distance based on the transmitter
 * power, wavelength, pathloss coefficient and a threshold for the
 * minimal receive Power
 *
 * You may want to overwrite this function in order to do your own
 * interference calculation
 */
double ChannelControlExtended::calcInterfDistExtended(double freq)
{
    double SPEED_OF_LIGHT = 300000000.0;
    double interfDistance;

    //the carrier frequency used
    // double carrierFrequency = par("carrierFrequency");
    //maximum transmission power possible
    double pMax = par("pMax");
    //signal attenuation threshold
    double sat = par("sat");
    //path loss coefficient
    double alpha = par("alpha");

    double waveLength = (SPEED_OF_LIGHT / freq);
    //minimum power level to be able to physically receive a signal
    double minReceivePower = pow(10.0, sat / 10.0);

    interfDistance = pow(waveLength * waveLength * pMax /
                         (16.0 * M_PI * M_PI * minReceivePower), 1.0 / alpha);

    coreEV << "max interference distance:" << interfDistance << endl;

    return interfDistance;
}

void ChannelControlExtended::updateConnections(HostRef aux)
{

    HostRefExtended h = (HostRefExtended) aux;
    if (!h)
        error("ChannelControlExtended::updateConnections");

    Coord& hpos = h->pos;
    double maxDistSquared = maxInterferenceDistance * maxInterferenceDistance;
    for (HostList::iterator it = hosts.begin(); it != hosts.end(); ++it)
    {
        HostEntryExtended *hi = &(*it);
        if (hi == h)
            continue;

        // get the distance between the two hosts.
        // (omitting the square root (calling sqrdist() instead of distance()) saves about 5% CPU)
        bool inRange = hpos.sqrdist(hi->pos) < maxDistSquared;

        if (inRange)
        {
            // nodes within communication range: connect
            if (h->neighbors.insert(hi).second == true)
            {
                hi->neighbors.insert(h);
                h->isNeighborListValid = hi->isNeighborListValid = false;
            }
        }
        else
        {
            // out of range: disconnect
            if (h->neighbors.erase(hi))
            {
                hi->neighbors.erase(h);
                h->isNeighborListValid = hi->isNeighborListValid = false;
            }
        }
    }
}


void ChannelControlExtended::updateHostChannel(HostRef h, const int channel)
{
    Enter_Method_Silent();
    checkChannel(channel);
    HostRefExtended hExt = (HostRefExtended) h;
    if (hExt->radioList.empty())
    {
        cModule *module = hExt->host;
        ChannelControlExtended::RadioEntry ra;
        cGate *radioIn = module->gate("radioIn");
        ra.radioModule = radioIn->getOwnerModule();
        ra.channel = channel;
        ra.hostGateId = radioIn->getId();
        ra.radioCarrier = carrierFrequency;
        hExt->radioList.push_back(ra);
    }
    hExt->radioList.front().channel= channel;
}


/*
void ChannelControl::updateHostChannel(HostRef h, const int channel, cModule* ca)
{
    Enter_Method_Silent();

    checkChannel(channel);

    h->updateRadioChannel(ca,channel);
}
*/

void ChannelControlExtended::updateHostChannel(HostRef h, const int channel, cModule* ca,double freq)
{
    Enter_Method_Silent();

    checkChannel(channel);

    ((HostRefExtended)h)->updateRadioChannel(ca,channel,freq);
}


void ChannelControlExtended::sendToChannel(cSimpleModule *srcRadioMod, HostRef srcHost, AirFrame *airFrame)
{
    const HostRefVector& neighbors = getNeighbors(srcHost);
    int n = neighbors.size();
    AirFrameExtended * msgAux = dynamic_cast<AirFrameExtended*>(airFrame);
    for (int i=0; i<n; i++)
    {
        HostRefExtended h = dynamic_cast<HostRefExtended>(neighbors[i]);

        ChannelControlExtended::radioGatesList theRadioList;
        if (msgAux)
            theRadioList = h->getHostGatesOnChannel(airFrame->getChannelNumber(),msgAux->getCarrierFrequency());
        else
            theRadioList = h->getHostGatesOnChannel(airFrame->getChannelNumber(),0.0);
        // if there are some radio on the channel.
        int sizeGate = theRadioList.size();
        if (sizeGate>0)
        {
            for (ChannelControlExtended::radioGatesList::iterator rit=theRadioList.begin(); rit != theRadioList.end(); rit++)
            {
                cGate* radioGate = (*rit);
                coreEV << "sending message to host listening on the same channel "  << radioGate->getFullName() << endl;
                // account for propagation delay, based on distance in meters
                // Over 300m, dt=1us=10 bit times @ 10Mbps
                simtime_t delay = srcHost->pos.distance(h->pos) / LIGHT_SPEED;
                srcRadioMod->sendDirect((cMessage *)airFrame->dup(),delay, airFrame->getDuration(),radioGate);
            }
        }
        else
        {
            coreEV << "skipping host listening on a different channel\n";
        }
    }
    // register transmission in ChannelControl
    addOngoingTransmission(srcHost, airFrame);
}

