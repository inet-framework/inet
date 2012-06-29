#ifndef __GNPLIB_IMPL_NETWORK_GNP_GNPNETLAYERFACTORY_H
#define __GNPLIB_IMPL_NETWORK_GNP_GNPNETLAYERFACTORY_H

#include <cmath>
#include <map>
#include <vector>
#include <boost/scoped_ptr.hpp>
#include <gnplib/api/common/ComponentFactory.h>
#include <gnplib/impl/network/IPv4NetID.h>
#include <gnplib/impl/network/gnp/GeoLocation.h>
#include <gnplib/impl/network/gnp/GnpNetLayer.h>
#include <gnplib/impl/network/gnp/GnpSubnet.h>
#include <gnplib/impl/network/gnp/topology/GnpPosition.h>

namespace gnplib { namespace impl { namespace network { namespace gnp { namespace topology {
    class PingErLookup;
    class CountryLookup;
}

class GnpLatencyModel;

class GnpNetLayerFactory : public api::common::ComponentFactory
{
public:

    struct GnpHostInfo
    {
        topology::GnpPosition gnpPosition;
        GeoLocation geoLoc;
        double maxUpBandwidth; //  in bytes/s !
        double maxDownBandwidth; //  in bytes/s !
        double accessLatency; // in ms

        inline GnpHostInfo(const GeoLocation& _geoLoc, const topology::GnpPosition&gnpPos)
        : gnpPosition(gnpPos),
        geoLoc(_geoLoc),
        maxUpBandwidth(NAN),
        maxDownBandwidth(NAN),
        accessLatency(NAN){ }
    };

    typedef std::map<IPv4NetID, GnpHostInfo> hostPool_t;
    typedef std::map<std::string, std::vector<IPv4NetID> > namedGroups_t;
private:

    //static Logger log;
    const static double DEFAULT_DOWN_BANDWIDTH;
    const static double DEFAULT_UP_BANDWIDTH;
    const static double DEFAULT_ACCESS_LATENCY;

    GnpSubnet subnet;

    double downBandwidth;
    double upBandwidth;
    double accessLatency;
    hostPool_t hostPool;
    namedGroups_t namedGroups;
    boost::scoped_ptr<topology::PingErLookup> pingErLookup;
    boost::scoped_ptr<topology::CountryLookup> countryLookup;

public:
    GnpNetLayerFactory();
    ~GnpNetLayerFactory();
    GnpNetLayer* createComponent(api::common::Host* host);


    /**
     * random node form group
     *
     * @param id
     * @return
     */
    GnpNetLayer* newNetLayer(const std::string& id);

private:
    GnpNetLayer* newNetLayer(const IPv4NetID& netID);

public:
    void setGnpFile(const std::string& gnpFileName);

    inline void setDownBandwidth(double _downBandwidth)
    {
        downBandwidth=_downBandwidth;
    }

    inline void setUpBandwidth(double _upBandwidth)
    {
        upBandwidth=_upBandwidth;
    }

    inline void setAccessLatency(double _accessLatency)
    {
        accessLatency=_accessLatency;
    }

    void setLatencyModel(GnpLatencyModel* model);

    // inline void setBandwidthManager(const AbstractGnpNetBandwidthManager& bm)
    // {
    //     subnet.setBandwidthManager(bm);
    // }

    // inline void setPbaPeriod(const double& seconds)
    // {
    //     subnet.setPbaPeriod(Math.round(seconds*Simulator.SECOND_UNIT));
    // }
};

} } } } // namespace gnplib::impl::network::gnp

#endif // not defined __GNPLIB_IMPL_NETWORK_GNP_GNPNETLAYERFACTORY_H
