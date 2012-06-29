#include <gnplib/impl/network/IPv4NetID.h>

#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

using std::string;
using std::runtime_error;
using boost::tokenizer;
using boost::char_separator;
using boost::lexical_cast;
using boost::bad_lexical_cast;
using gnplib::impl::network::IPv4NetID;

/**
 * Creates an Instance of IPv4NetID.
 *
 * @param id
 *            The Long-ID.
 */
IPv4NetID::IPv4NetID(const string& _id)
: id(ipToLong(_id)) { }

/**
 * Prints a string representing this InternetProtocolNetID.
 */
// @Override
void IPv4NetID::print(std::ostream& os) const
{
    os<<(id>>24)<<"."<<((id&0x00ff0000)>>16)<<"."<<((id&0x0000ff00)>>8)<<"."<<(id&0x000000ff);
}

/**
 * @param ip
 *            32bit IP-Address
 * @return A readable IP-String like "192.168.0.1"
 */
//static string& IPv4NetID::ipToString(uint64_t ip)
//{
//    String returnString="";
//    returnString+=Long.toString((ip<<32)>>> 56)+".";
//    returnString+=Long.toString((ip<<40)>>> 56)+".";
//    returnString+=Long.toString((ip<<48)>>> 56)+".";
//    returnString+=Long.toString((ip<<56)>>> 56);
//    return returnString;
//}

/**
 *
 * @param ip
 *            readable IP-String like "192.168.0.1"
 * @return A 32bit IP-Address
 */
uint32_t IPv4NetID::ipToLong(const string& _ip)
{
    try
    {
        return lexical_cast<uint32_t>(_ip);
    } catch (bad_lexical_cast& e)
    {
        typedef tokenizer< char_separator<char> > tokenizer;
        tokenizer tokens(_ip, char_separator<char>("."));

        uint32_t ip(0);
        tokenizer::iterator tok_iter=tokens.begin();
        for (int i=3; i>=0; --i)
        {
            if (tok_iter==tokens.end()) throw runtime_error("Error parsing IP");
            uint8_t byte(lexical_cast<uint8_t>(*tok_iter));
            ip+=byte<<8*i;
        }
        if (tok_iter!=tokens.end()) throw runtime_error("Error parsing IP");

        return ip;
    }
}
