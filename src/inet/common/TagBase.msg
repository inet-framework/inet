//
// This program is property of its copyright holder. All rights reserved.
//

cplusplus {{
#include "INETDefs.h"
#define Register_Tag(TAG_NAME)  template<> inline int getTagIndex<TAG_NAME>() { static int index = -1; if (index == -1) index = getNextTagIndex(); return index; }
}}

namespace inet;

//
// This is an abstract base class that should not be directly added to a message as a tag.
// It is used as the base class of all tags, thus it doesn't have any fields.
//
class TagBase
{
}
