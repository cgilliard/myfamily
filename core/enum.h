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

#ifndef _CORE_ENUM__
#define _CORE_ENUM__

#include <core/rc.h>
#include <core/traits.h>
#include <core/type.h>
#include <core/unit.h>

#define PROC_ENUM_TYPES__(variant, v) static char* _Enum_variant_Global_##variant##__ = #v;
#define PROC_ENUM_TYPES_(...) PROC_ENUM_TYPES__(__VA_ARGS__)
#define PROC_ENUM_TYPES(ignore, v) PROC_ENUM_TYPES_(EXPAND_ALL v)

#define PROC_ENUM_DEFN__(name, v, ignore) _Enum_Global_##v##__,
#define PROC_ENUM_DEFN_(...) PROC_ENUM_DEFN__(__VA_ARGS__)
#define PROC_ENUM_DEFN(name, v) PROC_ENUM_DEFN_(name, EXPAND_ALL v)

// The enum macro builds the enum and the type string for the Enum.
#define Enum(name, ...)                                                    \
	typedef enum _Enum_##name{                                         \
	    FOR_EACH(PROC_ENUM_DEFN, name, (), __VA_ARGS__)                \
		_Enum_##name##_size} _Enum_##name;                         \
	FOR_EACH(PROC_ENUM_TYPES, name, (), __VA_ARGS__)                   \
	Builder(name, Config(i32, variant_id), Config(const Obj*, value)); \
	Impl(name, AsRef);                                                 \
	Impl(name, EnumProps);                                             \
	Impl(name, Build);
#define EnumImpl(name)                                                      \
	Type(name, Field(i32, variant_id), Object(Rc, value));              \
	void name##_build(const name##Config* config)                       \
	{                                                                   \
		$ContextVar($Var(), name, variant_id) = config->variant_id; \
		let rc = new (Rc, With(value, config->value));              \
		Move(&$ContextVar($Var(), name, value), &rc);               \
	}                                                                   \
	i32 name##_variant_id() { return $Context($(), name, variant_id); } \
	Obj name##_as_ref()                                                 \
	{                                                                   \
		var k = klone(&$Context($(), name, value));                 \
		let obj = unwrap(&k);                                       \
		ReturnObj(obj);                                             \
	}

#define PROC_DEFAULT(code)       \
                                 \
	default:                 \
	{                        \
		_ret__ = (code); \
	}                        \
	break;

#define PROC_VARIANT_MUT(name, ref, variant, value_name, ...) \
	case _Enum_##name##_##variant##__:                    \
	{                                                     \
		Obj value_name = ref;                         \
		_ret__ = (__VA_ARGS__);                       \
	}                                                     \
	break;

#define PROC_VARIANT____(name, ref, variant, value_name, ...) \
	case _Enum_##name##_##variant##__:                    \
	{                                                     \
		const Obj value_name = ref;                   \
		_ret__ = (__VA_ARGS__);                       \
	}                                                     \
	break;

#define PROC_VARIANT___(name, ref, variant, value_name, code, ...)          \
	__VA_OPT__(PROC_VARIANT_MUT(name, ref, variant, code, __VA_ARGS__)) \
	EXPAND __VA_OPT__(PAREN NONE)(PROC_VARIANT____(name, ref, variant, value_name, code)) __VA_OPT__(PAREN_END)
#define PROC_VARIANT__(name, ref, variant, value_name, ...) __VA_OPT__(PROC_VARIANT___(name, ref, variant, value_name, __VA_ARGS__)) EXPAND __VA_OPT__(PAREN NONE)(PROC_VARIANT____(name, ref, variant, _ignore__, value_name)) __VA_OPT__(PAREN_END)
#define PROC_VARIANT_(name, ref, variant, ...) __VA_OPT__(PROC_VARIANT__(name, ref, variant, __VA_ARGS__)) EXPAND __VA_OPT__(PAREN NONE)(PROC_DEFAULT(variant)) __VA_OPT__(PAREN_END)
#define PROC_VARIANT(...) PROC_VARIANT_(__VA_ARGS__)
#define PROC_CASE(v, variant_case) PROC_VARIANT(EXPAND_ALL v, EXPAND_ALL variant_case)

// The match macro handles pattern matching. The current macro is immutable, need to implement
// a syntax for mutable matching.
#define match(e, ...) ({                                             \
	let _v__ = as_ref(&e);                                       \
	i32 _v_id__ = variant_id(&e);                                \
	Obj _ret__ = OBJECT_INIT;                                    \
	switch (_v_id__)                                             \
	{                                                            \
		FOR_EACH(PROC_CASE, (Global, _v__), (), __VA_ARGS__) \
	}                                                            \
	_ret__;                                                      \
})

#define PROC_DEFAULTN(code) \
                            \
	default:            \
	{                   \
		(code);     \
	}                   \
	break;

