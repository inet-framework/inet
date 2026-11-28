// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/applications/mqtt/MqttTopicFilter.h"
#include "inet/applications/mqtt/MqttPacketSerializer.h"
namespace inet::mqtt {
namespace {
std::vector<std::string> levels(const std::string& text)
{
    std::vector<std::string> result;
    size_t start = 0;
    while (true) {
        auto end = text.find('/', start);
        result.push_back(text.substr(start, end == std::string::npos ? end : end - start));
        if (end == std::string::npos) return result;
        start = end + 1;
    }
}
}
bool MqttTopicFilter::isValidTopic(const std::string& topic)
{
    return !topic.empty() && topic.size() <= 65535 && topic.find_first_of("+#") == std::string::npos && MqttPacketSerializer::isValidUtf8(topic);
}
bool MqttTopicFilter::isValidFilter(const std::string& filter)
{
    if (filter.empty() || filter.size() > 65535 || !MqttPacketSerializer::isValidUtf8(filter)) return false;
    auto parts = levels(filter);
    for (size_t i = 0; i < parts.size(); i++) {
        auto& part = parts[i];
        if (part.find('+') != std::string::npos && part != "+") return false;
        if (part.find('#') != std::string::npos && (part != "#" || i + 1 != parts.size())) return false;
    }
    return true;
}
bool MqttTopicFilter::matches(const std::string& filter, const std::string& topic)
{
    // Section 4.7: wildcards at the first level do not match the '$' namespace.
    if (!isValidFilter(filter) || !isValidTopic(topic)) return false;
    if (topic.front() == '$' && (filter.front() == '#' || filter.front() == '+')) return false;
    auto f = levels(filter), t = levels(topic);
    size_t i = 0;
    for (; i < f.size(); i++) {
        if (f[i] == "#") return true;
        if (i == t.size() || (f[i] != "+" && f[i] != t[i])) return false;
    }
    return i == t.size();
}
}
