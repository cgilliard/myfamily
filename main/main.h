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

#ifndef _MAIN_MAIN__
#define _MAIN_MAIN__

int real_main(int argc, char **argv);

#define MAIN                                                                                       \
	int main(int argc, char **argv) {                                                              \
		return real_main(argc, argv);                                                              \
	}

#endif // _MAIN_MAIN__