#define PROC_VARIANTN_MUT(name, ref, variant, value_name, code) \
	case _Enum_##name##_##variant##__:                      \
	{                                                       \
		Obj value_name = ref;                           \
		(code);                                         \
	}                                                       \
	break;

#define PROC_VARIANTN____(name, ref, variant, value_name, code) \
	case _Enum_##name##_##variant##__:                      \
	{                                                       \
		const Obj value_name = ref;                     \
		(code);                                         \
	}                                                       \
	break;

#define PROC_VARIANTN___(name, ref, variant, value_name, code, ...)          \
	__VA_OPT__(PROC_VARIANTN_MUT(name, ref, variant, code, __VA_ARGS__)) \
	EXPAND __VA_OPT__(PAREN NONE)(PROC_VARIANTN____(name, ref, variant, value_name, code)) __VA_OPT__(PAREN_END)
#define PROC_VARIANTN__(name, ref, variant, value_name, ...) __VA_OPT__(PROC_VARIANTN___(name, ref, variant, value_name, __VA_ARGS__)) EXPAND __VA_OPT__(PAREN NONE)(PROC_VARIANTN____(name, ref, variant, _ignore__, value_name)) __VA_OPT__(PAREN_END)
#define PROC_VARIANTN_(name, ref, variant, ...) __VA_OPT__(PROC_VARIANTN__(name, ref, variant, __VA_ARGS__)) EXPAND __VA_OPT__(PAREN NONE)(PROC_DEFAULTN(variant)) __VA_OPT__(PAREN_END)
#define PROC_VARIANTN(...) PROC_VARIANTN_(__VA_ARGS__)
#define PROC_CASEN(v, variant_case) PROC_VARIANTN(EXPAND_ALL v, EXPAND_ALL variant_case)

// The matchn macro is identical to match except that there's no return value.
// This allows for simple match expressions that do not return a value.
#define matchn(e, ...) ({                                             \
	let _v__ = as_ref(&e);                                        \
	i32 _v_id__ = variant_id(&e);                                 \
	switch (_v_id__)                                              \
	{                                                             \
		FOR_EACH(PROC_CASEN, (Global, _v__), (), __VA_ARGS__) \
	};                                                            \
})

static bool is_prim_match(const char* expected, const char* found)
{
	if (!strcmp(found, "I8") && !strcmp(expected, "int8_t"))
		return true;
	if (!strcmp(found, "I16") && !strcmp(expected, "int16_t"))
		return true;
	if (!strcmp(found, "I32") && !strcmp(expected, "int32_t"))
		return true;
	if (!strcmp(found, "I64") && !strcmp(expected, "int64_t"))
		return true;
	if (!strcmp(found, "I128") && !strcmp(expected, "int128_t"))
		return true;
	if (!strcmp(found, "U8") && !strcmp(expected, "uint8_t"))
		return true;
	if (!strcmp(found, "U16") && !strcmp(expected, "uint16_t"))
		return true;
	if (!strcmp(found, "U32") && !strcmp(expected, "uint32_t"))
		return true;
	if (!strcmp(found, "U64") && !strcmp(expected, "uint64_t"))
		return true;
	if (!strcmp(found, "U128") && !strcmp(expected, "uint128_t"))
		return true;
	if (!strcmp(found, "F32") && !strcmp(expected, "float"))
		return true;
	if (!strcmp(found, "F64") && !strcmp(expected, "double"))
		return true;
	if (!strcmp(found, "Bool") && !strcmp(expected, "_Bool"))
		return true;
	return false;
}

// Check that the type is the expected type.
#define ENUM_CHECK_TYPE_MATCH(name, variant, v) ({\
	char* found_name = TypeName(v);\
	char *exp_name = _Enum_variant_Global_##variant##__;\
if (strcmp(exp_name, found_name)) { if (!is_prim_match(exp_name, found_name)) panic("Enum type mismatch! Expected [%s]. Found [%s].", exp_name, found_name); } })

// Box primitive values
#define HANDLE_PRIM_BOXING(name, variant, v) ({           \
	let _val__ = Box(v);                              \
	ENUM_CHECK_TYPE_MATCH(name, variant, _val__);     \
	Obj _ret__ = new (                                \
	    name,                                         \
	    With(variant_id, _Enum_Global_##variant##__), \
	    With(value, &_val__));                        \
	_ret__;                                           \
})

#define HANDLE_PRIM_GEN(name, variant, _val__) ({         \
	ENUM_CHECK_TYPE_MATCH(name, variant, _val__);     \
	Obj _ret__ = new (                                \
	    name,                                         \
	    With(variant_id, _Enum_Global_##variant##__), \
	    With(value, &_val__));                        \
	_ret__;                                           \
})

