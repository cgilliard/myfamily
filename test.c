typedef struct MyObject {
	int x;
	char buf[10];
} MyObject;

int main() {
	size_t x = sizeof(MyObject);
	char data[x];

	MyObject *obj = &data;
	obj->x = 1;
	strcpy(obj->buf, "test");

	return 0;
}

