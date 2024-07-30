// Macro to expand arguments
#define EXPAND(x) x

// Helper macro to stringify arguments
#define STRINGIFY(x) #x

// Recursively define TYPES2 to process variadic arguments
#define TYPES2(a, ...) STRINGIFY(a) __VA_OPT__(, EXPAND(TYPES2(__VA_ARGS__)))
#define TYPES(...) TYPES2(__VA_ARGS__)
TYPES(int, bool, String)
