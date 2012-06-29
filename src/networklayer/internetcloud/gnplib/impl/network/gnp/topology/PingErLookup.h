#ifndef __GNPLIB_IMPL_NETWORK_GNP_TOPOLOGY_PINGERLOOKUP_H
#define __GNPLIB_IMPL_NETWORK_GNP_TOPOLOGY_PINGERLOOKUP_H

#include <string>
#include <vector>
#include <map>
#include <boost/scoped_ptr.hpp>
#include <boost/math/distributions/lognormal.hpp>

namespace gnplib { namespace impl { namespace network { namespace gnp { namespace topology {

class CountryLookup;

typedef boost::math::lognormal_distribution<> LognormalDist;

/**
 * This Class Implements a container for the PingER summary reports
 * used as a lookup table for rtt, packet loss and jitter distribution
 * 
 * @author Gerald Klunker
 * @author ported to C++ by Philipp Berndt <philipp.berndt@tu-berlin.de>
 * @version 0.1, 05.02.2008
 *
 */
class PingErLookup
{
public:

    /**
     * This class encapsulates the PingER summary Data for one aggregated
     * Country/Region to Country/Region link.
     * 
     * The calculation of the log-normal jitter distribution is implemented within this class.
     *
     */
    class LinkProperty
    {

        /**
         * Container the log-normal distribution parameters.
         * Used in the Downhill Simplex method.
         *
         */
        class JitterParameter
        {
            friend class LinkProperty;
            double m;
            double s;
            double ew;
            double iqr;

        public:

            inline JitterParameter()
            : m(),
            s(),
            ew(),
            iqr() { }

            inline JitterParameter(double _m, double _s, double _ew, double _iqr)
            : m(_m),
            s(_s),
            ew(_ew),
            iqr(_iqr) { }

            /**
             * error will be minimized within the downhill simplx algorithm
             *
             * @return error (variation between measured expectation
             * and iqr and the resulting log-normal expectation and iqr.
             */
            double getError() const;

            inline bool operator<(const JitterParameter& other) const
            {
                return getError()<other.getError();
            }

            /**
             *
             * @return expectation value of the log-normal distribution
             */
            double getAverageJitter();

            /**
             *
             * @return iqr of the log-normal distribution
             */
            double getIQR();

            std::string toString() const;
        };

    protected:
        friend class PingErLookup;
        double minRtt;
        double averageRtt;
        double delayVariation; // IQR
        double packetLoss;


    private:
        mutable boost::scoped_ptr<LognormalDist> jitterDistribution;

        /**
         *
         * @return log_normal jitter distribution calculated with the iqr and expectet jitter
         */
    public:

        inline LinkProperty()
        : minRtt(),
        averageRtt(),
        delayVariation(),
        packetLoss(),
        jitterDistribution() { }

        inline LinkProperty(double _minRtt, double _avgRtt, double _delayVariation, double _packetLoss)
        : minRtt(_minRtt),
        averageRtt(_avgRtt),
        delayVariation(_delayVariation),
        packetLoss(_packetLoss),
        jitterDistribution() { }

        LinkProperty(const LinkProperty& orig);

        LinkProperty&operator=(const LinkProperty& orig);

        const LognormalDist& getJitterDistribution() const;

        /**
         * Implemenation of a downhill simplex algortihm that finds the log-normal
         * parameters mu and sigma that minimized the error between measured expectation
         * and iqr and the resulting expectation and iqr.
         *
         * @param expectation value
         * @param iqr variation
         * @return JitterParameter object with the log-normal parameter mu and sigma
         */
    private:
        JitterParameter getJitterParameterDownhillSimplex(double expectation, double iqr) const;

        /**
         * movement of factor 2 to center of solutions
         *
         * @param solutions
         * @param expectation
         * @param iqr
         * @return moved solution
         */
        bool getNewParameter1(const std::vector<JitterParameter>& solutions, double expectation, double iqr, JitterParameter& result) const;

        /**
         * movement of factor 3 to center of solutions
         *
         * @param solutions
         * @param expectation
         * @param iqr
         * @return moved solution
         */
        bool getNewParameter2(const std::vector<JitterParameter>& solutions, double expectation, double iqr, JitterParameter& result) const;

    public:
        std::string toString() const;

    };
private:

    enum DataType
    {
        MIN_RTT, AVERAGE_RTT, VARIATION_RTT, PACKET_LOSS
    };
    std::map<std::string, std::string> files;

private:
    typedef std::map<std::string, std::map<std::string, LinkProperty> > data_t;
    data_t data;
    mutable boost::scoped_ptr<LinkProperty> averageLinkProperty;

