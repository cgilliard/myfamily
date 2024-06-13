
#ifndef __TESTSELF_BASE_
#define __TESTSELF_BASE_

#include <base/class.h>

CLASS(TestSelf, FIELD(u32, x) FIELD(u64, y))
IMPL(TestSelf, TRAIT_SIZE)
END_CLASS(TestSelf)
#define TestSelf DEFINE_CLASS(TestSelf)

// GETTER(TestSelf, x)
// SETTER(TestSelf, x)
GETTER(TestSelf, y)
SETTER(TestSelf, y)

#endif // __TESTSELF_BASE_
