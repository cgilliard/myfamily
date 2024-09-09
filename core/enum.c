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

#include <core/enum.h>
#include <core/rc.h>

/*
// Define the EnumImpl type
Type(EnumImpl, Field(i32, variant_id), Object(Rc, value));

// Implement building and the EnumProps traits
#define IMPL EnumImpl
void EnumImpl_build(const EnumImplConfig* config)
{
	// set the variant id that's passed in
	$Var(variant_id) = config->variant_id;
	// build the reference counter. As drop is automatically called when we go
	// out of scope, and Rc handles its own dropping, there is no explicit cleanup needed.
	let rc = new (Rc, With(value, config->value));
	// Move the rc into the data structure.
	Move(&$Var(value), &rc);
}

// Return the variant_id
i32 EnumImpl_variant_id() { return $(variant_id); }

// Return a reference. We first clone the Rc and then return it as an object.
Obj EnumImpl_as_ref()
{
	var k = klone(&$(value));
	let obj = unwrap(&k);
	ReturnObj(obj);
}
#undef IMPL
*/
