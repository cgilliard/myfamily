#include <core/type.h>

#define HiddenApi DefineTrait(HiddenApi, Required(Const, u64, get_value), Required(Var, void, set_value, Param(u64)), Required(Const, u64, get_capacity_impl))
TraitImpl(HiddenApi);

TypeDef(Hidden, Config(u64, capacity));

Impl(Hidden, HiddenApi);
Impl(Hidden, Drop);
Impl(Hidden, Build);
