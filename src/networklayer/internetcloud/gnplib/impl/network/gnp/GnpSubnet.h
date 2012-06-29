#ifndef __GNPLIB_IMPL_NETWORK_GNP_GNPSUBNET_H
#define __GNPLIB_IMPL_NETWORK_GNP_GNPSUBNET_H

#include <map>
#include <boost/scoped_ptr.hpp>
#include <gnplib/impl/network/AbstractSubnet.h>
#include <gnplib/impl/network/IPv4NetID.h>

namespace gnplib { namespace impl { namespace network { namespace gnp {

class GnpNetLayer;
class GnpLatencyModel;

/**
 * 
 * @author Gerald Klunker
 * @author ported to C++ by Philipp Berndt <philipp.berndt@tu-berlin.de>
 * @version 0.1, 17.01.2008
 * 
 */
class GnpSubnet : public AbstractSubnet // , public SimulationEventHandler
{
    // static Logger log;

    // const static Byte REALLOCATE_BANDWIDTH_PERIODICAL_EVENT;
    // const static Byte REALLOCATE_BANDWIDTH_EVENTBASED_EVENT;

    // AbstractGnpNetBandwidthManager bandwidthManager;

    // long pbaPeriod;
    // long nextPbaTime;
    // long nextResheduleTime;

    // std::map<GnpNetBandwidthAllocation, std::set<TransferProgress> > currentlyTransferedStreams;
    // std::map<int, TransferProgress> currentStreams;

    // int lastCommId;

    //public:
    // static std::set<TransferProgress> obsoleteEvents;

    //private:
    boost::scoped_ptr<GnpLatencyModel> netLatencyModel;

    typedef std::map<IPv4NetID, GnpNetLayer*> layers_t;
    layers_t layers;

    // GeoLocationOracle oracle;

public:
    GnpSubnet();

    inline const GnpNetLayer* getNetLayer(const api::network::NetID& netId) const
    {
        const IPv4NetID&ipv4NetId(dynamic_cast<const IPv4NetID&>(netId));
        layers_t::const_iterator layer=layers.find(ipv4NetId);
        return layer!=layers.end() ? layer->second : 0;
    }

    void setLatencyModel(GnpLatencyModel* _netLatencyModel);

    inline const GnpLatencyModel& getLatencyModel() const
    {
        return *netLatencyModel;
    }

    // inline void setBandwidthManager(const AbstractGnpNetBandwidthManager& bm)
    // {
    //     this.bandwidthManager=bm;
    // }

    // inline void setPbaPeriod(long timeUnits)
    // {
    //     this.pbaPeriod=timeUnits;
    // }

    /**
     * Registers a NetWrapper in the SubNet.
     *
     * @param wrapper
     *            The NetWrapper.
     */
    // 	@Override
    void registerNetLayer(api::network::NetLayer& netLayer);

    /**
     *
     */
    // @Override
    // void send(const NetMessage& msg);

    /**
     *
     * @param msg
     * @param sender
     * @param receiver
     */
    //private:
    // void sendMessage(const IPv4Message& msg, const GnpNetLayer& sender, const GnpNetLayer& receiver);

    /**
     *
     * @param netLayer
     */
    //public:
    // void goOffline(const NetLayer& netLayer);

    /**
     *
     * @param msg
     */
    // void cancelTransmission(int commId);

    /**
     * Processes a SimulationEvent. (Is called by the Scheduler.)
     *
     * @param se
     *            SimulationEvent that is returned by the Scheduler.
     */
    // void eventOccurred(const SimulationEvent& se);

    /**
     * ToDo
     *
     * @param ba
     */
    //private:
    // void reschedulePeriodical(const GnpNetBandwidthAllocation& ba);

    /**
     * ToDo
     *
     * @param ba
     */
    // void rescheduleEventBased(const GnpNetBandwidthAllocation& ba);

};

} } } } // namespace gnplib::impl::network::gnp

#endif // not defined __GNPLIB_IMPL_NETWORK_GNP_GNPSUBNET_H
