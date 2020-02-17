#include "utils.h"

namespace keras2cpp {
    Stream::Stream(const std::string& filename)
    : stream_(filename, std::ios::binary) {
        stream_.exceptions();
        if (!stream_.is_open())
            throw std::runtime_error("Cannot open " + filename);
    }

    Stream& Stream::reads(char* ptr, size_t count) {
        stream_.read(ptr, static_cast<ptrdiff_t>(count));
        if (!stream_)
            throw std::runtime_error("File read failure");
        return *this;
    }
}
