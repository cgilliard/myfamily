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

#ifndef _CLEANUP_BASE__
#define _CLEANUP_BASE__

// Ensure the cleanup attribute is available
#ifdef __GNUC__
#define Cleanup(f)                                                             \
	__attribute__((warn_unused_result)) __attribute__((cleanup(f)))
#else
#define Cleanup(f)
#endif

#endif // _CLEANUP_BASE__
