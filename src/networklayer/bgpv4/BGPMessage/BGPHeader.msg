
cplusplus {{
const int BGP_HEADER_OCTETS = 19;
}}


enum BGPType
{

    BGP_OPEN = 1;
    BGP_UPDATE = 2;
    BGP_NOTIFICATION = 3;
    BGP_KEEPALIVE = 4;
}

//
// Represents a \BGP header, to be used with the BGP module.
//
// \BGP header fields modelled:
// - Marker : 16 octets (authentification)
// - Lenght  : 2 octets  (indique la taille totale du message)
// - Type : 1 octet
//

packet BGPHeader
{
    unsigned short length = BGP_HEADER_OCTETS;
    char type @enum(BGPType);
}
