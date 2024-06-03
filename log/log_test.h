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

#ifndef __LOG_LOG_TEST_
#define __LOG_LOG_TEST_

// doc hidden functions that tests need to access

void _debug_global_logger_is_init__();
int _log_allocate_config_option(LogConfigOption* option, size_t size, bool debug_malloc_err, void* value);
int sprintf_err(char* str, const char* string, ...);

#endif /* __LOG_LOG_TEST */
