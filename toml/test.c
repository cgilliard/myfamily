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

#include <core/resources.h>
#include <core/test.h>
#include <toml/toml.h>

MySuite(toml);

MyTest(toml, test_toml_basic) {
	char errbuf[200];

	FILE *fp = myfopen("./resources/test1.toml", "r");

	toml_table_t *conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
	myfclose(fp);

	toml_table_t *package = toml_table_in(conf, "package");
	cr_assert(package);
	toml_table_t *package2 = toml_table_in(conf, "package2");
	cr_assert(!package2);

	toml_array_t *keywords = toml_array_in(package, "keywords");
	cr_assert(keywords);

	int i = 0;
	for (;; i++) {
		toml_datum_t key = toml_string_at(keywords, i);
		if (!key.ok)
			break;
		char *value = key.u.s;

		if (i == 0)
			cr_assert(!strcmp(value, "Jesus"));
		else if (i == 1)
			cr_assert(!strcmp(value, "Bible"));
		else if (i == 2)
			cr_assert(!strcmp(value, "Family"));
		else if (i == 3)
			cr_assert(!strcmp(value, "Crypto"));
	}
	cr_assert_eq(i, 4);
}
