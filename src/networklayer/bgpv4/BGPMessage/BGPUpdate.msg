
cplusplus {{
#include "BGPHeader_m.h"
//#include "BGPHeader.h"
#include "BGPUpdatePathAttributes_m.h"
#include "IPAddress.h"
}}


class BGPHeader;

//class BGPUpdatePathAttributes;
class noncobject BGPUpdatePathAttributesOrigin;

class noncobject BGPUpdatePathAttributesASPath;

class noncobject BGPUpdatePathAttributesNextHop;

class noncobject BGPUpdatePathAttributesLocalPref;

class noncobject BGPUpdatePathAttributesAtomicAggregate;

class noncobject IPAddress;

//
// Represents a \BGP Msg UPDATE, to be used with the BGP module.
//
// \BGP UPDATE fields modelled:
// - Unfeasible routes length : 2 octet (taille de champs Withdrawn routes, si = 0, le champs Withdrawn routes n'existe pas)
// - Withdrown routes  : taille variable  (liste des préfixes d'@ IP pour les routes impraticables)
// - Total Path Attributes Length : 1 octet (si = 0, pas de Path Attributes)
// - Path Attributes : 	- Attribute Type (2 octets)
//						- Attribute Length
//						- Attribute Values (taille variable)
// - Network Layer Reachability Information : taille variable
//						- Length : 1 octet
//						- prefixe : taille variable (contient les prefixe d'@ IP) ipv4 = 4 octets
//


struct BGPUpdateWithdrawnRoutes
{
    unsigned char length;	// 1 octet
    IPAddress prefix;	// 4 octets
}

struct BGPUpdateNLRI
{
    unsigned char length;	// 1 octet
    IPAddress prefix;	// 4 octets
}

class BGPUpdatePathAttributesContent
{
    BGPUpdatePathAttributesOrigin origin;
    BGPUpdatePathAttributesASPath asPath[];
    BGPUpdatePathAttributesNextHop nextHop;
    BGPUpdatePathAttributesLocalPref localPref[];		// optional
    BGPUpdatePathAttributesAtomicAggregate atomicAggregate[];	// optional
}

packet BGPUpdate extends BGPHeader
{
    @customize(true);
    unsigned short unfeasibleRoutesLength = 0;
		//if(unfeasibleRoutesLength != 0){
    BGPUpdateWithdrawnRoutes withdrawnRoutes[];

    unsigned short totalPathAttributeLength = 0;
    //if(totalPathAttributesLengh != 0 ){
    BGPUpdatePathAttributesContent pathAttributesContent[];	// optional

    BGPUpdateNLRI NLRI;
}
