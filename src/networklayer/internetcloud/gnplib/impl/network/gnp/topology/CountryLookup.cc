#include <gnplib/impl/network/gnp/topology/CountryLookup.h>
#include <gnplib/impl/network/gnp/topology/PingErLookup.h>

using std::string;
using std::map;
using std::set;
using std::vector;

using gnplib::impl::network::gnp::topology::CountryLookup;
using gnplib::impl::network::gnp::topology::PingErLookup;

// const long CountryLookup::serialVersionUID(-3994762133062677848L);
// Logger CountryLookup::log(SimLogger.getLogger(CountryLookup.class));

//countryLookup(new HashMap<String, String[]>()),
//pingErCountryRegions(new HashMap<String, String>()),
//pingErCountryRegionFilename(),
//pingErCountry(new ArrayList<String>()),
//pingErRegion(new ArrayList<String>()),

/**
 *
 * @param 2-digits
 *            country code
 * @return dedicated GeoIP country name
 */
//const string& CountryLookup::getGeoIpCountryName(const string& countryCode) const
//{
//    if (countryLookup.containsKey(countryCode))
//        return countryLookup.get(countryCode)[0];
//    else
//        return null;
//}

/**
 *
 * @param 2-digits
 *            country code
 * @return dedicated PingEr country name
 */
const char* CountryLookup::getPingErCountryName(const string& countryCode) const
{
    countryLookup_t::const_iterator it(countryLookup.find(countryCode));
    return (it != countryLookup.end()) ? it->second[1].c_str() : 0;
}

/**
 *
 * @param country
 *            2-digits country code or pingEr country name
 * @return dedicated PingEr region name
 */
const char* CountryLookup::getPingErRegionName(const string& country) const
{
    countryLookup_t::const_iterator it(countryLookup.find(country));
    if (it != countryLookup.end())
        return it->second[2].c_str();

    std::map< std::string, std::string >::const_iterator it2(pingErCountryRegions.find(country));
    return (it2 != pingErCountryRegions.end()) ? it2->second.c_str() : 0;
}

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
void CountryLookup::addCountryFromGeoIP(const string& countryCode, const string& country)
{
    if (countryLookup.find(countryCode) == countryLookup.end())
    {
        vector<string>& names(countryLookup[countryCode]);
        names.push_back(country);
        map<string, string>::const_iterator region(pingErCountryRegions.find(country));
        if (region != pingErCountryRegions.end())
        {
            names.push_back(country);
            names.push_back(region->second);
        }
        else {
            names.push_back(string());
            names.push_back(string());
        }
    }
}

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
void CountryLookup::assignCountryCodeToPingErData(const string& code, const string& country, const string& region)
{
    countryLookup_t::iterator it(countryLookup.find(code));
    if (it != countryLookup.end())
    {
        vector<string>& names(it->second);
        names[1]=country;
        names[2]=region;
    }
}

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
//void CountryLookup::importPingErCountryRegionFile(const File& file)
//{
//    try
//    {
//        log.debug("Importing PingER-Country-Region-File: "+file.getAbsolutePath());
//        FileReader inputFilePingER=new FileReader(file);
//        BufferedReader inputPingER=new BufferedReader(inputFilePingER);
//        HashMap<String, String> countryToRegion=new HashMap<String, String>();
//        String line=inputPingER.readLine();
//        while (line!=null)
//        {
//            String[] parts=line.split(",");
//            countryToRegion.put(parts[0], parts[1]);
//            line=inputPingER.readLine();
//        }
//        inputPingER.close();
//        inputFilePingER.close();
//
//        Set<String> codes=countryLookup.keySet();
//        for (String cc : codes)
//        {
//            String geoIpCountry=getGeoIpCountryName(cc);
//            if (geoIpCountry!=null&&countryToRegion.containsKey(geoIpCountry))
//            {
//                assignCountryCodeToPingErData(cc, geoIpCountry, countryToRegion.get(geoIpCountry));
//            }
//        }
//        pingErCountryRegions.putAll(countryToRegion);
//        this.pingErCountryRegionFilename=file.getAbsolutePath();
//
//    } catch (IOException e)
//    {
//        e.printStackTrace();
//    }
//}

