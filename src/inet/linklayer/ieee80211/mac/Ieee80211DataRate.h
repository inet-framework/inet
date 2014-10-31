#ifndef __INET_IEEE80211DATARATE_H
#define __INET_IEEE80211DATARATE_H

#include "inet/physicallayer/common/ModulationType.h"

namespace inet {

namespace ieee80211 {

struct Ieee80211DescriptorData
{
    char mode;
    double bitrate;
    ModulationType modulationType;
};

class Ieee80211Descriptor
{
  private:
    static const int descriptorSize;
    static const Ieee80211DescriptorData data[];

  public:
    static int findIdx(char mode, double bitrate);
    static int getIdx(char mode, double bitrate);
    static int getMinIdx(char mode);
    static int getMaxIdx(char mode);
    static bool incIdx(int& idx);
    static bool decIdx(int& idx);
    static const Ieee80211DescriptorData& getDescriptor(int idx);
    static ModulationType getModulationType(char mode, double bitrate);
    static int size() { return descriptorSize; }
};

} // namespace ieee80211

} // namespace inet

#endif // ifndef __INET_IEEE80211DATARATE_H

