#ifndef EXTERNC_H
#define EXTERNC_H

// NOLINTBEGIN(cppcoreguidelines-macro-usage) -- no language alternative for extern "C" linkage blocks
#ifdef __cplusplus
#define EXTERN_C_BEGIN \
    extern "C"         \
    {
#define EXTERN_C_END }
#else
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#endif
// NOLINTEND(cppcoreguidelines-macro-usage)

#endif /* EXTERNC_H */