/**
 * Import lockup data from an xml-element.
 *
 * @param element
 */
//void CountryLookup::importFromXML(const Element& element)
//{
//    Iterator<Element> iter=element.elementIterator("CountryKey");
//    while (iter.hasNext())
//    {
//        Element variable=iter.next();
//        String code=variable.attributeValue("code");
//        String[] names=new String[3];
//        names[0]=variable.attributeValue("countryGeoIP");
//        names[1]=variable.attributeValue("countryPingEr");
//        names[2]=variable.attributeValue("regionPingEr");
//        countryLookup.put(code, names);
//        pingErCountryRegions.put(names[1], names[2]);
//    }
//}

/**
 * Import lockup data from an xml-file.
 *
 * @param file
 */
//void CountryLookup::importFromXML(const File& file)
//{
//    try
//    {
//        SAXReader reader=new SAXReader(false);
//        Document configuration=reader.read(file);
//        Element root=configuration.getRootElement();
//        importFromXML(root);
//    } catch (DocumentException e)
//    {
//        // TODO Auto-generated catch block
//        e.printStackTrace();
//    }
//}

/**
 *
 * @return xml-element containing current lookup data
 */
//const Element& CountryLookup::exportToXML()
//{
//    DefaultElement country=new DefaultElement("CountryLookup");
//    Set<String> codeKeys=countryLookup.keySet();
//    for (String code : codeKeys)
//    {
//        DefaultElement countryXml=new DefaultElement("CountryKey");
//        countryXml.addAttribute("code", code);
//        countryXml.addAttribute("countryGeoIP", getGeoIpCountryName(code));
//        countryXml.addAttribute("countryPingEr", getPingErCountryName(code));
//        countryXml.addAttribute("regionPingEr", getPingErRegionName(code));
//        country.add(countryXml);
//    }
//    return country;
//}

/**
 *
 * @return set of all available country codes added
 */
//const set<string>& CountryLookup::getCountryCodes()
//{
//    return countryLookup.keySet();
//}

/**
 *
 * @return sorted list of all available PingEr country names
 */
//const vector<string>& CountryLookup::getPingErCountrys()
//{
//    if (pingErCountry.size()<=1)
//    {
//        pingErCountry.clear();
//        pingErCountry.addAll(pingErCountryRegions.keySet());
//        pingErCountry.add("");
//        Collections.sort(pingErCountry);
//    }
//    return pingErCountry;
//}

/**
 *
 * @return sorted list of all available PingEr region names
 */
//const vector<string>& CountryLookup::getPingErRegions()
//{
//    if (pingErRegion.size()<=1)
//    {
//        pingErRegion.clear();
//        Set<String> region=new HashSet<String>();
//        region.addAll(pingErCountryRegions.values());
//        pingErRegion.addAll(region);
//        pingErRegion.add("");
//        Collections.sort(pingErRegion);
//    }
//    return pingErRegion;
//}

/**
 * Remove all countries that are not defined in the Set
 *
 * @param countryCodes
 *            Set of country codes
 */
//void CountryLookup::keepCountries(const set<string>& countryCodes)
//{
//    countryLookup.keySet().retainAll(countryCodes);
//}

/**
 *
 * @param name
 *            of country or region
 * @return true, if there is a country or region with name exist
 */
//bool CountryLookup::containsPingErCountryOrRegion(const string& name) const
//{
//    for (countryLookup_t::const_iterator i=countryLookup.begin(); i!=countryLookup.end(); ++i) {
//        const vector<string>& names(i->second);
//        if (names[1] == name || names[2] == name)
//            return true;
//    }
//    return false;
//}
