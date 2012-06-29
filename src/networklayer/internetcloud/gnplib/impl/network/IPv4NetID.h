#ifndef __GNPLIB_IMPL_NETWORK_IPV4NETID_H
#define __GNPLIB_IMPL_NETWORK_IPV4NETID_H

#include <gnplib/api/network/NetID.h>
#include <boost/operators.hpp>
#include <stdint.h>

namespace gnplib { namespace impl { namespace network {

/**
 * Implementation of the NetID-Interface for IPv4-Addresses
 * 
 * @author Gerald Klunker
 * @author ported to C++ by Philipp Berndt <philipp.berndt@tu-berlin.de>
 */
class IPv4NetID : public api::network::NetID, boost::totally_ordered<IPv4NetID>
{
    /**
     * 32bit IP Address.
     */
    uint32_t id;

    /**
     * Creates an Instance of IPv4NetID.
     *
     * @param id
     *            The Long-ID.
     */
public:

    inline IPv4NetID(uint32_t _id)
    : id(_id) { }

    IPv4NetID(const std::string& id);

    /**
     * @return The Long-ID.
     */
    inline uint32_t getID() const
    {
        return id;
    }

    /**
     * @param other
     *            An object.
     * @return Whether the parameter is equal to this IPv4NetID or not.
     */
    inline bool operator==(const IPv4NetID& other) const
    {
        return id==other.id;
    }

    inline bool operator<(const IPv4NetID& other) const
    {
        return id<other.id;
    }

    /**
     * @return The hashcode of this IPv4NetID.
     */
    // 	@Override

    //    inline int hashCode()
    //    {
    //        return this.id.hashCode();
    //    }


    /**
     * Prints a string representing this IPv4NetID.
     */
    // @Override
    void print(std::ostream& os) const;


    // ToDo Exceptions bei ungueltigen Parameterwerten schmeissen

    /**
     * @param ip
     *            32bit IP-Address
     * @return A readable IP-String like "192.168.0.1"
     */
    // static std::string ipToString(uint64_t ip);


    /**
     *
     * @param ip
     *            readable IP-String like "192.168.0.1"
     * @return A 32bit IP-Address
     */
    static uint32_t ipToLong(const std::string& ip);

};

} } } // namespace gnplib::impl::network

#endif // not defined __GNPLIB_IMPL_NETWORK_IPV4NETID_H
