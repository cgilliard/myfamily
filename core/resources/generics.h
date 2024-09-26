
// This example shows how to refer to the implementing type from a trait.
// Since it is not known by the trait and could be different if
// multiple types implement this trait.
@Clone(T[Self]) {
	// The symbol 'T' is used to denote the unknown implementing type.
	@clone()->T;

	// This mutable function has a default implementation so no '@'. 'source' is of type '$'
	// (completing type) and it is passed by reference '&'.
	mut clone_from(T & source);
}

// SuperClone has two super traits 'Clone' and 'PartialEq'
@SuperClone(T[Copy]) : Clone, PartialEq, MyOtherTrait(T) {
}

// Trait ServerApi has a single generic which is declared as 'T'. There is also a trait bound. T
// must implement Clone.
@ServerApi(T[Clone]) {
	// The method has a parameter of type 'T' (the generic type).
	@mut start_server(T value);
}

// The Server type has two generics: T which has a trait bound of Clone and PartialEq and R which
// has the trait bound of Eq. It also has a field of each generic type and the u32 field.
Server(T[Clone, PartialEq], R[Eq]) {
	T obj1;
	R obj2;
	u32 other;
}

// This line indicates that the Server type implements the ServerApi trait. In the Server.c
// implementation file, Server must implement the required 'server_start' method.
// This particular implementation specifies that type T generic is of type 'SuperClone' which in
// turn has a super trait of Clone. Therefore, ServerApi (which uses this T generic) has its trait
// bounds satisfied because 'SuperClone' is also 'Clone'. Additionally Server has the R generic
// which is inferred to be 'Eq' from its declaration above.
impl Server(T[SuperClone, Clone, PartialEq], R[Eq]) ServerApi(T);

// notes:
// * brackets used for the trait list
// * parens used to enclose the generics
// * colon used to begin the super trait list.

// Simple Example :
// mod.h

@Swappable(T) {
	@mut swap();
}

@AccessPair(T) {
	@set_a(T a);
	@set_b(T b);
	@get_a()->T;
	@get_b()->T;
}

Pair(T) {
	T a;
	T b;
}

impl Pair(T) Swapable(T);
impl Pair(T) Build;
impl Pair(T) AccessPair(T);

// In c implementation file(Pair.c)

fn(swap) {
	mut tmp = OBJECT_INIT;
	Move(&tmp, $(b));
	Move($Mut(b), $(a));
	Move(&Mut(a), &tmp);
}

fn(build) {
	$Mut(a) = $Config(a);
	$Mut(b) = $Config(b);
}

fn(get_a) {
	return $(a);
}
fn(get_b) {
	return $(b);
}
fn(set_a) {
	$Mut(a) = a;
}
fn(set_b) {
	$Mut(b) = b;
}

// In c main file (main.c)

int main() {
	mut my_pair = _(Pair, Generic(T, u32));
	set_a(&my_pair, 1);
	set_b(&my_pair, 2);
	assert(get_a(&my_pair) == 1);
	assert(get_b(&my_pair) == 2);
	swap(&my_pair);
	assert(get_a(&my_pair) == 2);
	assert(get_b(&my_pair) == 1);
}

// Simple Example :
// mod.h

@Swappable(T) {
	@mut swap();
}

#[Derive(Get(a), Get(b), Set(a), Set(b))]
Pair(T) {
	T a;
	T b;
}

impl Pair(T) Swapable(T);
impl Pair(T) Build;

// In c implementation file(Pair.c)

fn(swap) {
	mut tmp = OBJECT_INIT;
	Move(&tmp, $(b));
	Move($Mut(b), $(a));
	Move(&Mut(a), &tmp);
}

fn(build) {
	$Mut(a) = $Config(a);
	$Mut(b) = $Config(b);
}

// In c main file (main.c)

int main() {
	mut my_pair = _(Pair, Generic(T, u32));
	set_a(&my_pair, 1);
	set_b(&my_pair, 2);
	assert(get_a(&my_pair) == 1);
	assert(get_b(&my_pair) == 2);
	swap(&my_pair);
	assert(get_a(&my_pair) == 2);
	assert(get_b(&my_pair) == 1);
}

// With swap derived as well

#[Derive(Get(a), Get(b), Set(a), Set(b), Swappable)]
Pair(T) {
	T a;
	T b;
}

// In c implementation file(Pair.c)

// nothing required

// In c main file (main.c)

int main() {
	mut my_pair = _(Pair, Generic(T, u32));
	set_a(&my_pair, 1);
	set_b(&my_pair, 2);
	assert(get_a(&my_pair) == 1);
	assert(get_b(&my_pair) == 2);
	swap(&my_pair);
	assert(get_a(&my_pair) == 2);
	assert(get_b(&my_pair) == 1);
}
