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

#include <core/prim.h>
#include <core/rc.h>

// Rc has two 'FatPtr's The first is a counter and the second is holds the value. The value
// can be any object (Obj).
Type(Rc, Field(FatPtr, counter), Field(FatPtr, value));

#define IMPL Rc
// Build an Rc.
void Rc_build(const RcConfig* config)
{
	// Allocate memory for both the counter and the value. The value is an Obj regardless of
	// what type. It's data is held in the Object's own FatPtr.
	chain_malloc(&($Var(value)), sizeof(Obj));
	if ($(value).data == NULL)
		panic("Could not allocate sufficient memory");
	chain_malloc(&($Var(counter)), sizeof(u64));
	if ($(counter).data == NULL)
	{
		chain_free(&($Var(value)));
		panic("Could not allocate sufficient memory");
	}
	// pointer to the internal data location
	Obj* obj = $Var(value).data;
	// initialize the object
	*obj = OBJECT_INIT;
	// if a value is passed in (which will be the case generally if the user is calling new,
	// we move the Object into the shared memory location.
	if (config->value)
		Move(obj, config->value);
	// Initialize the counter to 1.
	*(u64*)$Var(counter).data = 1;
}
// Drop an Rc.
void Rc_drop()
{
	// When an instance of Rc is dropped, we decrement our counter and check if it is 0
	// indicating that no more references remain.
	(*(u64*)$Var(counter).data)--;
	if ((*(u64*)$Var(counter).data) == 0)
	{
		// No other references remain.
		Obj* value = $(value).data;
		// If the Object is not yet consumed, drop it.
		if ((value->flags & OBJECT_FLAGS_CONSUMED) == 0)
			drop(value);
		// Free the allocated memory for this Rc.
		if (chain_free(&($Var(counter))))
			panic("Unexpected return from a chain_free!");
		if (chain_free(&($Var(value))))
			panic("Unexpeted return from a chain_free!");
	}
}

// Unwrap an Rc.
Obj Rc_unwrap()
{
	Obj ret;
	// If there are additional references, the returned object (acting as a reference to the shared data)
	// should not be cleaned up. We merely copy the data and marked the returned reference as 'no cleanup'.
	if (*(u64*)($(counter).data) > 1)
	{
		ret = *(Obj*)$(value).data;
		ret.flags |= OBJECT_FLAGS_NO_CLEANUP;
	}
	else
	{
		// This is the final reference so we move out the Object
		ret = OBJECT_INIT;
		Move(&ret, (Obj*)$(value).data);
	}
	// Drop this instance of the Rc
	drop($Var());
	// return the objecct.
	return ret;
}

// Clone the Rc.
Obj Rc_klone()
{
	// Update the counter by incrementing
	u64* counter = $(counter).data;
	(*counter)++;

	// Create an Rc without a value
	var ret = new (Rc);
	// Set both the value and the counter to the appropriate values
	$ContextVar((&ret), Rc, value).data = ((Obj*)$(value).data);
	$ContextVar((&ret), Rc, counter).data = counter;
	// Safely return the Object
	ReturnObj(ret);
}
#undef IMPL
