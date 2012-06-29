#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <gnplib/impl/network/gnp/topology/PingErLookup.h>
#include <gnplib/impl/network/gnp/topology/CountryLookup.h>

using std::string;
using std::vector;
using std::set;
using std::cerr;
using std::endl;
using std::sort;

using gnplib::impl::network::gnp::topology::PingErLookup;
using gnplib::impl::network::gnp::topology::LognormalDist;
/**
 *
 * @param from PingEr Country or Region string
 * @param to PingEr Country or Region string
 * @return minimum RTT
 */
double PingErLookup::getMinimumRtt(const string& from, const string& to) const
{
    return getLinkProperty(from, to).minRtt;
}

/**
 *
 * @param from PingEr Country or Region string
 * @param to PingEr Country or Region string
 * @return average RTT
 */
double PingErLookup::getAverageRtt(const string& from, const string& to) const
{
    return getLinkProperty(from, to).averageRtt;
}

/**
 *
 * @param from PingEr Country or Region string
 * @param to PingEr Country or Region string
 * @return IQR of RTT
 */
double PingErLookup::getRttVariation(const string& from, const string& to) const
{
    return getLinkProperty(from, to).delayVariation;
}

/**
 *
 * @param from PingEr Country or Region string
 * @param to PingEr Country or Region string
 * @return packet Loss Rate in Percent
 */
double PingErLookup::getPacktLossRate(const string& from, const string& to) const
{
    return getLinkProperty(from, to).packetLoss;
}

/**
 *
 * @param ccFrom 2-digits Country Code
 * @param ccTo 2-digits Country Code
 * @param cl GeoIP to PingEr Dictionary
 * @return log-normal jitter distribution
 */
const LognormalDist& PingErLookup::getJitterDistribution(const string& ccFrom, const string& ccTo, const CountryLookup& cl)
{
    return getLinkProperty(ccFrom, ccTo, cl).getJitterDistribution();
}

/**
 *
 * @param ccFrom 2-digits Country Code
 * @param ccTo 2-digits Country Code
 * @param cl GeoIP to PingEr Dictionary
 * @return minimum RTT
 */
double PingErLookup::getMinimumRtt(const string& ccFrom, const string& ccTo, const CountryLookup& cl)
{
    return getLinkProperty(ccFrom, ccTo, cl).minRtt;
}

/**
 *
 * @param ccFrom 2-digits Country Code
 * @param ccTo 2-digits Country Code
 * @param cl GeoIP to PingEr Dictionary
 * @return average RTT
 */
double PingErLookup::getAverageRtt(const string& ccFrom, const string& ccTo, const CountryLookup& cl)
{
    return getLinkProperty(ccFrom, ccTo, cl).averageRtt;
}

/**
 *
 * @param ccFrom 2-digits Country Code
 * @param ccTo 2-digits Country Code
 * @param cl GeoIP to PingEr Dictionary
 * @return packet Loss Rate in Percent
 */
double PingErLookup::getPacktLossRate(const string& ccFrom, const string& ccTo, const CountryLookup& cl)
{
    return getLinkProperty(ccFrom, ccTo, cl).packetLoss;
}

/**
 *
 * @param from PingEr Country or Region string
 * @param to PingEr Country or Region string
 * @return LinkProperty object that contains all PingEr informations for a link
 */
