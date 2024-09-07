// Copyright (c) 2024, The MyFamily Developers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <core/test_encapsulation.h>
#include <core/traits.h>
#include <core/type.h>

Type(HiddenDrop);
Builder(HiddenDrop);
Impl(HiddenDrop, Drop);

#define IMPL HiddenDrop
void HiddenDrop_drop() {}
#undef IMPL

Type(
    Hidden,
    Where(T, TraitBound(Drop)),
    Field(u64, value),
    Field(HiddenConfig, config),
    Generic(T, v2),
    Object(HiddenDrop, dd));

#define IMPL Hidden
void Hidden_build(const HiddenConfig* config)
{
	printf("building hidden: capacity = %" PRIu64 "\n", config->capacity);
	$Var(config) = *config;
	let hd = new (HiddenDrop);
	Move(&$Var(v2), &hd);
}

void Hidden_drop()
{
	printf("drop value=%" PRIu64 "\n", $(value));
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
