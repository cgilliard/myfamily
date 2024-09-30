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

#ifndef _FAML_PARSER__
#define _FAML_PARSER__

#include <base/path.h>
#include <faml/faml.h>

typedef struct FamlParserImpl {

} FamlParserImpl;

void faml_parser_cleanup(FamlParserImpl *ptr);

#define FamlParser FamlParserImpl __attribute__((warn_unused_result, cleanup(faml_parser_cleanup)))

int faml_parse_file(FamlParser *parser, const Path *file, FamlObj *obj);
int faml_parse(FamlParser *parser, const char *conf, FamlObj *obj);
int faml_display_error(const FamlParser *parser);

int faml_array_len(const FamlObj *obj);

#endif // _FAML_PARSER__
