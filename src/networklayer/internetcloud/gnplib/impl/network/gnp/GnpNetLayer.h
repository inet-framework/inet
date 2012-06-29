#ifndef __GNPLIB_IMPL_NETWORK_GNP_GNPNETLAYER_H
#define __GNPLIB_IMPL_NETWORK_GNP_GNPNETLAYER_H

#include <gnplib/impl/network/AbstractNetLayer.h>
#include <gnplib/impl/network/IPv4NetID.h>
#include <gnplib/impl/network/gnp/GeoLocation.h>

namespace gnplib { namespace impl { namespace network { namespace gnp {
namespace topology {
    class GnpPosition;
}
class GnpSubnet;

/**
 * 
 * @author geraldklunker
 * @author Philipp Berndt <philipp.berndt@tu-berlin.de>
 * 
 */
class GnpNetLayer : public AbstractNetLayer //, public SimulationEventHandler
{
    GeoLocation geoLocation;
    GnpSubnet& subnet;
    //    long nextFreeSendingTime;
    //    long nextFreeReceiveTime;
    //    map<GnpNetLayer, GnpNetBandwidthAllocation> connections;

public:
    GnpNetLayer(GnpSubnet& subNet, const IPv4NetID& netID, const topology::GnpPosition& netPosition, const GeoLocation& geoLoc, double downBandwidth, double upBandwidth, double wireAccessLatency);

    inline GeoLocation getGeoLocation() const
    {
        return geoLocation;
    }

    /**
     *
     * @return 2-digit country code
     */
    inline const std::string& getCountryCode() const
{
    return geoLocation.getCountryCode();
}


    // @Override

    inline const IPv4NetID& getNetID() const
    {
        return dynamic_cast<const IPv4NetID&>(AbstractNetLayer::getNetID());
    }

    /**
     *
     * @return first time sending is possible (line is free)
     */
    // long getNextFreeSendingTime();

    /**
     *
     * @param time
     *            first time sending is possible (line is free)
     */
    // void setNextFreeSendingTime(long time);

    /**
     *
     * @param netLayer
     * @return
     */
    // bool isConnected(GnpNetLayer netLayer);

    /**
     *
     * @param netLayer
     * @param allocation
     */
    // void addConnection(GnpNetLayer netLayer, GnpNetBandwidthAllocation allocation);

    /**
     *
     * @param netLayer
     * @return
     */
    // GnpNetBandwidthAllocation getConnection(GnpNetLayer netLayer);

    /**
     *
     * @param netLayer
     */
    // void removeConnection(GnpNetLayer netLayer);

    /**
     *
     * @param msg
     */
    // void addToReceiveQueue(IPv4Message msg);

    // @Override
    // bool isSupported(TransProtocol transProtocol);

    // void send(Message msg, NetID receiver, NetProtocol netProtocol);

    // @Override
    // std::string toString() const;


    /*
     * (non-Javadoc)
     *
     * @see de.tud.kom.p2psim.api.simengine.SimulationEventHandler#eventOccurred(de.tud.kom.p2psim.api.simengine.SimulationEvent)
     */
    // void eventOccurred(SimulationEvent se);

    // void goOffline();

    // void cancelTransmission(int commId);

    // for JUnit Test
    // void goOffline(long time);

    // void cancelTransmission(int commId, long time);

    // void send(Message msg, NetID receiver, NetProtocol netProtocol, long sendTime);
};

} } } } // namespace gnplib::impl::network::gnp

#endif // not defined __GNPLIB_IMPL_NETWORK_GNP_GNPNETLAYER_H
