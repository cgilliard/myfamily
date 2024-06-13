#include <base/test_self.h>

GETTER(TestSelf, x)
SETTER(TestSelf, x)

void TestSelf_cleanup(TestSelfPtr *ptr) {
	printf("cleanupx.x=%i,x.y=%i\n", *TestSelf_get_x(ptr),
	       *TestSelf_get_y(ptr));
}
size_t TestSelf_size(TestSelfPtr *ptr) { return sizeof(TestSelfPtr); }
