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

#ifndef SYN_EXPR
#define SYN_EXPR

enum ExprEnum {
	ExprSlice = 1, // slice literal [1, 2, 3, 4]
	ExprAssign = 2, // a = test()
	ExprBlock = 3, // { ... }
	ExprBinary = 4, // a + b, a += b
	ExprBreak = 5, // break c-style break only
	ExprCall = 6, // function call test(1)
	ExprClosure = 7, // |a, b| a + b
	ExprConst = 8, // const { ... }
	ExprContinue = 9, // continue c-style continue only
	ExprField = 10, // Access of a named struct field (obj.k) or unnamed tuple struct field (obj.0).
	ExprForLoop = 11, // for pattern in expr { ... }
	ExprIf = 12, // if { ... } else { ... }
	ExprIndex = 13, // v[3]
	ExprInfer = 14, // _
	ExprDefine = 15, // u32 x = 1
	ExprLit = 16, // literal "abc def", 4
	ExprWhile = 17, // while expr { ... }
	

};
typedef enum ExprEnum ExprEnum;

struct Expr {

};
typedef struct Expr Expr;

#endif /* SYN_EXPR */
