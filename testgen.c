#include "core/class2.h"

TRAIT(Hash, ASSOCIATED_TYPE(Item, IMPLEMENTS(Display, Clone)),
      WHERE(GENERIC(V, "Clone")),
      TRAIT_REQUIRED(int, hash1, WHERE(GENERIC(K, "Copy")), int input))
TRAIT(Copy, WHERE(GENERIC(K, "Copy")),
      TRAIT_REQUIRED(bool, copy, void *dst, void *src)
	  TRAIT_REQUIRED(bool, copy_quick, void *dst, void *src)
	      TRAIT_IMPL(copy_test, copy_test_impl) TRAIT_SUPER(Copy0))

TRAIT(Iterator, ASSOCIATED_TYPE(Item), REQUIRED(Option(ASSOCIATED(Item)

TRAIT(MyTrait, WHERE()

CLASS(MyClass1, WHERE(GENERIC(K, "Hash", "Copy"), GENERIC(V)),
      FIELD(Slab, x) FIELD(u64, capacity))
#define MyClass1 DEFINE_CLASS(MyClass1)

CLASS(MyClass2, WHERE(), FIELD(i32, x))
#define MyClass2 DEFINE_CLASS(MyClass2)

CLASS(Result, WHERE(GENERIC(T)), FIELD(GENERIC_PARAM(T), val) FIELD(int, err))
#define Result DEFINE_CLASS(Result)

int main() {
	Slab s;
	MyClass1 mc = BUILD(MyClass1, BIND(K, String), BIND(V, u64));
	MyClass2 mc2;
	Result r;

	return 0;
}
