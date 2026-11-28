// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_MQTTTOPICFILTER_H
#define __INET_MQTTTOPICFILTER_H
#include "inet/common/INETDefs.h"
namespace inet::mqtt {
class INET_API MqttTopicFilter
{
  public:
    static bool isValidTopic(const std::string& topic);
    static bool isValidFilter(const std::string& filter);
    static bool matches(const std::string& filter, const std::string& topic);
};
}
#endif
