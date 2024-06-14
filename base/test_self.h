
#ifndef __TESTSELF_BASE_
#define __TESTSELF_BASE_

#include <base/class.h>
#include <base/traits.h>

CLASS(TestSelf, FIELD(u32, x) FIELD(u64, y))
IMPL(TestSelf, TRAIT_SIZE)
#define TestSelf DEFINE_CLASS(TestSelf)
GETTER(TestSelf, y)
SETTER(TestSelf, y)

#endif // __TESTSELF_BASE_