#define _(name, variant, v) _Generic((v), \
    bool: ({ let _val__ = new (Bool, With(value, v)); HANDLE_PRIM_GEN(name, variant, _val__); }),                          \
    f32: ({ let _val__ = new (F32, With(value, v)); HANDLE_PRIM_GEN(name, variant, _val__); }),                           \
    f64: ({ let _val__ = new (F64, With(value, v)); HANDLE_PRIM_GEN(name, variant, _val__); }),                           \
    i8: ({ let _val__ = new (I8, With(value, v)); HANDLE_PRIM_GEN(name, variant, _val__); }),                            \
    i16: ({ let _val__ = new (I16, With(value, v)); HANDLE_PRIM_GEN(name, variant, _val__); }),                           \
    i32: ({ let _val__ = new (I32, With(value, v)); HANDLE_PRIM_GEN(name, variant, _val__); }),                           \
    i64: ({ let _val__ = new (I64, With(value, v)); HANDLE_PRIM_GEN(name, variant, _val__); }),                           \
    i128: ({ let _val__ = new (I128, With(value, v)); HANDLE_PRIM_GEN(name, variant, _val__); }),                          \
    u8: ({ let _val__ = new (U8, With(value, v)); HANDLE_PRIM_GEN(name, variant, _val__); }),                            \
    u16: ({ let _val__ = new (U16, With(value, v)); HANDLE_PRIM_GEN(name, variant, _val__); }),                           \
    u32: ({ let _val__ = new (U32, With(value, v)); HANDLE_PRIM_GEN(name, variant, _val__); }),                           \
    u64: ({ let _val__ = new (U64, With(value, v)); HANDLE_PRIM_GEN(name, variant, _val__); }),                           \
    u128: ({ let _val__ = new (U128, With(value, v)); HANDLE_PRIM_GEN(name, variant, _val__); }))

// type specific implementations both primitive and Object
// variants may be created

#define _obj(name, variant, v) ({                         \
	ENUM_CHECK_TYPE_MATCH(name, variant, v);          \
	Obj _ret__ = new (                                \
	    name,                                         \
	    With(variant_id, _Enum_Global_##variant##__), \
	    With(value, &v));                             \
	_ret__;                                           \
})

// TODO: this is a stub implementation. Once we have String,
// replace with the String.
#define _string(name, variant, v) ({                      \
	const Obj _val__ = new (U32, With(value, 1234));  \
	Obj _ret__ = new (                                \
	    name,                                         \
	    With(variant_id, _Enum_Global_##variant##__), \
	    With(value, &_val__));                        \
	_ret__;                                           \
})

#define _i8(name, variant, v) ({                   \
	i64 _vin__ = v;                            \
	HANDLE_PRIM_BOXING(name, variant, _vin__); \
})

#define _i16(name, variant, v) ({                  \
	i64 _vin__ = v;                            \
	HANDLE_PRIM_BOXING(name, variant, _vin__); \
})

#define _i32(name, variant, v) ({                  \
	i32 _vin__ = v;                            \
	HANDLE_PRIM_BOXING(name, variant, _vin__); \
})

#define _i64(name, variant, v) ({                  \
	i64 _vin__ = v;                            \
	HANDLE_PRIM_BOXING(name, variant, _vin__); \
})

#define _i128(name, variant, v) ({                 \
	i128 _vin__ = v;                           \
	HANDLE_PRIM_BOXING(name, variant, _vin__); \
})

#define _u8(name, variant, v) ({                   \
	u8 _vin__ = v;                             \
	HANDLE_PRIM_BOXING(name, variant, _vin__); \
})

#define _u16(name, variant, v) ({                  \
	u16 _vin__ = v;                            \
	HANDLE_PRIM_BOXING(name, variant, _vin__); \
})

#define _u32(name, variant, v) ({                  \
	u32 _vin__ = v;                            \
	HANDLE_PRIM_BOXING(name, variant, _vin__); \
})

#define _u64(name, variant, v) ({                  \
	u64 _vin__ = v;                            \
	HANDLE_PRIM_BOXING(name, variant, _vin__); \
})

#define _u128(name, variant, v) ({                 \
	u128 _vin__ = v;                           \
	HANDLE_PRIM_BOXING(name, variant, _vin__); \
})

#define _f32(name, variant, v) ({                  \
	f32 _vin__ = v;                            \
	HANDLE_PRIM_BOXING(name, variant, _vin__); \
})

#define _f64(name, variant, v) ({                  \
	f64 _vin__ = v;                            \
	HANDLE_PRIM_BOXING(name, variant, _vin__); \
})

#define _bool(name, variant, v) ({                 \
	bool _vin__ = v;                           \
	HANDLE_PRIM_BOXING(name, variant, _vin__); \
})

#endif // _CORE_ENUM__
