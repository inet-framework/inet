#include <gnplib/impl/network/gnp/GnpNetLayerFactory.h>

#include <memory>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>

#include <gnplib/api/Simulator.h>
#include <gnplib/api/common/Host.h>
#include <gnplib/api/common/HostProperties.h>
#include <gnplib/api/common/random/Rng.h>
#include <gnplib/impl/network/gnp/GnpLatencyModel.h>
#include <gnplib/impl/network/gnp/topology/CountryLookup.h>
#include <gnplib/impl/network/gnp/topology/PingErLookup.h>
#include <gnplib/impl/util/xml/Attributes.h>
#include <gnplib/impl/util/xml/PathSAX2Handler.h>

using std::auto_ptr;
using std::string;
using std::vector;
using std::remove;
using std::cerr;
using std::cout;
using std::endl;
using std::make_pair;
using boost::tokenizer;
using boost::char_separator;
using boost::lexical_cast;
using boost::bad_lexical_cast;
using boost::mem_fn;
namespace Simulator=gnplib::api::Simulator;
namespace Rng=gnplib::api::common::random::Rng;
using gnplib::api::common::Host;
using gnplib::impl::network::IPv4NetID;
//using gnplib::impl::network::gnp::GnpNetLayerFactory;
//using gnplib::impl::network::gnp::GeoLocation;
using gnplib::impl::network::gnp::topology::GnpPosition;
using gnplib::impl::network::gnp::topology::CountryLookup;
using gnplib::impl::network::gnp::topology::PingErLookup;
using gnplib::impl::util::xml::EmptyString;
using gnplib::impl::util::xml::MySAX2Handler;
using gnplib::impl::util::xml::Attributes;
using gnplib::impl::util::xml::Attribute;
using XERCES_CPP_NAMESPACE::XMLString;

namespace {
int init_xerces()
{
    // Initialize the XML4C2 system
    try
    {
        XERCES_CPP_NAMESPACE::XMLPlatformUtils::Initialize();
        return 0;
    } catch (const XERCES_CPP_NAMESPACE::XMLException& toCatch)
    {
        cerr<<"Error during initialization! :\n"
                <<toCatch.getMessage()<<endl;
        return -1;
    }
}

// initialize xerces before transformed XMLCh constants:
int xerces_initialized(init_xerces());
}

namespace gnplib { namespace impl { namespace network { namespace gnp {

struct HostsParser : public boost::signals::trackable
{
    static const XMLCh*const IP;
    static const XMLCh*const COORDINATES;
    static const XMLCh*const CONTINENTAL_AREA;
    static const XMLCh*const COUNTRY_CODE;
    static const XMLCh*const REGION;
    static const XMLCh*const CITY;
    static const XMLCh*const ISP;
    static const XMLCh*const LONGITUDE;
    static const XMLCh*const LATITUDE;
    static const XMLCh*const BW_UPSTREAM;
    static const XMLCh*const BW_DOWNSTREAM;
    static const XMLCh*const ACCESS_DELAY;

    GnpNetLayerFactory::hostPool_t hostPool;

