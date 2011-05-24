//
// IEEE802.15.4 network control info: 
//
cplusplus {{
    using namespace std;
}}


message Ieee802154NetworkCtrlInfo
{
    // MAC layer is responsible for MAC address translation
    bool toParent;
    string destName;
    int  netwAddr=-1; 
}
