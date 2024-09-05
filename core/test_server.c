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

#include <core/test_server.h>

// In this file we implement the HttpServer type along with some other types that are
// used by HttpServer. This is a .c file so the implementation details are hidden
// from the user providing encapsulation of the data within the HttpServer type.

// We create a type that implements the 'Drop' trait. This is needed to satisfy
// the trait bound of HttpServer in our example. We also call the required macros
// to minimally intialize this type and declare that the Drop trait will be
// implemented.
Type(CanDrop);
Builder(CanDrop);
Impl(CanDrop, Drop);

// Do the drop implementation for this type. By defining 'IMPL', we have access
// to the $() macro which allows for self operations and access to the data types.
#define IMPL CanDrop
void CanDrop_drop() { printf("dropping can drop\n"); }
#undef IMPL

// Now we declare a second type which will demonstrate concrete types. This type
// also has some data fields which are used and a Builder which allows configuration.
Type(HttpServerComponent, Field(u64, state), Field(void*, alloc_mem));
Builder(HttpServerComponent, Config(u64, state));

// Implement Drop and Build for this type.
Impl(HttpServerComponent, Drop);
Impl(HttpServerComponent, Build);

// Here we implement drop and build, the required methods for our traits.
#define IMPL HttpServerComponent
void HttpServerComponent_drop()
{
	// The $() macro allows us to access data fields in our type. In this case
	// We print out $(alloc_mem) and free it.
	printf("dropping http server component. Free %p\n", $(alloc_mem));
	free($(alloc_mem));
}
void HttpServerComponent_build(const HttpServerComponentConfig* config_in)
{
	// In contrast to the drop implementation, here we use the $Var macro.
	// The difference from the $ macro is that the $Var macro is mutable.
	// If the $Var macro is used in an immutable function, a thread panic
	// will occur.
	const HttpServerComponentConfig* config = config_in;
	// Set the 'state' data member of this type using the config.
	$Var(state) = config->state;
	// allocate memory to our void pointer type to demonstrate the usage of build/drop.
	$Var(alloc_mem) = malloc(100);
	printf("building http server component. Alloc %p\n", $(alloc_mem));
}
#undef IMPL

// Now we define the main type. This type information is hidden from the user as this
// data is defined in our .c file and treated as an opaque pointer.
// The HttpServer type demonstrates the 'Where' clause for defining generics and the usage
// of all three types of data members (Generic, Object, and Field).
Type(
    HttpServer,
    Where(T, TraitBound(Drop)),
    Field(HttpServerConfig, config),
    Field(bool, is_started),
    Object(HttpServerComponent, hsc),
    Generic(T, droppable));

// Finally, we implement the HttpServer type.
#define IMPL HttpServer
// This function is not defined by any traits. It serves as a private method within our type.
// Since it is within the IMPL block, we can access both the $Var and $ variables and mutability
// is maintained based on what function calls this function.
void HttpServer_validate_input(const HttpServerConfig* config)
{
	// First set the config to the passed in configuration.
	$Var(config) = *config;
	// check threads and panic if they are not configured. memset is called on all
	// data of the type so everything will be set to 0 initially including this parameter. So,
	// here we effectively force the user to configure threads or a thread panic will occur.
	if (config->threads == 0)
		panic("Threads must be greater than 0. Halting!");

	// For port and host, we set default values keeping in mind the fact that everything is
	// initialized to 0.
	if (config->port == 0)
		$Var(config).port = 8080;
	if (config->host == NULL)
		$Var(config).host = "127.0.0.1";
}
void HttpServer_build(const HttpServerConfig* config)
{
	// The build function is automatically called by the system when an Object is instantiated.
	HttpServer_validate_input(config);
	// set is_started to false.
	$Var(is_started) = false;
	// create a 'CanDrop' object which will fulfil the trait bounds of 'droppable'.
	let can_drop = new (CanDrop);
	// Always use Move to move Objects as it will enforce all trait bounds and set the cleanup
	// and other flags appropriately for all types.
	Move(&$Var(droppable), &can_drop);
	// create an immutable HttpServerComponent instance with state initialized to 1.
	let hsc = new (HttpServerComponent, With(state, 1));
	// Once again use the Move macro to move the instance into the proper memory location.
	Move(&$Var(hsc), &hsc);
}
// Implement a drop method which just prints out the message below.
void HttpServer_drop() { printf("dropping http server\n"); }
// Return the is_started status of the server using the immutable self macro '$'.
bool HttpServer_is_started() { return $(is_started); }
// Implement the start_server Required function.
bool HttpServer_start_server()
{
	// Do error checking.
	if ($(is_started))
		return false;
	// Set is_started to appropriate value.
	$Var(is_started) = true;
	// Print the configuration
	printf(
	    "Server started on %s:%" PRIu16 " with %" PRIu32 " threads\n",
	    $(config).host, $(config).port, $(config).threads);
	// Call the format_print_stats function. This function will execute the provided format_print_stats
	// function. Note that we use the $() which refers to the pointer whose context we are currently in.
	// This can be used to call any trait member functions.
	format_print_stats($());
	return true;
}
// Implement a basic printing of the stats.
void HttpServer_print_stats()
{
	printf("Current stats: is_started: %i\n", $(is_started));
}
void HttpServer_blorp(u64 v) {}
#undef IMPL

// This is the default implementation for the format_print_stats function. Note that we can still access
// the $ and $Var macros as a sort of a self operator, but since this is not part of any implementation,
// fields cannot be specified.
void format_print_stats_impl()
{
	printf("-----------------------------------------------------\n");
	print_stats($());
	printf("-----------------------------------------------------\n");
}
