
cplusplus {{
#include "BGPHeader_m.h"
#include "IPAddress.h"
}}


class BGPHeader;

class noncobject IPAddress;

//
// Represents a \BGP Msg OPEN, to be used with the BGP module.
//
// \BGP OPEN fields modelled:
// - Version : 1 octet (BGP version)
// - My Autonomous system  : 2 octets  (AS de l'emetteur)
// - Hold Time : 2 octets (nbr de secondes que l'emetteur propose pour le compteur de renetue)
// - BGP Identifier : 4 octets (identifiant BGP)
// - Optional Param Length : 1 Octet (si = 0, pas de parametres optionnels)
// - Optional Parameters : 	- Parameter Type
//							- Parameter Length
//							- Parameter Values (taille variable)
//									-Auth code
//									-Authentication Data			
//


struct BGPParameterValues
{
    short authCode;
    long authenticationData;
}

struct BGPOptionalParameters
{
    short parameterType;
    unsigned short parameterLength;
    BGPParameterValues parameterValues;
}


packet BGPOpenMessage extends BGPHeader
{
    @customize(true);
    @omitGetVerb(true);
    char version = 4;
    unsigned short myAS;
    simtime_t holdTime;
    IPAddress BGPIdentifier;
    char optionalParamLength = 0;
    BGPOptionalParameters optionalParameters[];
}