    /**
     *
     * @param from PingEr Country or Region String
     * @param to PingEr Country or Region String
     * @return minimum RTT
     */
public:
    double getMinimumRtt(const std::string& from, const std::string& to) const;

    /**
     *
     * @param from PingEr Country or Region String
     * @param to PingEr Country or Region String
     * @return average RTT
     */
    double getAverageRtt(const std::string& from, const std::string& to) const;

    /**
     *
     * @param from PingEr Country or Region String
     * @param to PingEr Country or Region String
     * @return IQR of RTT
     */
    double getRttVariation(const std::string& from, const std::string& to) const;

    /**
     *
     * @param from PingEr Country or Region String
     * @param to PingEr Country or Region String
     * @return packet Loss Rate in Percent
     */
    double getPacktLossRate(const std::string& from, const std::string& to) const;

    /**
     *
     * @param ccFrom 2-digits Country Code
     * @param ccTo 2-digits Country Code
     * @param cl GeoIP to PingEr Dictionary
     * @return log-normal jitter distribution
     */
    const LognormalDist& getJitterDistribution(const std::string& ccFrom, const std::string& ccTo, const CountryLookup& cl);

    /**
     *
     * @param ccFrom 2-digits Country Code
     * @param ccTo 2-digits Country Code
     * @param cl GeoIP to PingEr Dictionary
     * @return minimum RTT
     */
    double getMinimumRtt(const std::string& ccFrom, const std::string& ccTo, const CountryLookup& cl);

    /**
     *
     * @param ccFrom 2-digits Country Code
     * @param ccTo 2-digits Country Code
     * @param cl GeoIP to PingEr Dictionary
     * @return average RTT
     */
    double getAverageRtt(const std::string& ccFrom, const std::string& ccTo, const CountryLookup& cl);

    /**
     *
     * @param ccFrom 2-digits Country Code
     * @param ccTo 2-digits Country Code
     * @param cl GeoIP to PingEr Dictionary
     * @return packet Loss Rate in Percent
     */
    double getPacktLossRate(const std::string& ccFrom, const std::string& ccTo, const CountryLookup& cl);

private:
    /**
     *
     * @param from PingEr Country or Region String
     * @param to PingEr Country or Region String
     * @return LinkProperty object that contains all PingEr informations for a link
     */
    const LinkProperty& getLinkProperty(const std::string& from, const std::string& to) const;

    /**
     * Method will search the available PingER Data for best fit with ccFrom and ccTo.
     * If there is no Country-Country connections other possibilities are testet
     * like Country-Region, Region-Country, Region-Region also in opposite direction.
     * If there is no PingEr Data for the pair a World-Average Link Property
     * will be returned.
     *
     * @param ccFrom 2-digits Country Code
     * @param ccTo 2-digits Country Code
     * @param cl GeoIP to PingEr Dictionary
     * @return LinkProperty object that contains all PingEr informations for a link
     */
    const LinkProperty& getLinkProperty(const std::string& ccFrom, const std::string& ccTo, const CountryLookup& cl);

    /**
     *
     * @param from PingEr Country or Region String
     * @param to PingEr Country or Region String
     * @return true if there are PinEr Data for the pair
     */
    bool contains(const char* from, const char* to);

    /**
     *
     * @param from PingEr Country or Region String
     * @param to PingEr Country or Region String
     * @param value
     * @param dataType
     */
    void setData(const std::string& from, const std::string& to, double value, DataType dataType);

public:
    void setData(const std::string& from, const std::string& to, const LinkProperty& data);

    /**
     * Loads the Class Attributes from an XML Element
     *
     * @param element
     */
    // void loadFromXML(const Element& element);

    /**
     * Export the Class Attributes to an XML Element
     *
     * @param element
     */
    // const Element& exportToXML();

    /**
     * Loads PingEr Summary Reports in CVS-Format as provided on the PingER Website
     *
     * @param file
     * @param dataType
     */
    // void loadFromTSV(const File& file, const DataType& dataType);

private:
    /**
     *
     * @param file
     * @return TSV-File Data in an 2-dimensional String Array
     */
    // const std::string[][]&parseTsvFile(const File& file);

    /**
     *
     * @return world-average LinkProperty Object
     */
    const LinkProperty& getAverageLinkProperty() const;

public:

    /**
     *
     * @return loaded PingEr TSV-Files (map: File -> DataType)
     */
    const std::map< std::string, std::string >& getFiles();

    /**
     *
     * @return reference to the PingER Adjacency List (used by the GUI only)
     */
    const std::map< std::string, std::map<std::string, LinkProperty> >& getData();
};

} } } } } // namespace gnplib::impl::network::gnp::topology

#endif // not defined __GNPLIB_IMPL_NETWORK_GNP_TOPOLOGY_PINGERLOOKUP_H