const PingErLookup::LinkProperty& PingErLookup::getLinkProperty(const string& from, const string& to) const
{
    data_t::const_iterator source(data.find(from));
    if (source==data.end())
        throw std::runtime_error("getLinkProperty(): source "+from+" not found");
    std::map<std::string, LinkProperty>::const_iterator dest(source->second.find(to));
    if (dest==source->second.end())
        throw std::runtime_error("getLinkProperty(): dest "+to+" not found");
    return dest->second;
}

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
const PingErLookup::LinkProperty& PingErLookup::getLinkProperty(const string& ccFrom, const string& ccTo, const CountryLookup& cl)
{
    const char* countrySender=cl.getPingErCountryName(ccFrom);
    const char* countryReceiver=cl.getPingErCountryName(ccTo);
    const char* regionSender=cl.getPingErRegionName(ccFrom);
    const char* regionReceiver=cl.getPingErRegionName(ccTo);

    if (contains(countrySender, countryReceiver))
        return getLinkProperty(countrySender, countryReceiver);
    if (contains(countrySender, regionReceiver))
        return getLinkProperty(countrySender, regionReceiver);
    if (contains(regionSender, countryReceiver))
        return getLinkProperty(regionSender, countryReceiver);
    if (contains(regionSender, regionReceiver))
        return getLinkProperty(regionSender, regionReceiver);

    if (contains(countryReceiver, countrySender))
        return getLinkProperty(countryReceiver, countrySender);
    if (contains(countryReceiver, regionSender))
        return getLinkProperty(countryReceiver, regionSender);
    if (contains(regionReceiver, countrySender))
        return getLinkProperty(regionReceiver, countrySender);
    if (contains(regionReceiver, regionSender))
        return getLinkProperty(regionReceiver, regionSender);

    //    log.debug("Using World-Average Link Propertiess for "+ccFrom+"->"+ccTo);
    cerr<<"Using World-Average Link Propertiess for "<<ccFrom<<"->"<<ccTo<<endl;
    return getAverageLinkProperty();
}

/**
 *
 * @param from PingEr Country or Region string
 * @param to PingEr Country or Region string
 * @return true if there are PinEr Data for the pair
 */
bool PingErLookup::contains(const char* from, const char* to)
{
    if (!from || !to)
        return false;
    data_t::const_iterator source(data.find(from));
    if (source==data.end())
        return false;
    return (source->second.find(to)!=source->second.end());
}

void PingErLookup::setData(const std::string& from, const std::string& to, const PingErLookup::LinkProperty& _data)
{
    data[from][to]=_data;
}

/**
 *
 * @param from PingEr Country or Region string
 * @param to PingEr Country or Region string
 * @param value
 * @param dataType
 */
void PingErLookup::setData(const string& from, const string& to, double value, DataType dataType)
{
    LinkProperty&values(data[from][to]);
    if (dataType==MIN_RTT)
        values.minRtt=value;
    else if (dataType==AVERAGE_RTT)
        values.averageRtt=value;
    else if (dataType==VARIATION_RTT)
        values.delayVariation=value;
    else if (dataType==PACKET_LOSS)
        values.packetLoss=value;
}

/**
 * Loads the Class Attributes from an XML Element
 *
 * @param element
 */
//void PingErLookup::loadFromXML(const Element& element)
//{
//    for (Iterator<Element> iter=element.elementIterator("SummaryReport"); iter.hasNext();)
//    {
//        Element variable=iter.next();
//        string regionFrom=variable.attributeValue("from");
//        string regionTo=variable.attributeValue("to");
//        double minRtt=Double.parseDouble(variable
//                                         .attributeValue("minimumRtt"));
//        double averageRtt=Double.parseDouble(variable
//                                             .attributeValue("averageRtt"));
//        double delayVariation=Double.parseDouble(variable
//                                                 .attributeValue("delayVariation"));
//        double packetLoss=Double.parseDouble(variable
//                                             .attributeValue("packetLoss"));
//        setData(regionFrom, regionTo, minRtt, DataType.MIN_RTT);
//        setData(regionFrom, regionTo, averageRtt, DataType.AVERAGE_RTT);
//        setData(regionFrom, regionTo, delayVariation,
//                DataType.VARIATION_RTT);
//        setData(regionFrom, regionTo, packetLoss, DataType.PACKET_LOSS);
//    }
//}

/**
 * Export the Class Attributes to an XML Element
 *
 * @param element
 */
