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

Type(CanDrop);
Builder(CanDrop);
Impl(CanDrop, Drop);

#define IMPL CanDrop
void CanDrop_drop() { printf("dropping can drop\n"); }
#undef IMPL

Type(HttpServerComponent, Field(u64, state), Field(void*, alloc_mem));
Builder(HttpServerComponent, Config(u64, state));

Impl(HttpServerComponent, Drop);
Impl(HttpServerComponent, Build);

#define IMPL HttpServerComponent
void HttpServerComponent_drop()
{
	printf("dropping http server component. Free %p\n", $(alloc_mem));
	free($(alloc_mem));
}
void HttpServerComponent_build(const void* config_in)
{
	const HttpServerComponentConfig* config = config_in;
	$Var(state) = config->state;
	$Var(alloc_mem) = malloc(100);
	printf("building http server component. Alloc %p\n", $(alloc_mem));
}
#undef IMPL

Type(
    HttpServer,
    Where(T, TraitBound(Drop)),
    Field(HttpServerConfig, config),
    Field(bool, is_started),
    Object(HttpServerComponent, hsc),
    Generic(T, droppable));

#define IMPL HttpServer
void HttpServer_validate_input(const HttpServerConfig* config)
{
	if (config->threads == 0)
		panic("Threads must be greater than 0. Halting!");
	if (config->port == 0)
		panic("Port 0 is reserved. Halting!");
	if (config->host == NULL)
		panic("host must not be NULL. Halting!");

	$Var(config) = *config;
}
void HttpServer_build(const void* config_in)
{
	const HttpServerConfig* config = config_in;
	HttpServer_validate_input(config);
	$Var(is_started) = false;
	let can_drop = new (CanDrop);
	Move(&$Var(droppable), &can_drop);
	let hsc = new (HttpServerComponent, With(state, 1));
	Move(&$Var(hsc), &hsc);
}
void HttpServer_drop() { printf("dropping http server\n"); }
bool HttpServer_is_started() { return $(is_started); }
bool HttpServer_start_server()
{
	if ($(is_started))
		return false;
	$Var(is_started) = true;
	printf(
	    "Server started on %s:%" PRIu16 " with %" PRIu32 " threads\n",
	    $(config).host, $(config).port, $(config).threads);
	format_print_stats($());
	return true;
}
void HttpServer_print_stats()
{
	printf("Current stats: is_started: %i\n", $(is_started));
}
#undef IMPL

void format_print_stats_impl()
{
	printf("-----------------------------------------------------\n");
	print_stats($());
	printf("-----------------------------------------------------\n");
}
