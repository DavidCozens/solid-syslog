/** @file
 *  The C-linkage guard macros (EXTERN_C_BEGIN / EXTERN_C_END) that wrap every
 *  public header so a C++ consumer links the declarations with C linkage. */
#ifndef EXTERNC_H
#define EXTERNC_H

#ifdef __cplusplus
#define EXTERN_C_BEGIN \
    extern "C"         \
    {
#define EXTERN_C_END }
#else
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#endif

#endif /* EXTERNC_H */
