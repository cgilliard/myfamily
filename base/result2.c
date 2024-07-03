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

#include <base/result2.h>

void ResultHolder_cleanup(ResultHolder *ptr) {
	cleanup(ptr->_ref);
	tlfree(ptr->_ref);
}

bool ResultHolder_clone(ResultHolder *dst, ResultHolder *src) {
	dst->_ref = tlmalloc(size(src->_ref));
	bool ret = clone(dst->_ref, src->_ref);
	return ret;
}

void *Result2_unwrap(Result2 *ptr) {
	Result2Ptr v = *ptr;
	void *ret = NULL;
	MATCH(v, VARIANT(Ok, {
		      ResultHolderPtr rholder =
			  ENUM_VALUE(rholder, ResultHolder, v);
		      ret = rholder._ref;
	      }) VARIANT(Err, { panic("Attempt to unwrap an error!"); }));
	return ret;
}
