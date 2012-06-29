#ifndef __GNPLIB_IMPL_NETWORK_GNP_TOPOLOGY_COUNTRYLOOKUP_H
#define __GNPLIB_IMPL_NETWORK_GNP_TOPOLOGY_COUNTRYLOOKUP_H

#include <set>
#include <map>
#include <vector>
#include <string>

namespace gnplib { namespace impl { namespace network { namespace gnp { namespace topology {

class CountryLookupParser;

/**
 * This class manages the mapping of the 2-digits country code to country and
 * region used in the aggregated PingEr reports.
 * 
 * @author Gerald Klunker
 * @author ported to C++ by Philipp Berndt <philipp.berndt@tu-berlin.de>
 * @version 0.1, 09.01.2008
 * 
 */
class CountryLookup // : public Serializable
{
    //friend class CountryLookupParser;
    friend class CountryLookupParser;
    //const static long serialVersionUID;

    typedef std::map< std::string, std::vector<std::string> > countryLookup_t; // countryCode => [ countryGeoIP, countryPingER, regionPingEr ]
    countryLookup_t countryLookup;
    std::map< std::string, std::string > pingErCountryRegions;
    std::string pingErCountryRegionFilename;

    // sorted list holding PingEr Data for using in a graphical frontend only
    std::vector<std::string> pingErCountry;
    std::vector<std::string> pingErRegion;

    /**
     *
     * @param 2-digits
     *            country code
     * @return dedicated GeoIP country name
     */
public:
    // const std::string& getGeoIpCountryName(const std::string& countryCode) const;

    /**
     *
     * @param 2-digits
     *            country code
     * @return dedicated PingEr country name
     */
    const char* getPingErCountryName(const std::string& countryCode) const;

    /**
     *
     * @param country
     *            2-digits country code or pingEr country name
     * @return dedicated PingEr region name
     */
    const char* getPingErRegionName(const std::string& country) const;

    /**
     * Adds GeoIP country code and country name.
     *
     * It will be assign automatically to PingEr Countries, if there are obvious
     * consenses.
     *
     * @param 2-digits
     *            country code
     * @param dedicated
     *            country name from GeoIP
     */
    void addCountryFromGeoIP(const std::string& countryCode, const std::string& country);

    /**
     * Assign a country code (from GeoIP) to PingER country and/or region name.
     * Attention: Nothing happens, if country code was not added before
     *
     * @param 2-digits
     *            country code
     * @param dedicated
     *            country name from PingER
     * @param dedicated
     *            region name from PingER
     */
    void assignCountryCodeToPingErData(const std::string& code, const std::string& country, const std::string& region);

    /**
     * Import all country and region names, that are used by the PingER Project.
     * The Country - Region Mapping File can be downloaded form the website of
     * the project.
     * (http://www-iepm.slac.stanford.edu/pinger/region_country.txt)
     *
     * GeoIP countries will be assign automatically to PingEr Countries, if
     * there are obviouse consensuses.
     *
     * @param file
     */
    // void importPingErCountryRegionFile(const std::string& file);

    /**
     * Import lockup data from an xml-element.
     *
     * @param element
     */
    // void importFromXML(const Element& element);

    /**
     * Import lockup data from an xml-file.
     *
     * @param file
     */
    // void importFromXML(const std::string& file);

    /**
     *
     * @return xml-element containing current lookup data
     */
    // const Element& exportToXML();

    /**
     *
     * @return set of all available country codes added
     */
    // const std::set<std::string>& getCountryCodes();

    /**
     *
     * @return sorted list of all available PingEr country names
     */
    // const std::vector<std::string>& getPingErCountrys();


    /**
     *
     * @return sorted list of all available PingEr region names
     */
    // const std::vector<std::string>& getPingErRegions();

    /**
     * Remove all countries that are not defined in the Set
     *
     * @param countryCodes
     *            Set of country codes
     */
    // void keepCountries(const std::set<std::string>& countryCodes);

    /**
     *
     * @param name
     *            of country or region
     * @return true, if there is a country or region with name exist
     */
    // bool containsPingErCountryOrRegion(const std::string& name) const;

    /**
     *
     * @return PingER Country - Region Mapping Filename (region_country.txt)
     *         used for naming
     */
    //    inline const std::string& getPingErCountryRegionFilename() const
    //    {
    //        return pingErCountryRegionFilename;
    //    }

};

} } } } } // namespace gnplib::impl::network::gnp::topology

#endif // not defined __GNPLIB_IMPL_NETWORK_GNP_TOPOLOGY_COUNTRYLOOKUP_H
