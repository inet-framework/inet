//
// Copyright (C) 2010 Philipp Berndt
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

#include "InternetPropagationDelayGate.h"

Define_Module(InternetPropagationDelayGate);

#ifdef HAVE_GNPLIB

#include <gnplib/impl/network/gnp/GeoLocationOracle.h>
#include <gnplib/impl/network/gnp/GnpLatencyModel.h>
#include <gnplib/impl/network/IPv4NetID.h>
#include "IPDatagram.h"

namespace GeoLocationOracle=gnplib::impl::network::gnp::GeoLocationOracle;
using gnplib::impl::network::IPv4NetID;


void InternetPropagationDelayGate::initialize(int stage)
{
    if (stage==1)
    {
        outGateId=findGate("ipOut");

        // This must be after stage 0 where GnpNetworkConfigurator creates GnpLatencyModel
        GeoLocationOracle::lm->setUsePingErJitter(par("usePingErJitter"));
        GeoLocationOracle::lm->setUsePingErRttData(par("usePingErRttData"));
        GeoLocationOracle::lm->setUseAnalyticalRtt(par("useAnalyticalRtt"));
        GeoLocationOracle::lm->setUsePingErPacketLoss(par("usePingErPacketLoss"));
        GeoLocationOracle::lm->setUseAccessLatency(par("useAccessLatency"));
    }
}

void InternetPropagationDelayGate::handleMessage(cMessage *msg)
{
    IPDatagram*pkt(dynamic_cast<IPDatagram*>(msg));
    if (pkt)
    {
        const IPv4NetID src(pkt->getSrcAddress().getInt());
        const IPv4NetID dst(pkt->getDestAddress().getInt());

        double delay=GeoLocationOracle::getInternetPropagationDelay(src, dst);
        simtime_t propDelay=delay/1000.0;
        ev<<"Delaying packet from "<<src<<" to "<<dst<<" by "<<propDelay<<endl;

        sendDelayed(pkt, propDelay, outGateId);
    } else
    { // All non-IP traffic passes through undelayed
        send(msg, outGateId);
    }
}

#else

// gnplib was not found => compile as stub

void InternetPropagationDelayGate::initialize(int stage)
{
    error("Please compile INET with gnplib support to use this module!");
}

void InternetPropagationDelayGate::handleMessage(cMessage *msg) {}

#endif