    void startElement(const XMLCh*const uri,
                      const XMLCh*const localname,
                      const XMLCh*const qname,
                      const xercesc::Attributes& _attrs)
    {
        try
        {
            Attributes attrs(_attrs);
            // IP-Address
            IPv4NetID hostID(attrs[IP].as<uint32_t>());

            // GNP-Coordinates
            string coordinatesS(attrs[COORDINATES]);
            typedef tokenizer< char_separator<char> > tokenizer;
            tokenizer tokens(coordinatesS, char_separator<char>(","));
            vector<double> coordinatesD;
            for (tokenizer::iterator tok_iter=tokens.begin();
                 tok_iter!=tokens.end(); ++tok_iter)
                coordinatesD.push_back(lexical_cast<double>(*tok_iter));
            GnpPosition gnpPos(coordinatesD);

            // GeoLocation
            EmptyString e;
            string continentalArea(e+attrs[CONTINENTAL_AREA]);
            string countryCode(e+attrs[COUNTRY_CODE]);
            string region(e+attrs[REGION]);
            string city(e+attrs[CITY]);
            string isp(e+attrs[ISP]);
            double longitude(attrs[LONGITUDE].as<double>());
            double latitude(attrs[LATITUDE].as<double>());
            GeoLocation geoLoc(continentalArea, countryCode, region, city, isp, latitude, longitude);

            GnpNetLayerFactory::GnpHostInfo hostInfo(geoLoc, gnpPos);

            hostInfo.maxDownBandwidth = attrs[BW_DOWNSTREAM].as<double>(NAN);
            hostInfo.maxUpBandwidth = attrs[BW_UPSTREAM].as<double>(NAN);
            hostInfo.accessLatency = attrs[ACCESS_DELAY].as<double>(NAN);

            hostPool.insert(make_pair(hostID, hostInfo));
        } catch (bad_lexical_cast& e)
        {
            cerr<<"Error parsing <Host>"<<e.what()<<endl;
        }
    }

};

const XMLCh*const HostsParser::IP(XMLString::transcode("ip"));
const XMLCh*const HostsParser::COORDINATES(XMLString::transcode("coordinates"));
const XMLCh*const HostsParser::CONTINENTAL_AREA(XMLString::transcode("continentalArea"));
const XMLCh*const HostsParser::COUNTRY_CODE(XMLString::transcode("countryCode"));
const XMLCh*const HostsParser::REGION(XMLString::transcode("region"));
const XMLCh*const HostsParser::CITY(XMLString::transcode("city"));
const XMLCh*const HostsParser::ISP(XMLString::transcode("isp"));
const XMLCh*const HostsParser::LONGITUDE(XMLString::transcode("longitude"));
const XMLCh*const HostsParser::LATITUDE(XMLString::transcode("latitude"));
const XMLCh*const HostsParser::BW_UPSTREAM(XMLString::transcode("bw_upstream"));
const XMLCh*const HostsParser::BW_DOWNSTREAM(XMLString::transcode("bw_downstream"));
const XMLCh*const HostsParser::ACCESS_DELAY(XMLString::transcode("accessDelay"));


struct GroupLookupParser : public boost::signals::trackable
{
    static const XMLCh*const ID;
    static const XMLCh*const MAX_SIZE;
    static const XMLCh*const IPS;
    static const XMLCh*const VALUE;

    GnpNetLayerFactory::namedGroups_t namedGroups;
    string currentGroupId;

    void startGroup(const XMLCh*const uri,
                    const XMLCh*const localname,
                    const XMLCh*const qname,
                    const XERCES_CPP_NAMESPACE::Attributes& _attrs)
    {
        Attributes attrs(_attrs);

        try
        {
            currentGroupId=attrs[ID];
            if (namedGroups.find(currentGroupId)!=namedGroups.end())
                cerr<<"Multiple Group Definition in gnpFile ( Group: "<<currentGroupId<<" )"<<endl;

            size_t currentMaxSize=attrs[MAX_SIZE].as<size_t>();
            namedGroups[currentGroupId].reserve(currentMaxSize);
        } catch (bad_lexical_cast& e)
        {
            cerr<<"Error parsing <Host>"<<e.what()<<endl;
        }
    }

    void startIPs(const XMLCh*const uri,
                  const XMLCh*const localname,
                  const XMLCh*const qname,
                  const XERCES_CPP_NAMESPACE::Attributes& _attrs)
    {
        Attributes attrs(_attrs);
        vector<IPv4NetID>& group(namedGroups[currentGroupId]);
        try
        {
            // IP-Addresses
            string value(attrs[VALUE]);
            typedef tokenizer< char_separator<char> > tokenizer;
            tokenizer tokens(value, char_separator<char>(","));
            for (tokenizer::iterator tok_iter=tokens.begin();
                 tok_iter!=tokens.end(); ++tok_iter)
                group.push_back(IPv4NetID(lexical_cast<uint32_t>(*tok_iter)));
        } catch (bad_lexical_cast& e)
        {
            cerr<<"Error parsing <IPs>"<<e.what()<<endl;
        }
    }

};

const XMLCh*const GroupLookupParser::ID(XMLString::transcode("id"));
const XMLCh*const GroupLookupParser::MAX_SIZE(XMLString::transcode("maxsize"));
const XMLCh*const GroupLookupParser::IPS(XMLString::transcode("IPs"));
const XMLCh*const GroupLookupParser::VALUE(XMLString::transcode("value"));

namespace topology {

struct PingERLookupParser : public boost::signals::trackable
{
    static const XMLCh*const FROM;
    static const XMLCh*const TO;
    static const XMLCh*const MIN_RTT;
    static const XMLCh*const AVG_RTT;
    static const XMLCh*const DELAY_VARIATION;
    static const XMLCh*const PACKET_LOSS;

    GnpNetLayerFactory::hostPool_t hostPool;
    PingErLookup& pingErLookup;

    inline PingERLookupParser(PingErLookup&_pingErLookup)
    : pingErLookup(_pingErLookup) { }