//const Element& PingErLookup::exportToXML()
//{
//    DefaultElement pingEr=new DefaultElement("PingErLookup");
//    Set<string> fromKeys=data.keySet();
//    for (string from : fromKeys)
//    {
//        Set<string> toKeys=data.get(from).keySet();
//        for (string to : toKeys)
//        {
//            DefaultElement pingErXml=new DefaultElement("SummaryReport");
//            pingErXml.addAttribute("from", from);
//            pingErXml.addAttribute("to", to);
//            pingErXml.addAttribute("minimumRtt", string
//                                   .valueOf(getMinimumRtt(from, to)));
//            pingErXml.addAttribute("averageRtt", string
//                                   .valueOf(getAverageRtt(from, to)));
//            pingErXml.addAttribute("delayVariation", string
//                                   .valueOf(getRttVariation(from, to)));
//            pingErXml.addAttribute("packetLoss", string
//                                   .valueOf(getPacktLossRate(from, to)));
//            pingEr.add(pingErXml);
//        }
//    }
//    return pingEr;
//}

/**
 * Loads PingEr Summary Reports in CVS-Format as provided on the PingER Website
 *
 * @param file
 * @param dataType
 */
//void PingErLookup::loadFromTSV(const File& file, const DataType& dataType)
//{
//    string[][] data=parseTsvFile(file);
//    for (int i=1; i<data.length-1; i++)
//    { // To
//        for (int j=1; j<data[i].length; j++)
//        { // From
//            if (!data[i][j].equals("."))
//                setData(data[0][j].replace('+', ' '), data[i][0].replace(
//                                                                         '+', ' '), Double.parseDouble(data[i][j]), dataType);
//        }
//    }
//    files.put(file.getName(), dataType.name());
//}

/**
 *
 * @param file
 * @return TSV-File Data in an 2-dimensional string Array
 */
//const string[][]&PingErLookup::parseTsvFile(const File& file)
//{
//    ArrayList<string[]>tempResult=new ArrayList<string[]>();
//    string[][] result=new string[1][1];
//    try
//    {
//        FileReader inputFile=new FileReader(file);
//        BufferedReader input=new BufferedReader(inputFile);
//        string line=input.readLine();
//        while (line!=null)
//        {
//            tempResult.add(line.split("\t"));
//            line=input.readLine();
//        }
//    } catch (IOException e)
//    {
//        e.printStackTrace();
//    }
//    return tempResult.toArray(result);
//}

/**
 *
 * @return world-average LinkProperty Object
 */
const PingErLookup::LinkProperty& PingErLookup::getAverageLinkProperty() const
{
    if (!averageLinkProperty)
    {
        int counter(0);
        averageLinkProperty.reset(new LinkProperty());
        for (data_t::const_iterator from=data.begin(); from!=data.end(); ++from)
        {
            for (std::map<std::string, LinkProperty>::const_iterator to=from->second.begin();
                 to!=from->second.end(); ++to)
            {
                const LinkProperty&lp(to->second);

                averageLinkProperty->minRtt+=lp.minRtt;
                averageLinkProperty->averageRtt+=lp.averageRtt;
                averageLinkProperty->delayVariation+=lp.delayVariation;
                averageLinkProperty->packetLoss+=lp.packetLoss;
                ++counter;
            }
        }
        averageLinkProperty->minRtt/=counter;
        averageLinkProperty->averageRtt/=counter;
        averageLinkProperty->delayVariation/=counter;
        averageLinkProperty->packetLoss/=counter;
    }
    return *averageLinkProperty;
}

/**
 *
 * @return loaded PingEr TSV-Files (map: File -> DataType)
 */
//const map<string, string>& PingErLookup::getFiles()
//{
//    return files;
//}

/**
 *
 * @return reference to the PingER Adjacency List (used by the GUI only)
 */
//const map<string, map<string, LinkProperty>>&PingErLookup::getData()
//{
//    return data;
//}



// class PingErLookup::LinkProperty:

