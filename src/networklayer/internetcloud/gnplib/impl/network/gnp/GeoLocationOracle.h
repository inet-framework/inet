#ifndef __GNPLIB_IMPL_NETWORK_GNP_GEOLOCATIONORACLE_H
#define __GNPLIB_IMPL_NETWORK_GNP_GEOLOCATIONORACLE_H

namespace gnplib { namespace api { namespace network {

class NetID;

} }

namespace impl { namespace network { namespace gnp {

class GnpSubnet;
class GnpLatencyModel;

namespace GeoLocationOracle
{
extern GnpSubnet* subnet;
extern GnpLatencyModel* lm;

/**
 * This method determines the priority level of the specified remote host by
 * considering the geographical(-regional) underlay awareness between this
 * host and the specified local host.
 *
 * The priority level is calculated as follows. Both hosts are located in the same:
 * 	- city => Priority 4
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
int getGeoPriority(const api::network::NetID& local, const api::network::NetID& remote);


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
double getInternetPropagationDelay(const api::network::NetID& local, const api::network::NetID& remote);


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
double getGeographicalDistance(const api::network::NetID& local, const api::network::NetID& remote);

};

} } } } // namespace gnplib::impl::network::gnp

#endif // not defined __GNPLIB_IMPL_NETWORK_GNP_GEOLOCATIONORACLE_H
