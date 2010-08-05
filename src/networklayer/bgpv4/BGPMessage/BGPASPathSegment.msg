
enum BGPPathSegmentType
{

    AS_SET = 1;
    AS_SEQUENCE = 2;
}

// AS_PATH (RFC4271 Section 4.3, Type code 2):
class BGPASPathSegment
{
    @customize(true);
    unsigned char type @enum(BGPPathSegmentType);
    unsigned char length;
    unsigned short asValue[];
}
