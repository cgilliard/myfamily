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

// This file demonstrates the usage of the library. The key functionalities in the library are:
// * Encapsulation and polymorphism through the usage of a trait library.
// * Generics.
// * Mutability.
// * Super Traits.
// * Default Implementations.
// * Builder/Drop traits and automatic memory management.
// * var/let syntax and late binding of trait bounds.
// * Configuration.
// * Self operator through the $() macro.

#ifndef _CORE_TEST_SERVER__
#define _CORE_TEST_SERVER__

#include <core/traits.h>
#include <core/type.h>

// In the header file, we define two traits related to our server. The first is the core functionalities of the server.
// This is a contrived example, but demonstrates the usage of the library fairly well.
// The 'DefineTrait' macro is used to define a trait. This one is called 'HttpServerCore'. It has one required method
// and one default implementation. Any default implementation can be overriden using the 'Override' macro.
#define HttpServerCore DefineTrait(     \
    HttpServerCore,                     \
    Required(Const, void, print_stats), \
    RequiredWithDefault(format_print_stats_impl, Const, void, format_print_stats))

// The 'TraitImpl' mcro generates headers and implementations of the calling functions.
// In this case 'print_stats' and 'format_print_stats'.
TraitImpl(HttpServerCore);

// Here we define a second trait. This one names 'HttpServerCore' as a super trait. Any Type implementing
// this trait must also implement 'HttpServerCore' as well. There are three other required methods in this trait.
#define HttpServerApi DefineTrait(     \
    HttpServerApi,                     \
    Super(HttpServerCore),             \
    Required(Var, bool, start_server), \
    Required(Const, bool, is_started), \
    Required(Const, void, blorp, Param(u64)))

// Call TraitImpl as is required for all traits.
TraitImpl(HttpServerApi);

// Here we declare a 'Builder' for the HttpServer. This defines how a type can be built.
// In this case, there are three configurable parameters in the Builder: threads, port, and host.
// These will be validated and defaults set in the Builder trait implementation for this type.
Builder(
    HttpServer,
    Config(u32, threads),
    Config(u16, port),
    Config(char*, host));

// Call Impl for all four traits that the HttpServer will implement. Note that HttpServerCore must
// come before HttpServerApi since the former is declared to be a super-trait of the latter. In addition,
// the Drop and Build traits are implemented.
Impl(HttpServer, Drop);
Impl(HttpServer, Build);
Impl(HttpServer, HttpServerCore);
Impl(HttpServer, HttpServerApi);

#endif // _CORE_TEST_SERVER__