    void startSummaryReport(const XMLCh*const uri,
                            const XMLCh*const localname,
                            const XMLCh*const qname,
                            const Attributes& _attrs)
    {
        Attributes attrs(_attrs);

        try
        {
            string regionFrom(attrs[FROM]);
            string regionTo(attrs[TO]);
            double minRtt(attrs[MIN_RTT].as<double>());
            double averageRtt(attrs[AVG_RTT].as<double>());
            double delayVariation(attrs[DELAY_VARIATION].as<double>());
            double packetLoss(attrs[PACKET_LOSS].as<double>());

            pingErLookup.setData(regionFrom, regionTo, PingErLookup::LinkProperty(minRtt, averageRtt, delayVariation, packetLoss));

        } catch (bad_lexical_cast& e)
        {
            cerr<<"Error parsing <SummaryReport>"<<e.what()<<endl;
        }
    }
};

const XMLCh*const PingERLookupParser::FROM(XMLString::transcode("from"));
const XMLCh*const PingERLookupParser::TO(XMLString::transcode("to"));
const XMLCh*const PingERLookupParser::MIN_RTT(XMLString::transcode("minimumRtt"));
const XMLCh*const PingERLookupParser::AVG_RTT(XMLString::transcode("averageRtt"));
const XMLCh*const PingERLookupParser::DELAY_VARIATION(XMLString::transcode("delayVariation"));
const XMLCh*const PingERLookupParser::PACKET_LOSS(XMLString::transcode("packetLoss"));


struct CountryLookupParser : public boost::signals::trackable
{
    static const XMLCh*const CODE;
    static const XMLCh*const COUNTRY_GEO_IP;
    static const XMLCh*const COUNTRY_PINGER;
    static const XMLCh*const REGION_PINGER;

    GnpNetLayerFactory::hostPool_t hostPool;
    CountryLookup& countryLookup;

    inline CountryLookupParser(CountryLookup&_countryLookup)
    : countryLookup(_countryLookup) { }

