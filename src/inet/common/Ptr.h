//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_PTR_
#define __INET_PTR_

#include "inet/common/INETDefs.h"

#define INET_STD_SHARED_PTR 1
#define INET_INTRUSIVE_PTR 2
#ifndef INET_PTR_IMPLEMENTATION
#define INET_PTR_IMPLEMENTATION INET_INTRUSIVE_PTR
#endif

#if INET_PTR_IMPLEMENTATION == INET_STD_SHARED_PTR
#include <memory>
#elif INET_PTR_IMPLEMENTATION == INET_INTRUSIVE_PTR
#include "inet/common/IntrusivePtr.h"
#else
#error "Unknown shared pointer implementation selected."
#endif

namespace inet {

// The following macro and template definitions allow the user to replace the
// standard shared pointer implementation in the Packet API with another one.
// The standard shared pointer implementation is thread safe if the threading
// library is used. Thread safety requires additional synchronization primitives
// which results in unnecessary performance penalty. In general, thread safety
// is not a requirement for OMNeT++/INET simulations
#if INET_PTR_IMPLEMENTATION == INET_STD_SHARED_PTR

template<class T>
using Ptr = std::shared_ptr<T>;

template<class T, typename... Args>
Ptr<T> makeShared(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template<class T, class U>
Ptr<T> staticPtrCast(const Ptr<U>& r)
{
    return std::static_pointer_cast<T>(r);
}

template<class T, class U>
Ptr<T> dynamicPtrCast(const Ptr<U>& r)
{
    return std::dynamic_pointer_cast<T>(r);
}

template<class T, class U>
Ptr<T> constPtrCast(const Ptr<U>& r)
{
    return std::const_pointer_cast<T>(r);
}

#elif INET_PTR_IMPLEMENTATION == INET_INTRUSIVE_PTR

template<class T>
using Ptr = IntrusivePtr<T>;

template<class T, typename... Args>
Ptr<T> makeShared(Args&&... args)
{
    return IntrusivePtr<T>(new T(std::forward<Args>(args)...));
}

template<class T, class U>
Ptr<T> staticPtrCast(const Ptr<U>& r)
{
    return static_pointer_cast<T>(r);
}

template<class T, class U>
Ptr<T> dynamicPtrCast(const Ptr<U>& r)
{
    return dynamic_pointer_cast<T>(r);
}

template<class T, class U>
Ptr<T> constPtrCast(const Ptr<U>& r)
{
    return const_pointer_cast<T>(r);
}

#else
#error "Unknown Ptr implementation"
#endif

template<class T>
Ptr<T> __checknull(const Ptr<T>& p, const char *expr, const char *file, int line)
{
    if (p == nullptr)
        throw cRuntimeError("Expression %s returned nullptr at %s:%d", expr, file, line);
    return p;
}

} // namespace inet

#endif  // #ifndef __INET_PTR_
