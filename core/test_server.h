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

#ifndef _CORE_TEST_SERVER__
#define _CORE_TEST_SERVER__

#include <core/traits.h>
#include <core/type.h>

#define HttpServerCore DefineTrait(     \
    HttpServerCore,                     \
    Required(Const, void, print_stats), \
    RequiredWithDefault(format_print_stats_impl, Const, void, format_print_stats))

TraitImpl(HttpServerCore);

#define HttpServerApi DefineTrait(     \
    HttpServerApi,                     \
    Super(HttpServerCore),             \
    Required(Var, bool, start_server), \
    Required(Const, bool, is_started))

TraitImpl(HttpServerApi);

Builder(
    HttpServer,
    Config(u32, threads),
    Config(u16, port),
    Config(char*, host));

Impl(HttpServer, Drop);
Impl(HttpServer, Build);
Impl(HttpServer, HttpServerCore);
Impl(HttpServer, HttpServerApi);

#endif // _CORE_TEST_SERVER__
