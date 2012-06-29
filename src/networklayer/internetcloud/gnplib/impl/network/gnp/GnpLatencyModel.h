#ifndef __GNPLIB_IMPL_NETWORK_GNP_GNPLATENCYMODEL_H
#define __GNPLIB_IMPL_NETWORK_GNP_GNPLATENCYMODEL_H

#include <gnplib/api/network/NetLatencyModel.h>

namespace gnplib { namespace impl { namespace network { namespace gnp { namespace topology {
class PingErLookup;
class CountryLookup;
}

class GnpNetLayer;

class GnpLatencyModel : public api::network::NetLatencyModel
{
public:
    // static const int MSS=IPv4Message.MTU_SIZE-IPv4Message.HEADER_SIZE-TCPMessage.HEADER_SIZE;
private:
    static topology::PingErLookup* pingErLookup;
    static topology::CountryLookup* countryLookup;

    bool usePingErInsteadOfGnp;
    bool useAnalyticalFunctionInsteadOfGnp;
    bool usePingErJitter;
    bool usePingErPacketLoss;
    bool useAccessLatency;

public:
    GnpLatencyModel();
    void init(topology::PingErLookup& pingErLookup, topology::CountryLookup& countryLookup);

private:
    double getMinimumRTT(const GnpNetLayer& sender, const GnpNetLayer& receiver);
    double getPacketLossProbability(const GnpNetLayer& sender, const GnpNetLayer& receiver);
    double getNextJitter(const GnpNetLayer& sender, const GnpNetLayer& receiver);
    double getAverageJitter(const GnpNetLayer& sender, const GnpNetLayer& receiver);

public:
    // double getUDPerrorProbability(const GnpNetLayer& sender, const GnpNetLayer& receiver, const IPv4Message& msg);
    double getTcpThroughput(const GnpNetLayer& sender, const GnpNetLayer& receiver);
    long getTransmissionDelay(double bytes, double bandwidth);
    long getPropagationDelay(const GnpNetLayer& sender, const GnpNetLayer& receiver);
    long getLatency(const api::network::NetLayer& sender, const api::network::NetLayer& receiver);

    inline void setUsePingErRttData(bool pingErRtt)
    {
        usePingErInsteadOfGnp=pingErRtt;
    }

    inline void setUseAnalyticalRtt(bool analyticalRtt)
    {
        useAnalyticalFunctionInsteadOfGnp=analyticalRtt;
    }

    inline void setUsePingErJitter(bool pingErRtt)
    {
        usePingErJitter=pingErRtt;
    }

    inline void setUsePingErPacketLoss(bool pingErPacketLoss)
    {
        usePingErPacketLoss=pingErPacketLoss;
    }

    /**
     * Whether result of getMinimumRTT() should include access network latency.
     * Defaults to false.
     */
    inline void setUseAccessLatency(bool accessLatency)
    {
        useAccessLatency=accessLatency;
    }

};

} } } } // namespace gnplib::impl::network::gnp

#endif // not defined __GNPLIB_IMPL_NETWORK_GNP_GNPLATENCYMODEL_H