PingErLookup::LinkProperty::LinkProperty(const LinkProperty& o)
: minRtt(o.minRtt),
averageRtt(o.averageRtt),
delayVariation(o.delayVariation),
packetLoss(o.packetLoss),
jitterDistribution(o.jitterDistribution ? new LognormalDist(*o.jitterDistribution) : 0) { }

PingErLookup::LinkProperty& PingErLookup::LinkProperty::operator=(const LinkProperty& o)
{
    minRtt=o.minRtt;
    averageRtt=o.averageRtt;
    delayVariation=o.delayVariation;
    packetLoss=o.packetLoss;
    jitterDistribution.reset(o.jitterDistribution ? new LognormalDist(*o.jitterDistribution) : 0);
    return *this;
}


/**
 *
 * @return log_normal jitter distribution calculated with the iqr and expectet jitter
 */
const LognormalDist& PingErLookup::LinkProperty::getJitterDistribution() const
{
    if (!jitterDistribution)
    {
        JitterParameter optimized=getJitterParameterDownhillSimplex(
                                                                    averageRtt-minRtt, delayVariation);
        jitterDistribution.reset(new LognormalDist(optimized.m, optimized.s));
        //        log.debug("Set lognormal Jitter-Distribution with Average Jitter of "+optimized.getAverageJitter()
        //                  +" ("+(averageRtt-minRtt)+") and an IQR of "+optimized.getIQR()
        //                  +" ("+delayVariation+")");
        cerr<<"Set lognormal Jitter-Distribution with Average Jitter of "<<optimized.getAverageJitter()
                <<" ("<<(averageRtt-minRtt)<<") and an IQR of "<<optimized.getIQR()
                <<" ("<<delayVariation<<")"<<endl;
    }
    return *jitterDistribution;
}

/**
 * Implemenation of a downhill simplex algortihm that finds the log-normal
 * parameters mu and sigma that minimized the error between measured expectation
 * and iqr and the resulting expectation and iqr.
 *
 * @param expectation value
 * @param iqr variation
 * @return JitterParameter object with the log-normal parameter mu and sigma
 */
PingErLookup::LinkProperty::JitterParameter PingErLookup::LinkProperty::getJitterParameterDownhillSimplex(double expectation, double iqr) const
{
    vector<JitterParameter> solutions;
    solutions.push_back(JitterParameter(0.1, 0.1, expectation, iqr));
    solutions.push_back(JitterParameter(0.1, 5.0, expectation, iqr));
    solutions.push_back(JitterParameter(5.0, 0.1, expectation, iqr));
    sort(solutions.begin(), solutions.end());

    // 100 interations are enough for good results
    for (int c=0; c<100; sort(solutions.begin(), solutions.end()), ++c)
    {
        JitterParameter newSolution;
        if (getNewParameter1(solutions, expectation, iqr, newSolution))
        {
            if (newSolution.getError()<solutions[0].getError())
            {
                JitterParameter newSolution2;
                if (getNewParameter2(solutions, expectation, iqr, newSolution2)
                    && newSolution2.getError()<newSolution.getError())
                {
                    solutions.erase(++++solutions.begin()); //solutions[2]
                    solutions.push_back(newSolution2);
                } else
                {
                    solutions.erase(++++solutions.begin()); //solutions[2]
                    solutions.push_back(newSolution);
                }
                continue;
            } else if (newSolution.getError()<solutions[2].getError())
            {
                solutions.erase(++++solutions.begin()); //solutions[2]
                solutions.push_back(newSolution);
                continue;
            }
        }
        solutions[1].m = solutions[1].m + 0.5*(solutions[0].m-solutions[1].m);
        solutions[2].m = solutions[2].m + 0.5*(solutions[0].m-solutions[2].m);
        solutions[1].s = solutions[1].s + 0.5*(solutions[0].s-solutions[1].s);
        solutions[2].s = solutions[2].s + 0.5*(solutions[0].s-solutions[2].s);
    }
    return solutions[0];
}

/**
 * movement of factor 2 to center of solutions
 *
 * @param solutions
 * @param expectation
 * @param iqr
 * @return moved solution
 */
