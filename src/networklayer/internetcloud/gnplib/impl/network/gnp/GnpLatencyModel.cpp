#include <iostream>
#include <cmath>
#include <gnplib/api/Simulator.h>
#include <gnplib/api/common/random/Rng.h>
#include <gnplib/api/network/NetPosition.h>
#include <gnplib/impl/network/gnp/GeoLocationOracle.h>
#include <gnplib/impl/network/gnp/GnpLatencyModel.h>
#include <gnplib/impl/network/gnp/GnpNetLayer.h>
#include <gnplib/impl/network/gnp/topology/PingErLookup.h>

using std::string;
using std::cerr;
using std::endl;

namespace Simulator=gnplib::api::Simulator;
namespace Rng=gnplib::api::common::random::Rng;
using gnplib::api::network::NetLayer;
using gnplib::api::network::NetPosition;
using gnplib::impl::network::gnp::GnpLatencyModel;
using gnplib::impl::network::gnp::topology::LognormalDist;
using gnplib::impl::network::gnp::topology::CountryLookup;
using gnplib::impl::network::gnp::topology::PingErLookup;

// const int GnpLatencyModel::MSS=IPv4Message.MTU_SIZE-IPv4Message.HEADER_SIZE-TCPMessage.HEADER_SIZE;

PingErLookup* GnpLatencyModel::pingErLookup(0);

CountryLookup* GnpLatencyModel::countryLookup(0);

GnpLatencyModel::GnpLatencyModel()
: usePingErInsteadOfGnp(false),
useAnalyticalFunctionInsteadOfGnp(false),
usePingErJitter(false),
usePingErPacketLoss(false),
useAccessLatency(false) { }

void GnpLatencyModel::init(PingErLookup& _pingErLookup, CountryLookup& _countryLookup)
{
    pingErLookup=&_pingErLookup;
    countryLookup=&_countryLookup;
}

double GnpLatencyModel::getMinimumRTT(const GnpNetLayer& sender, const GnpNetLayer& receiver)
{
    string ccSender=sender.getCountryCode();
    string ccReceiver=receiver.getCountryCode();
    double minRtt=0.0;
    if (usePingErInsteadOfGnp)
    {
        minRtt=pingErLookup->getMinimumRtt(ccSender, ccReceiver, *countryLookup);
    } else if (useAnalyticalFunctionInsteadOfGnp)
    {
        double distance=GeoLocationOracle::getGeographicalDistance(sender.getNetID(), receiver.getNetID());
        minRtt=62+(0.02*distance);
    } else
    {
        const NetPosition& senderPos=sender.getNetPosition();
        const NetPosition& receiverPos=receiver.getNetPosition();
        minRtt=senderPos.getDistance(receiverPos);
    }

    if (useAccessLatency)
        minRtt += sender.getAccessLatency() + receiver.getAccessLatency();

//    log.info("Minimum RTT for "+ccSender+" to "+ccReceiver+": "+minRtt+" ms");
    //cerr << "Minimum RTT for "<<ccSender<<" to "<<ccReceiver<<": "<<minRtt<<" ms"<<endl;
    return minRtt;
}

double GnpLatencyModel::getPacketLossProbability(const GnpNetLayer& sender, const GnpNetLayer& receiver)
{
    string ccSender=sender.getCountryCode();
    string ccReceiver=receiver.getCountryCode();
    double twoWayLossRate=0.0;
    double oneWayLossRate=0.0;
    if (usePingErPacketLoss)
    {
        twoWayLossRate=pingErLookup->getPacktLossRate(ccSender, ccReceiver, *countryLookup);
        twoWayLossRate/=100;
        oneWayLossRate=1-sqrt(1-twoWayLossRate);
    }
//    log.debug("Packet Loss Probability for "+ccSender+" to "+ccReceiver+": "+(oneWayLossRate*100)+" %");
    cerr << "Packet Loss Probability for "<<ccSender<<" to "<<ccReceiver<<": "<<(oneWayLossRate*100)<<" %"<<endl;
    return oneWayLossRate;

}