    void startElement(const XMLCh*const uri,
                      const XMLCh*const localname,
                      const XMLCh*const qname,
                      const Attributes& _attrs)
    {
        EmptyString e;
        Attributes attrs(_attrs);

        try
        {
            string code(attrs[CODE]);
            vector<string> names;

            string countryGeoIP(attrs[COUNTRY_GEO_IP]);
            string countryPinger(e+attrs[COUNTRY_PINGER]);
            string regionPinger(e+attrs[REGION_PINGER]);

            names.push_back(countryGeoIP);
            names.push_back(countryPinger);
            names.push_back(regionPinger);

            countryLookup.countryLookup[code]=names;
            countryLookup.pingErCountryRegions[countryPinger]=regionPinger;
        } catch (bad_lexical_cast& e)
        {
            cerr<<"Error parsing <CountryKey>"<<e.what()<<endl;
        }
    }

};

const XMLCh*const CountryLookupParser::CODE(XMLString::transcode("code"));
const XMLCh*const CountryLookupParser::COUNTRY_GEO_IP(XMLString::transcode("countryGeoIP"));
const XMLCh*const CountryLookupParser::COUNTRY_PINGER(XMLString::transcode("countryPingEr"));
const XMLCh*const CountryLookupParser::REGION_PINGER(XMLString::transcode("regionPingEr"));

} // namespace topology

//static Logger GnpNetLayerFactory::log(SimLogger.getLogger(GnpNetLayerFactory.class));
const double GnpNetLayerFactory::DEFAULT_DOWN_BANDWIDTH(1000);

const double GnpNetLayerFactory::DEFAULT_UP_BANDWIDTH(1000);

const double GnpNetLayerFactory::DEFAULT_ACCESS_LATENCY(50);

GnpNetLayerFactory::GnpNetLayerFactory()
: subnet(),
downBandwidth(DEFAULT_DOWN_BANDWIDTH),
upBandwidth(DEFAULT_UP_BANDWIDTH),
accessLatency(DEFAULT_ACCESS_LATENCY),
hostPool(),
namedGroups(),
pingErLookup(),
countryLookup() { }

GnpNetLayerFactory::~GnpNetLayerFactory() { }

GnpNetLayer* GnpNetLayerFactory::createComponent(Host* host)
{
    auto_ptr<GnpNetLayer> netLayer(newNetLayer(host->getProperties().getGroupID()));
    netLayer->setHost(host);
    return netLayer.release();
}

/**
 * random node from group
 *
 * @param id
 * @return
 */
GnpNetLayer* GnpNetLayerFactory::newNetLayer(const string& id)
{
    namedGroups_t::iterator entry=namedGroups.find(id);
    if (entry!=namedGroups.end()&& !entry->second.empty())
    {
        vector<IPv4NetID>& ids(entry->second);
        int size=ids.size();
        IPv4NetID netId(ids[Rng::intrand(size)]);
        ids.erase(remove(ids.begin(), ids.end(), netId), ids.end());
        return newNetLayer(netId);
    } else
    {
        //throw IllegalStateException("No (more) Hosts are assigned to \""+id+"\"");
        throw std::runtime_error(string("No (more) Hosts are assigned to \"")+id+"\"");
    }
}

GnpNetLayer* GnpNetLayerFactory::newNetLayer(const IPv4NetID& netID)
{
    hostPool_t::const_iterator host(hostPool.find(netID));
    assert(host!=hostPool.end());
    const GnpHostInfo&info(host->second);
    const GnpPosition& gnpPos=info.gnpPosition;
    const GeoLocation& geoLoc=info.geoLoc;
    const double bw_down = isnan(info.maxDownBandwidth) ? downBandwidth : info.maxDownBandwidth;
    const double bw_up = isnan(info.maxUpBandwidth) ? upBandwidth : info.maxUpBandwidth;
    const double wireAccessLatency = isnan(info.accessLatency) ? accessLatency : info.accessLatency;
    auto_ptr<GnpNetLayer> nw(new GnpNetLayer(subnet, netID, gnpPos, geoLoc, bw_down, bw_up, wireAccessLatency));
    // hostPool.remove(netID); //TODO: Why remove? This information is needed if a host is in multiple groups
    return nw.release();
}

void GnpNetLayerFactory::setGnpFile(const string& gnpFileName)
{
    MySAX2Handler handler;
    HostsParser hostsParser;
    GroupLookupParser namedGroupsParser;
    pingErLookup.reset(new PingErLookup());
    topology::PingERLookupParser pingErLookupParser(*pingErLookup);
    countryLookup.reset(new CountryLookup());
    topology::CountryLookupParser countryLookupParser(*countryLookup);
    handler.registerStartElementListener("gnp/Hosts/Host",
                                         boost::bind(&HostsParser::startElement, &hostsParser, _1, _2, _3, _4));
    handler.registerStartElementListener("gnp/GroupLookup/Group",
                                         boost::bind(&GroupLookupParser::startGroup, &namedGroupsParser, _1, _2, _3, _4));
    handler.registerStartElementListener("gnp/GroupLookup/Group/IPs",
                                         boost::bind(&GroupLookupParser::startIPs, &namedGroupsParser, _1, _2, _3, _4));
    handler.registerStartElementListener("gnp/PingErLookup/SummaryReport",
                                         boost::bind(&topology::PingERLookupParser::startSummaryReport, &pingErLookupParser, _1, _2, _3, _4));
    handler.registerStartElementListener("gnp/CountryLookup/CountryKey",
                                         boost::bind(&topology::CountryLookupParser::startElement, &countryLookupParser, _1, _2, _3, _4));

    try
    {
        XERCES_CPP_NAMESPACE::XMLPlatformUtils::Initialize();
    } catch (const XERCES_CPP_NAMESPACE::XMLException& toCatch)
    {
        char* message=XMLString::transcode(toCatch.getMessage());
        cerr<<"Error during initialization! :\n";
        cerr<<"Exception message is: \n" <<message<<"\n";
        XMLString::release(&message);
        throw;
    }

    auto_ptr<XERCES_CPP_NAMESPACE::SAX2XMLReader> parser(XERCES_CPP_NAMESPACE::XMLReaderFactory::createXMLReader());
    parser->setFeature(XERCES_CPP_NAMESPACE::XMLUni::fgSAX2CoreValidation, true);
    parser->setFeature(XERCES_CPP_NAMESPACE::XMLUni::fgSAX2CoreNameSpaces, true); // optional

    parser->setContentHandler(&handler);
    parser->setErrorHandler(&handler);

    try
    {
        cerr<<"Reading hosts from file..."<<gnpFileName<<endl;
        parser->parse(gnpFileName.c_str());
        cerr<<"done"<<endl;
    } catch (const XERCES_CPP_NAMESPACE::XMLException& toCatch)
    {
        char* message=XMLString::transcode(toCatch.getMessage());
        cerr<<"Exception message is: \n" <<message<<"\n";
        XMLString::release(&message);
        throw;
    } catch (const XERCES_CPP_NAMESPACE::SAXParseException& toCatch)
    {
        char* message=XMLString::transcode(toCatch.getMessage());
        cerr<<"Exception message is: \n" <<message<<"\n";
        XMLString::release(&message);
        throw;
    } catch (...)
    {
        cerr<<"Unexpected Exception while parsing hosts file\n";
        throw;
    }

    hostPool.swap(hostsParser.hostPool);
    namedGroups.swap(namedGroupsParser.namedGroups);
}

void GnpNetLayerFactory::setLatencyModel(GnpLatencyModel* model)
{
    model->init(*pingErLookup, *countryLookup);
    subnet.setLatencyModel(model);
}

} } } } // namespace gnplib::impl::network::gnp

