#ifndef __GNPLIB_IMPL_NETWORK_GNP_GEOLOCATION_H
#define __GNPLIB_IMPL_NETWORK_GNP_GEOLOCATION_H

#include <string>

namespace gnplib { namespace impl { namespace network { namespace gnp {

class GeoLocation
{
    std::string countryCode;
    std::string region;
    std::string city;
    std::string isp;
    std::string continentalArea;
    double latitude;
    double longitude;

public:

    inline GeoLocation(const std::string& _conArea, const std::string& _countryCode, const std::string& _region, const std::string& _city, const std::string& _isp, double _latitude, double _longitude)
    : countryCode(_countryCode),
    region(_region),
    city(_city),
    isp(_isp),
    continentalArea(_conArea),
    latitude(_latitude),
    longitude(_longitude) { }

    inline const std::string& getContinentalArea() const
    {
        return continentalArea;
    }

    inline const std::string& getCountryCode() const
    {
        return countryCode;
    }

    inline const std::string& getRegion() const
    {
        return region;
    }

    inline const std::string& getCity() const
    {
        return city;
    }

    inline const std::string& getIsp() const
    {
        return isp;
    }

    inline double getLatitude() const
    {
        return latitude;
    }

    inline double getLongitude() const
    {
        return longitude;
    }
};

} } } } // namespace gnplib::impl::network::gnp

#endif // not defined __GNPLIB_IMPL_NETWORK_GNP_GEOLOCATION_H
