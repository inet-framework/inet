
cplusplus {{
#include "IPAddress.h"
#include "BGPASPathSegment.h"
}}


class noncobject IPAddress;

class noncobject BGPASPathSegment;


enum BGPUpdateAttributeTypeCode
{

    ORIGIN = 1;
    AS_PATH = 2;
    NEXT_HOP = 3;
    LOCAL_PREF = 5;
    ATOMIC_AGGREGATE = 6;
}

struct BGPUpdateAttributeFlags
{
    bool optionalBit;
    bool transitiveBit;
    bool partialBit;
    bool estendedLengthBit;
}

struct BGPUpdateAttributeType
{
    BGPUpdateAttributeFlags flags;
    unsigned char typeCode @enum(BGPUpdateAttributeTypeCode);
}

class BGPUpdatePathAttributes
{
    BGPUpdateAttributeType type;	// 2 octets
    unsigned short length;	// 2 octets
}

// ORIGIN (RFC4271 Section 4.3, Type code 1):
enum BGPUpdateOriginValues
{

    IGP = 0;
    EGP = 1;
    INCOMPLETE = 2;
}

class BGPUpdatePathAttributesOrigin extends BGPUpdatePathAttributes
{
    int value @enum(BGPUpdateOriginValues);
}

// AS_PATH (RFC4271 Section 4.3, Type code 2):
class BGPUpdatePathAttributesASPath extends BGPUpdatePathAttributes
{
    BGPASPathSegment value[];
}

//NEXT_HOP (RFC4271 Section 4.3, Type code 3):
class BGPUpdatePathAttributesNextHop extends BGPUpdatePathAttributes
{
    IPAddress value;
}

//LOCAL_PREF (RFC4271 Section 4.3, Type code 5):
class BGPUpdatePathAttributesLocalPref extends BGPUpdatePathAttributes
{
    unsigned int value;
}

//ATOMIC_AGGREGATE (RFC4271 Section 4.3, Type code 6):
enum BGPUpdateAtomicAggregateValues
{

    NO_SPECIFIC_ROUTE = 0;
    SPECIFIC_ROUTE = 1;
}

class BGPUpdatePathAttributesAtomicAggregate extends BGPUpdatePathAttributes
{
    int value @enum(BGPUpdateAtomicAggregateValues);
}