bool PingErLookup::LinkProperty::getNewParameter1(const vector<JitterParameter>& solutions, double expectation, double iqr, JitterParameter& result) const
{
    double middleM=(solutions[0].m+solutions[1].m+solutions[2].m)/3.0;
    double middleS=(solutions[0].s+solutions[1].s+solutions[2].s)/3.0;
    double newM=middleM+(solutions[0].m-solutions[2].m);
    double newS=middleS+(solutions[0].s-solutions[2].s);
    if (newS>0) {
        result = JitterParameter(newM, newS, expectation, iqr);
        return true;
    }else
        return false;
}

/**
 * movement of factor 3 to center of solutions
 *
 * @param solutions
 * @param expectation
 * @param iqr
 * @return moved solution
 */
bool PingErLookup::LinkProperty::getNewParameter2(const vector<JitterParameter>& solutions, double expectation, double iqr, JitterParameter& result) const
{
    double middleM=(solutions[0].m+solutions[1].m+solutions[2].m)/3.0;
    double middleS=(solutions[0].s+solutions[1].s+solutions[2].s)/3.0;
    double newM=middleM+2*(solutions[0].m-solutions[2].m);
    double newS=middleS+2*(solutions[0].s-solutions[2].s);
    if (newS>0) {
        result = JitterParameter(newM, newS, expectation, iqr);
        return true;
    } else
        return false;
}


// @Override
//string PingErLookup::LinkProperty::toString() const
//{
//    string min=(minRtt== -1) ? "-" : string.valueOf(minRtt);
//    string average=(averageRtt== -1) ? "-" : string
//            .valueOf(averageRtt);
//    string delayVar=(delayVariation== -1) ? "-" : string
//            .valueOf(delayVariation);
//    string loss=(packetLoss== -1) ? "-" : string.valueOf(packetLoss);
//    return min+" / "+average+" / "+delayVar+" / "+loss;
//}

// class PingErLookup::LinkProperty::JitterParameter

    /**
     * error will be minimized within the downhill simplx algorithm
     *
     * @return error (variation between measured expectation
     * and iqr and the resulting log-normal expectation and iqr.
     */
    double PingErLookup::LinkProperty::JitterParameter::getError() const
    {
        LognormalDist jitterDistribution(m, s);
//        double error1=pow((iqr-(jitterDistribution.inverseF(0.75)-jitterDistribution.inverseF(0.25)))/iqr, 2);
//        double error2=pow((ew-exp(m+(pow(s, 2)/2.0)))/ew, 2);
        double error1=pow((iqr-(quantile(jitterDistribution,0.75)-quantile(jitterDistribution, 0.25)))/iqr, 2);
        double error2=pow((ew-exp(m+(pow(s, 2)/2.0)))/ew, 2);
        return error1+error2;
    }

    /**
     *
     * @return expectation value of the log-normal distribution
     */
    double PingErLookup::LinkProperty::JitterParameter::getAverageJitter()
    {
        return exp(m+(pow(s, 2)/2.0));
    }

    /**
     *
     * @return iqr of the log-normal distribution
     */
    double PingErLookup::LinkProperty::JitterParameter::getIQR()
    {
        LognormalDist jitterDistribution(m, s);
//        return jitterDistribution.inverseF(0.75)-jitterDistribution.inverseF(0.25);
        return quantile(jitterDistribution, 0.75)-quantile(jitterDistribution, 0.25);
    }

//    string PingErLookup::LinkProperty::JitterParameter::toString() const
//    {
//        LognormalDist jitterDistribution=new LognormalDist(m, s);
//        double iqr1=jitterDistribution.inverseF(0.75)
//                -jitterDistribution.inverseF(0.25);
//        double ew1=exp(m+(pow(s, 2)/2.0));
//        return string("m: ")+m+" s: "+s+" Error: "+getError()
//                +" iqr: "+iqr1+" ew: "+ew1;
//    }