double GnpLatencyModel::getNextJitter(const GnpNetLayer& sender, const GnpNetLayer& receiver)
{
    string ccSender=sender.getCountryCode();
    string ccReceiver=receiver.getCountryCode();
    double randomJitter=0.0;
    if (usePingErJitter)
    {
        LognormalDist distri=pingErLookup->getJitterDistribution(ccSender, ccReceiver, *countryLookup);
//        randomJitter=distri.inverseF(Simulator::nextRandomDouble());
        randomJitter=quantile(distri, Rng::dblrand());
    }
//    log.info("Random Jitter for "+ccSender+" to "+ccReceiver+": "+randomJitter+" ms");
    //cerr << "Random Jitter for "<<ccSender<<" to "<<ccReceiver<<": "<<randomJitter<<" ms"<<endl;
    return randomJitter;

}

double GnpLatencyModel::getAverageJitter(const GnpNetLayer& sender, const GnpNetLayer& receiver)
{
    string ccSender=sender.getCountryCode();
    string ccReceiver=receiver.getCountryCode();
    double jitter=0.0;
    if (usePingErJitter)
    {
        jitter=pingErLookup->getAverageRtt(ccSender, ccReceiver, *countryLookup)-pingErLookup->getMinimumRtt(ccSender, ccReceiver, *countryLookup);
    }
//    log.info("Average Jitter for "+ccSender+" to "+ccReceiver+": "+jitter+" ms");
    cerr << "Average Jitter for "<<ccSender<<" to "<<ccReceiver<<": "<<jitter<<" ms"<<endl;
    return jitter;
}

//double GnpLatencyModel::getUDPerrorProbability(const GnpNetLayer& sender, const GnpNetLayer& receiver, const IPv4Message& msg)
//{
//    if (msg.getPayload().getSize()>65507)
//        throw new IllegalArgumentException("Message-Size ist too big for a UDP-Datagramm (max 65507 byte)");
//    double lp=getPacketLossProbability(sender, receiver);
//    double errorProb=1-Math.pow(1-lp, msg.getNoOfFragments());
//    log.info("Error Probability for a "+msg.getPayload().getSize()+" byte UDP Datagram from "+sender.getCountryCode()+" to "+receiver.getCountryCode()+": "+errorProb*100+" %");
//    return errorProb;
//}

//double GnpLatencyModel::getTcpThroughput(const GnpNetLayer& sender, const GnpNetLayer& receiver)
//{
//    double minRtt=getMinimumRTT(sender, receiver);
//    double averageJitter=getAverageJitter(sender, receiver);
//    double packetLossRate=getPacketLossProbability(sender, receiver);
//    double mathisBW=((MSS*1000)/(minRtt+averageJitter))*Math.sqrt(1.5/packetLossRate);
//    return mathisBW;
//}

long GnpLatencyModel::getTransmissionDelay(double bytes, double bandwidth)
{
    double messageTime=bytes/bandwidth;
    long delay=bytes == 0 ? 0 : bandwidth == 0 ? LONG_MAX : roundl((messageTime*Simulator::SECOND_UNIT));
//    log.info("Transmission Delay (s): "+messageTime+" ( "+bytes+" bytes  /  "+bandwidth+" bytes/s )");
    cerr << "Transmission Delay (s): "<<messageTime<<" ( "<<bytes<<" bytes  /  "<<bandwidth<<" bytes/s )"<<endl;
    return delay;
}

long GnpLatencyModel::getPropagationDelay(const GnpNetLayer& sender, const GnpNetLayer& receiver)
{
    double minRtt=getMinimumRTT(sender, receiver);
    double randomJitter=getNextJitter(sender, receiver);
    double receiveTime=(minRtt+randomJitter)/2.0;
    long latency=round(receiveTime*Simulator::MILLISECOND_UNIT);
//    log.info("Propagation Delay for "+sender.getCountryCode()+" to "+receiver.getCountryCode()+": "+receiveTime+" ms");
    //cerr << "Propagation Delay for "<<sender.getCountryCode()<<" to "<<receiver.getCountryCode()<<": "<<receiveTime<<" ms"<<endl;
    return latency;
}

long GnpLatencyModel::getLatency(const NetLayer& sender, const NetLayer& receiver)
{
    return getPropagationDelay((const GnpNetLayer&)sender, (const GnpNetLayer&)receiver);
}
