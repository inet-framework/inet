#include <gnplib/impl/network/gnp/GeoLocationOracle.h>

#include <cmath>
#include <gnplib/api/Simulator.h>
#include <gnplib/api/network/NetID.h>
#include <gnplib/impl/network/gnp/GeoLocation.h>
#include <gnplib/impl/network/gnp/GnpSubnet.h>
#include <gnplib/impl/network/gnp/GnpNetLayer.h>
#include <gnplib/impl/network/gnp/HaversineHelpers.h>
#include <gnplib/impl/network/gnp/GnpLatencyModel.h>

namespace Simulator=gnplib::api::Simulator;
namespace GeoLocationOracle=gnplib::impl::network::gnp::GeoLocationOracle;
using gnplib::impl::network::gnp::GnpLatencyModel;
using gnplib::impl::network::gnp::GnpSubnet;
using gnplib::api::network::NetID;

GnpSubnet* GeoLocationOracle::subnet(0);
GnpLatencyModel* GeoLocationOracle::lm(0);

/**
 * This method determines the priority level of the specified remote host by
 * considering the geographical(-regional) underlay awareness between this
 * host and the specified local host.
 *
 * The priority level is calculated as follows. Both hosts are located in the same:
 *  - city => Priority 4
 *  - region => Priority 3
 *  - country => Priority 2
 *  - "continental" region => Priority 1
 *  - world/untraceable => Priority 0
 *
 * @param local
 *            IP-address of the local host
 * @param remote
 *            IP-address of the remote host
 * @return the priority level
 */

int GeoLocationOracle::getGeoPriority(const NetID& local, const NetID& remote)
{
    assert (subnet);
    const GnpNetLayer* const localNetLayer(subnet->getNetLayer(local));
    const GnpNetLayer* const remoteNetLayer(subnet->getNetLayer(remote));
    assert (localNetLayer);
    assert (remoteNetLayer);
    const GeoLocation localGeo(localNetLayer->getGeoLocation());
    const GeoLocation remoteGeo(remoteNetLayer->getGeoLocation());
    if (remoteGeo.getCity()!="--" && localGeo.getCity()==remoteGeo.getCity())
        return 4;
    else if (remoteGeo.getRegion()!="--" && localGeo.getRegion()==remoteGeo.getRegion())
        return 3;
    else if (remoteGeo.getCountryCode()!="--" && localGeo.getCountryCode()==remoteGeo.getCountryCode())
        return 2;
    else if (remoteGeo.getContinentalArea()!="--" && localGeo.getContinentalArea()==remoteGeo.getContinentalArea())
        return 1;
    else
        return 0;
}


/**
 * Normally, the propagation of messages through channels and routers of the
 * Internet is affected by the propagation delays (of the physical media),
 * and the processing-, queuing-, and transmission delays of the routers.
 * The so called Internet propagation delay is modeled as the sum of a fixed
 * part that combines the aforementioned router and propagation delays, and
 * a variable part to reproduce the jitter.
 *
 * Invoking this method returns the Internet propagation delay (in ms) between two
 * hosts in the Internet. Note that this delay is derived from measurement
 * data, and it therefore estimates the one-way delay of the measured
 * round-trip-times between the specified hosts.
 *
 * @param local
 *            IP-address of the local host
 * @param remote
 *            IP-address of the remote host
 * @return
 *       the Internet propagation delay in ms
 */
double GeoLocationOracle::getInternetPropagationDelay(const NetID& local, const NetID& remote)
{
    assert (subnet);
    const GnpNetLayer* const localNet(subnet->getNetLayer(local));
    const GnpNetLayer* const remoteNet(subnet->getNetLayer(remote));
    assert (localNet);
    assert (remoteNet);
    return lm->getPropagationDelay(*localNet, *remoteNet)/(double)Simulator::MILLISECOND_UNIT;
}

/**
 * Calculates the distance in kilometers (km) from one host to another,
 * using the Haversine formula. The squashed shape of the earth into account
 * (approximately)
 *
 * @param local
 *            IP-address of the local host
 * @param remote
 *            IP-address of the remote host
 * @return the distance between the specified hosts in km
 *
 */
double GeoLocationOracle::getGeographicalDistance(const NetID& local, const NetID& remote)
{
    assert (subnet);
    const GnpNetLayer* const localNet(subnet->getNetLayer(local));
    const GnpNetLayer* const remoteNet(subnet->getNetLayer(remote));
    assert (localNet);
    assert (remoteNet);
    const GeoLocation localGeo(localNet->getGeoLocation());
    const GeoLocation remoteGeo(remoteNet->getGeoLocation());

    double lat1=HaversineHelpers::radians(localGeo.getLatitude());
    double lat2=HaversineHelpers::radians(remoteGeo.getLatitude());
    double dlat=lat2-lat1;
    double dlong=HaversineHelpers::radians(remoteGeo.getLongitude())-HaversineHelpers::radians(localGeo.getLongitude());

    double a=HaversineHelpers::square(sin(dlat/2))+cos(lat1)*cos(lat2)*HaversineHelpers::square(sin(dlong/2));

    // angle in radians formed by start point, earth's center, & end point
    double c=2*atan2(sqrt(a), sqrt(1-a));
    // radius of earth at midpoint of route
    double r=HaversineHelpers::globeRadiusOfCurvature((lat1+lat2)/2);
    return (r*c)/1000.0;
}
