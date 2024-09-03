#include <core/test_encapsulation.h>
#include <core/type.h>

Type(Hidden, Field(u64, value), Field(HiddenConfig, config));

#define IMPL Hidden
void Hidden_build(HiddenConfig* config)
{
	printf("building hidden: capacity = %llu\n", config->capacity);
	$Var(config) = *config;
}

void Hidden_drop()
{
	printf("drop value=%llu\n", $(value));
}
u64 Hidden_get_value()
{
	return $(value);
}
u64 Hidden_get_capacity_impl()
{
	return $(config).capacity;
}
void Hidden_set_value(u64 v)
{
	$Var(value) = v;
}
#undef IMPL
