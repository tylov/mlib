/*
 * M*LIB - Extended Pre-processing macros module
 *
 * Copyright (c) 2017-2021, Patrick Pelissier
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * + Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * + Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef MSTARLIB_MACRO_H
#define MSTARLIB_MACRO_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>

/* By default, always use stdio. Can be turned off in specific environement if needed
   by defining M_USE_STDIO to 0 */
#ifndef M_USE_STDIO
# define M_USE_STDIO 1
#endif
#if M_USE_STDIO
# include <stdio.h>
#endif

/* By default, always use stdarg. Can be turned off in specific environement if needed
   by defining M_USE_STDARG to 0 */
#ifndef M_USE_STDARG
# define M_USE_STDARG 1
#endif
#if M_USE_STDARG
# include <stdarg.h>
#endif


/***************************************************************/
/************************ Compiler Macro ***********************/
/***************************************************************/

/* Define M*LIB version */
#define M_CORE_VERSION_MAJOR 0
#define M_CORE_VERSION_MINOR 5
#define M_CORE_VERSION_PATCHLEVEL 1

/* M_ASSUME is equivalent to M_ASSERT, but gives hints to compiler
   about how to optimize the code if NDEBUG is defined.
   It is worth in very specific places usually. */
#if !defined(NDEBUG)
# define M_ASSUME(x) M_ASSERT(x)
#elif defined(__GNUC__)                                                       \
  && (__GNUC__ * 100 + __GNUC_MINOR__) >= 408
# define M_ASSUME(x)                                                          \
  (! __builtin_constant_p (!!(x) || !(x)) || (x) ?                            \
   (void) 0 : __builtin_unreachable())
#elif defined(_MSC_VER)
# define M_ASSUME(x) __assume(x)
#else
# define M_ASSUME(x) M_ASSERT(x)
#endif

/* M_LIKELY / M_UNLIKELY gives hints on the compiler of the likehood
   of the given condition */
#ifdef __GNUC__
# define M_LIKELY(cond)   __builtin_expect(!!(cond), 1)
# define M_UNLIKELY(cond) __builtin_expect(!!(cond), 0)
#else
# define M_LIKELY(cond)   (cond)
# define M_UNLIKELY(cond) (cond)
#endif

/* Define the exclusion size so that 2 atomic variables are in
   separate cache lines. This prevents false sharing to occur within the
   CPU. */
#if defined(_M_X64) || defined(_M_AMD64) || defined(__x86_64__)
# define M_ALIGN_FOR_CACHELINE_EXCLUSION 128
#else
# define M_ALIGN_FOR_CACHELINE_EXCLUSION 64
#endif

/* Deprecated attribute for a function */
#if defined(__GNUC__) &&  __GNUC__ >= 4
#define M_ATTR_DEPRECATED __attribute__((deprecated))
#else
#define M_ATTR_DEPRECATED
#endif

/* Extension attribute to silent warnings on extensions */
#if defined(__GNUC__)
#define M_ATTR_EXTENSION __extension__
#else
#define M_ATTR_EXTENSION
#endif


/* Ignore some warnings detected by some compilers in the library.
 * Whatever we do, there is some warnings that cannot be fixed.
 * So they are ignored in order to avoid polluting the user with
 * theses warnings. They are:
 *
 * * If we build in C++ mode, they are warnings about using the C 
 * dialect. It is expected as M*LIB is a C library.
 *
 * * For clang, the generated functions may not be always used,
 * and CLANG failed to realize it.
 * See https://bugs.llvm.org//show_bug.cgi?id=22712
 *
 * * A manualy created buffer is given to fscanf. It is needed
 * to give the size of the array of char to fscanf (this is the safe way).
 */
#if defined(__clang__) && defined(__cplusplus)

/* Warnings disabled for CLANG in C++ mode */
#if __clang_major__ >= 6
#define M_BEGIN_PROTECTED_CODE                                                \
  _Pragma("clang diagnostic push")                                            \
  _Pragma("clang diagnostic ignored \"-Wold-style-cast\"")                    \
  _Pragma("clang diagnostic ignored \"-Wzero-as-null-pointer-constant\"")     \
  _Pragma("clang diagnostic ignored \"-Wunused-function\"")                   \
  _Pragma("clang diagnostic ignored \"-Wformat-nonliteral\"")
#else
#define M_BEGIN_PROTECTED_CODE                                                \
  _Pragma("clang diagnostic push")                                            \
  _Pragma("clang diagnostic ignored \"-Wold-style-cast\"")                    \
  _Pragma("clang diagnostic ignored \"-Wunused-function\"")                   \
  _Pragma("clang diagnostic ignored \"-Wformat-nonliteral\"")
#endif

#define M_END_PROTECTED_CODE                                                  \
  _Pragma("clang diagnostic pop")

#elif defined(__GNUC__) && defined(__cplusplus)

/* Warnings disabled for GNU C in C++ mode
 * However, G++ doesn't support well disabling temporary warnings.
 * See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53431
 */
#define M_BEGIN_PROTECTED_CODE                                                \
  _Pragma("GCC diagnostic push")                                              \
  _Pragma("GCC diagnostic ignored \"-Wold-style-cast\"")                      \
  _Pragma("GCC diagnostic ignored \"-Wzero-as-null-pointer-constant\"")       \
  _Pragma("GCC diagnostic ignored \"-Wformat-nonliteral\"")

#define M_END_PROTECTED_CODE                                                  \
  _Pragma("GCC diagnostic pop")

#elif defined(__clang__)

/* Warnings disabled for CLANG in C mode */
#define M_BEGIN_PROTECTED_CODE                                                \
  _Pragma("clang diagnostic push")                                            \
  _Pragma("clang diagnostic ignored \"-Wunused-function\"")                   \
  _Pragma("clang diagnostic ignored \"-Wformat-nonliteral\"")

#define M_END_PROTECTED_CODE                                                  \
  _Pragma("clang diagnostic pop")

#elif defined(__GNUC__)

/* Warnings disabled for GNU C in C mode */
#define M_BEGIN_PROTECTED_CODE                                                \
  _Pragma("GCC diagnostic push")                                              \
  _Pragma("GCC diagnostic ignored \"-Wformat-nonliteral\"")

#define M_END_PROTECTED_CODE                                                  \
  _Pragma("GCC diagnostic pop")

#else

/* No warnings disabled */
#define M_BEGIN_PROTECTED_CODE
#define M_END_PROTECTED_CODE

#endif


/* Autodetect if Address sanitizer is run */
#if defined(__has_feature)
# if __has_feature(address_sanitizer)
#  define M_ADDRESS_SANITIZER 1
# endif
#endif
#if defined(__SANITIZE_ADDRESS__) && !defined(M_ADDRESS_SANITIZER)
#  define M_ADDRESS_SANITIZER 1
#endif

/*
 * Do not create the following symbols that are defined in GLIBC malloc.h
 * M_MXFAST
 * M_NLBLKS
 * M_GRAIN
 * M_KEEP
 * M_TRIM_THRESHOLD
 * M_TOP_PAD
 * M_MMAP_THRESHOLD
 * M_MMAP_MAX
 * M_CHECK_ACTION
 * M_PERTURB
 * M_ARENA_TEST
 * M_ARENA_MAX
 */

M_BEGIN_PROTECTED_CODE

/************************************************************/
/********************* MEMORY handling **********************/
/************************************************************/

/* Default MEMORY handling macros.
   Can be overloaded by user code.
*/

/* Define a C and C++ version for default memory allocators.
   Note: For C build, we explicitly don't cast the return value of
   malloc, realloc as it is safer (compilers shall warn in case
   of invalid implicit cast, whereas they won't if there is an 
   explicit cast) */

/* Define allocators for object:
 * void *M_MEMORY_ALLOC(type): Return a pointer to a new object of type 'type'
 *    It returns NULL in case of memory allocation failure.
 * void M_MEMORY_DEL(ptr): Free the object associated to the pointer.
 */
#ifndef M_MEMORY_ALLOC
#ifdef __cplusplus
# include <cstdlib>
# define M_MEMORY_ALLOC(type) ((type*)std::malloc (sizeof (type)))
# define M_MEMORY_DEL(ptr)  std::free(ptr)
#else
# define M_MEMORY_ALLOC(type) malloc (sizeof (type))
# define M_MEMORY_DEL(ptr)  free(ptr)
#endif
#endif

/* Define allocators for array 
 * void *M_MEMORY_REALLOC(type, ptr, n): Return a pointer to a new array of 'n' object of type 'type'
 *    If ptr is NULL, it creates a new array.
 *    If ptr is not null, it reallocates the given array to the new size.
 *    It returns NULL in case of memory allocation failure.
 * void M_MEMORY_FREE(ptr): Free the object associated to the array.
 */
#ifndef M_MEMORY_REALLOC
#ifdef __cplusplus
# include <cstdlib>
# define M_MEMORY_REALLOC(type, ptr, n)                                       \
  ((type*) (M_UNLIKELY ((n) > SIZE_MAX / sizeof(type)) ? NULL : std::realloc ((ptr), (n)*sizeof (type))))
# define M_MEMORY_FREE(ptr) std::free(ptr)
#else
# define M_MEMORY_REALLOC(type, ptr, n) (M_UNLIKELY ((n) > SIZE_MAX / sizeof(type)) ? NULL : realloc ((ptr), (n)*sizeof (type)))
# define M_MEMORY_FREE(ptr) free(ptr)
#endif
#endif

/* This macro is called on memory allocation failure.
 * By default, it aborts the program execution.
 * NOTE: Can be overloaded by user code.
*/
#ifndef M_MEMORY_FULL
#define M_MEMORY_FULL(size) do {                                              \
    fprintf(stderr, "ERROR(M*LIB): Cannot allocate %zu bytes of memory at (%s:%s:%d).\n", \
            (size_t) (size), __FILE__, __func__, __LINE__);                   \
    abort();                                                                  \
  } while (0)
#endif


/************************************************************/
/*********************  ERROR handling **********************/
/************************************************************/

/* Define the default assertion macro used by M*LIB.
 * By default, it is an encapsulation of CLIB assert.
 * NOTE: Can be overiden by user if it needs to keep finer access 
 * on the assertions.
 */
#ifndef M_ASSERT
#define M_ASSERT(expr) assert(expr)
#endif


/* If within the M*LIB tests, perform additional (potentialy slow) checks
 * By default, it is an encapsulation of CLIB assert.
 * NOTE: Can be overiden by user if it needs to keep finer access 
 * on the assertions.
 */
#ifndef M_ASSERT_SLOW
# if defined(M_USE_ADDITIONAL_CHECKS) && M_USE_ADDITIONAL_CHECKS
#  define M_ASSERT_SLOW(n) assert(n)
# else
#  define M_ASSERT_SLOW(n) (void) 0
# endif
#endif


/* Always perform a runtime check of the given condition
 * NOTE: Can be overiden by user if it needs to keep finer access 
 * on the assertions or display message on another device.
 */
#ifndef M_ASSERT_INIT
#define M_ASSERT_INIT(expr, object) {                                         \
    if (!(expr)) {                                                            \
      fprintf(stderr, "ERROR(M*LIB): Cannot initialize %s at (%s:%s:%d): %s\n", \
              (object), __FILE__, __func__, __LINE__, #expr);                 \
      abort();                                                                \
    } } while (0)
#endif


/* Define an assertion check on an index, compared to its maximum.
 * The index is supposed to be unsigned.
 * It is only used to valid user input, not an intermediary calculus.
 * NOTE: Can be overiden by user if it needs to keep access under control
 * even on release mode */
#ifndef M_ASSERT_INDEX
#define M_ASSERT_INDEX(index, max) do {                                       \
    M_ASSERT((index) < (max));                                                \
  } while (0)
#endif


/* Terminate the compilation of the current unit with an error message.
   The error is classified as error
   with an optional message detailling the error.
   Either use C11 to get a proper message, or at least a good hint in C99.
   error shall be a C name, msg a string.
   Quite usefull to terminate with a proper error message rather than
   a garbage of error due to incorrect code generation in the methods
   expansion.
   NOTE: Some implementation claims to be C11 (default mode) but fails
   to deliver a working assert.h with static_assert.
 */
#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && defined(static_assert)) || defined(__cplusplus)
# define M_STATIC_FAILURE(error, msg) static_assert(false, #error ": " msg);
#else
# define M_STATIC_FAILURE(error, msg) struct error { int error : 0;};
#endif


/* Test at compile time if the given condition is true.
   The error is classidied as error
   with an optional message detailling the error.
   NOTE: Use bitfield to be compatible with most compilers
   (so that it properly displays 'error' on the command line
   NOTE: Cannot use C11 Static Assert as is not usable in expression.
   NOTE: In C++, use of lambda to encapsulate static_assert in
   an expression.
*/
#if defined(__cplusplus)
# define M_STATIC_ASSERT(cond, error, msg)                                    \
  ([] { static_assert(cond, #error ": " msg); } ())
#elif defined(__GNUC__) && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && defined(static_assert)
# define M_STATIC_ASSERT(cond, error, msg)                                    \
  M_ATTR_EXTENSION  ({ static_assert(cond, #error ": " msg); })
#else
# define M_STATIC_ASSERT(cond, error, msg)                                    \
  ((void) sizeof(struct  { int error : !!(cond);}))
#endif


/***************************************************************/
/****************** Preprocessing Times Macro ******************/
/***************************************************************/

/* Maximum number of argument which can be handled by this header.
   Can be increased for future release of this header.
*/
#define M_MAX_NB_ARGUMENT 52

/* Basic handling of concatenation of symbols:
 * M_C, M_C3, M_C4
 */
#define M_C2I(a, ...)       a ## __VA_ARGS__
#define M_C(a, ...)         M_C2I(a, __VA_ARGS__)
#define M_C3I(a, b, ...)    a ## b ## __VA_ARGS__
#define M_C3(a, b, ...)     M_C3I(a ,b, __VA_ARGS__)
#define M_C4I(a, b, c, ...) a ## b ## c ## __VA_ARGS__
#define M_C4(a, b, c, ...)  M_C4I(a ,b, c, __VA_ARGS__)

/* Increment the number given as argument (from [0..52[)
   Generated by:
   for i in $(seq 0 52) ; do printf "#define M_INC_%d %d\n" $i $(($i + 1)) ; done
 */
#define M_INC(x)          M_C(M_INC_, x)
#define M_INC_0 1
#define M_INC_1 2
#define M_INC_2 3
#define M_INC_3 4
#define M_INC_4 5
#define M_INC_5 6
#define M_INC_6 7
#define M_INC_7 8
#define M_INC_8 9
#define M_INC_9 10
#define M_INC_10 11
#define M_INC_11 12
#define M_INC_12 13
#define M_INC_13 14
#define M_INC_14 15
#define M_INC_15 16
#define M_INC_16 17
#define M_INC_17 18
#define M_INC_18 19
#define M_INC_19 20
#define M_INC_20 21
#define M_INC_21 22
#define M_INC_22 23
#define M_INC_23 24
#define M_INC_24 25
#define M_INC_25 26
#define M_INC_26 27
#define M_INC_27 28
#define M_INC_28 29
#define M_INC_29 30
#define M_INC_30 31
#define M_INC_31 32
#define M_INC_32 33
#define M_INC_33 34
#define M_INC_34 35
#define M_INC_35 36
#define M_INC_36 37
#define M_INC_37 38
#define M_INC_38 39
#define M_INC_39 40
#define M_INC_40 41
#define M_INC_41 42
#define M_INC_42 43
#define M_INC_43 44
#define M_INC_44 45
#define M_INC_45 46
#define M_INC_46 47
#define M_INC_47 48
#define M_INC_48 49
#define M_INC_49 50
#define M_INC_50 51
#define M_INC_51 52
#define M_INC_52 53
#define M_INC_53 M_OVERFLOW
#define M_INC_M_OVERFLOW M_OVERFLOW


/* Decrement the number given in argument (from [0..52[) 
   Generated by:
   for i in $(seq 1 52) ; do printf "#define M_DEC_%d %d\n" $i $(($i - 1)) ; done
*/
#define M_DEC(x)          M_C(M_DEC_, x)
#define M_DEC_M_UNDERFLOW M_UNDERFLOW
#define M_DEC_0 M_UNDERFLOW
#define M_DEC_1 0
#define M_DEC_2 1
#define M_DEC_3 2
#define M_DEC_4 3
#define M_DEC_5 4
#define M_DEC_6 5
#define M_DEC_7 6
#define M_DEC_8 7
#define M_DEC_9 8
#define M_DEC_10 9
#define M_DEC_11 10
#define M_DEC_12 11
#define M_DEC_13 12
#define M_DEC_14 13
#define M_DEC_15 14
#define M_DEC_16 15
#define M_DEC_17 16
#define M_DEC_18 17
#define M_DEC_19 18
#define M_DEC_20 19
#define M_DEC_21 20
#define M_DEC_22 21
#define M_DEC_23 22
#define M_DEC_24 23
#define M_DEC_25 24
#define M_DEC_26 25
#define M_DEC_27 26
#define M_DEC_28 27
#define M_DEC_29 28
#define M_DEC_30 29
#define M_DEC_31 30
#define M_DEC_32 31
#define M_DEC_33 32
#define M_DEC_34 33
#define M_DEC_35 34
#define M_DEC_36 35
#define M_DEC_37 36
#define M_DEC_38 37
#define M_DEC_39 38
#define M_DEC_40 39
#define M_DEC_41 40
#define M_DEC_42 41
#define M_DEC_43 42
#define M_DEC_44 43
#define M_DEC_45 44
#define M_DEC_46 45
#define M_DEC_47 46
#define M_DEC_48 47
#define M_DEC_49 48
#define M_DEC_50 49
#define M_DEC_51 50
#define M_DEC_52 51

/* Add two integers.
   Generated by:
   for i in $(seq 0 52) ; do printf "#define M_ADDI_%d(n) " $i ; for j in $(seq 1 $i) ; do printf "M_INC(";done ; printf "n" ; for j in $(seq 1 $i) ; do printf ")" ;done ; printf "\n" ; done */
#define M_ADD(x,y)  M_ADDI(M_C(M_ADDI_, y), x)
#define M_ADDI(f,n) f(n)
#define M_ADDI_0(n) n
#define M_ADDI_1(n) M_INC(n)
#define M_ADDI_2(n) M_INC(M_INC(n))
#define M_ADDI_3(n) M_INC(M_INC(M_INC(n)))
#define M_ADDI_4(n) M_INC(M_INC(M_INC(M_INC(n))))
#define M_ADDI_5(n) M_INC(M_INC(M_INC(M_INC(M_INC(n)))))
#define M_ADDI_6(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n))))))
#define M_ADDI_7(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n)))))))
#define M_ADDI_8(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n))))))))
#define M_ADDI_9(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n)))))))))
#define M_ADDI_10(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n))))))))))
#define M_ADDI_11(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n)))))))))))
#define M_ADDI_12(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n))))))))))))
#define M_ADDI_13(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n)))))))))))))
#define M_ADDI_14(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n))))))))))))))
#define M_ADDI_15(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n)))))))))))))))
#define M_ADDI_16(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n))))))))))))))))
#define M_ADDI_17(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n)))))))))))))))))
#define M_ADDI_18(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n))))))))))))))))))
#define M_ADDI_19(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n)))))))))))))))))))
#define M_ADDI_20(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n))))))))))))))))))))
#define M_ADDI_21(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n)))))))))))))))))))))
#define M_ADDI_22(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n))))))))))))))))))))))
#define M_ADDI_23(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n)))))))))))))))))))))))
#define M_ADDI_24(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n))))))))))))))))))))))))
#define M_ADDI_25(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n)))))))))))))))))))))))))
#define M_ADDI_26(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n))))))))))))))))))))))))))
#define M_ADDI_27(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n)))))))))))))))))))))))))))
#define M_ADDI_28(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n))))))))))))))))))))))))))))
#define M_ADDI_29(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n)))))))))))))))))))))))))))))
#define M_ADDI_30(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n))))))))))))))))))))))))))))))
#define M_ADDI_31(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n)))))))))))))))))))))))))))))))
#define M_ADDI_32(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n))))))))))))))))))))))))))))))))
#define M_ADDI_33(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n)))))))))))))))))))))))))))))))))
#define M_ADDI_34(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n))))))))))))))))))))))))))))))))))
#define M_ADDI_35(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n)))))))))))))))))))))))))))))))))))
#define M_ADDI_36(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n))))))))))))))))))))))))))))))))))))
#define M_ADDI_37(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n)))))))))))))))))))))))))))))))))))))
#define M_ADDI_38(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n))))))))))))))))))))))))))))))))))))))
#define M_ADDI_39(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n)))))))))))))))))))))))))))))))))))))))
#define M_ADDI_40(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n))))))))))))))))))))))))))))))))))))))))
#define M_ADDI_41(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n)))))))))))))))))))))))))))))))))))))))))
#define M_ADDI_42(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n))))))))))))))))))))))))))))))))))))))))))
#define M_ADDI_43(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n)))))))))))))))))))))))))))))))))))))))))))
#define M_ADDI_44(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n))))))))))))))))))))))))))))))))))))))))))))
#define M_ADDI_45(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n)))))))))))))))))))))))))))))))))))))))))))))
#define M_ADDI_46(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n))))))))))))))))))))))))))))))))))))))))))))))
#define M_ADDI_47(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n)))))))))))))))))))))))))))))))))))))))))))))))
#define M_ADDI_48(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n))))))))))))))))))))))))))))))))))))))))))))))))
#define M_ADDI_49(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n)))))))))))))))))))))))))))))))))))))))))))))))))
#define M_ADDI_50(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n))))))))))))))))))))))))))))))))))))))))))))))))))
#define M_ADDI_51(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n)))))))))))))))))))))))))))))))))))))))))))))))))))
#define M_ADDI_52(n) M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(M_INC(n))))))))))))))))))))))))))))))))))))))))))))))))))))

/* Substract two integers.
   Generated by:
   for i in $(seq 0 52) ; do printf "#define M_SUBI_%d(n) " $i ; for j in $(seq 1 $i) ; do printf "M_DEC(";done ; printf "n" ; for j in $(seq 1 $i) ; do printf ")" ;done ; printf "\n" ; done */
#define M_SUB(x,y)  M_SUBI(M_C(M_SUBI_, y), x)
#define M_SUBI(f,n) f(n)
#define M_SUBI_0(n) n
#define M_SUBI_1(n) M_DEC(n)
#define M_SUBI_2(n) M_DEC(M_DEC(n))
#define M_SUBI_3(n) M_DEC(M_DEC(M_DEC(n)))
#define M_SUBI_4(n) M_DEC(M_DEC(M_DEC(M_DEC(n))))
#define M_SUBI_5(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n)))))
#define M_SUBI_6(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n))))))
#define M_SUBI_7(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n)))))))
#define M_SUBI_8(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n))))))))
#define M_SUBI_9(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n)))))))))
#define M_SUBI_10(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n))))))))))
#define M_SUBI_11(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n)))))))))))
#define M_SUBI_12(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n))))))))))))
#define M_SUBI_13(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n)))))))))))))
#define M_SUBI_14(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n))))))))))))))
#define M_SUBI_15(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n)))))))))))))))
#define M_SUBI_16(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n))))))))))))))))
#define M_SUBI_17(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n)))))))))))))))))
#define M_SUBI_18(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n))))))))))))))))))
#define M_SUBI_19(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n)))))))))))))))))))
#define M_SUBI_20(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n))))))))))))))))))))
#define M_SUBI_21(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n)))))))))))))))))))))
#define M_SUBI_22(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n))))))))))))))))))))))
#define M_SUBI_23(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n)))))))))))))))))))))))
#define M_SUBI_24(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n))))))))))))))))))))))))
#define M_SUBI_25(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n)))))))))))))))))))))))))
#define M_SUBI_26(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n))))))))))))))))))))))))))
#define M_SUBI_27(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n)))))))))))))))))))))))))))
#define M_SUBI_28(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n))))))))))))))))))))))))))))
#define M_SUBI_29(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n)))))))))))))))))))))))))))))
#define M_SUBI_30(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n))))))))))))))))))))))))))))))
#define M_SUBI_31(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n)))))))))))))))))))))))))))))))
#define M_SUBI_32(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n))))))))))))))))))))))))))))))))
#define M_SUBI_33(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n)))))))))))))))))))))))))))))))))
#define M_SUBI_34(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n))))))))))))))))))))))))))))))))))
#define M_SUBI_35(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n)))))))))))))))))))))))))))))))))))
#define M_SUBI_36(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n))))))))))))))))))))))))))))))))))))
#define M_SUBI_37(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n)))))))))))))))))))))))))))))))))))))
#define M_SUBI_38(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n))))))))))))))))))))))))))))))))))))))
#define M_SUBI_39(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n)))))))))))))))))))))))))))))))))))))))
#define M_SUBI_40(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n))))))))))))))))))))))))))))))))))))))))
#define M_SUBI_41(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n)))))))))))))))))))))))))))))))))))))))))
#define M_SUBI_42(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n))))))))))))))))))))))))))))))))))))))))))
#define M_SUBI_43(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n)))))))))))))))))))))))))))))))))))))))))))
#define M_SUBI_44(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n))))))))))))))))))))))))))))))))))))))))))))
#define M_SUBI_45(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n)))))))))))))))))))))))))))))))))))))))))))))
#define M_SUBI_46(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n))))))))))))))))))))))))))))))))))))))))))))))
#define M_SUBI_47(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n)))))))))))))))))))))))))))))))))))))))))))))))
#define M_SUBI_48(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n))))))))))))))))))))))))))))))))))))))))))))))))
#define M_SUBI_49(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n)))))))))))))))))))))))))))))))))))))))))))))))))
#define M_SUBI_50(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n))))))))))))))))))))))))))))))))))))))))))))))))))
#define M_SUBI_51(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n)))))))))))))))))))))))))))))))))))))))))))))))))))
#define M_SUBI_52(n) M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(M_DEC(n))))))))))))))))))))))))))))))))))))))))))))))))))))

/* Return the nth argument (either variable or static).
   Generated by:
   for i in $(seq 1 53 ) ; do printf "#define M_RETI_ARG%d(" $i; for j in $(seq 1 $i) ; do printf "_%d, " $j ; done ; printf "...) _%d\n#define M_RET_ARG%d(...) M_RETI_ARG%d(__VA_ARGS__,)\n\n" $i $i $i ; done
  */
#define M_RETI_ARG(f, ...) f(__VA_ARGS__)
#define M_RET_ARG(n, ...) M_RETI_ARG(M_C(M_RET_ARG, n), __VA_ARGS__)

#define M_RETI_ARG1(_1, ...) _1
#define M_RET_ARG1(...) M_RETI_ARG1(__VA_ARGS__,)

#define M_RETI_ARG2(_1, _2, ...) _2
#define M_RET_ARG2(...) M_RETI_ARG2(__VA_ARGS__,)

#define M_RETI_ARG3(_1, _2, _3, ...) _3
#define M_RET_ARG3(...) M_RETI_ARG3(__VA_ARGS__,)

#define M_RETI_ARG4(_1, _2, _3, _4, ...) _4
#define M_RET_ARG4(...) M_RETI_ARG4(__VA_ARGS__,)

#define M_RETI_ARG5(_1, _2, _3, _4, _5, ...) _5
#define M_RET_ARG5(...) M_RETI_ARG5(__VA_ARGS__,)

#define M_RETI_ARG6(_1, _2, _3, _4, _5, _6, ...) _6
#define M_RET_ARG6(...) M_RETI_ARG6(__VA_ARGS__,)

#define M_RETI_ARG7(_1, _2, _3, _4, _5, _6, _7, ...) _7
#define M_RET_ARG7(...) M_RETI_ARG7(__VA_ARGS__,)

#define M_RETI_ARG8(_1, _2, _3, _4, _5, _6, _7, _8, ...) _8
#define M_RET_ARG8(...) M_RETI_ARG8(__VA_ARGS__,)

#define M_RETI_ARG9(_1, _2, _3, _4, _5, _6, _7, _8, _9, ...) _9
#define M_RET_ARG9(...) M_RETI_ARG9(__VA_ARGS__,)

#define M_RETI_ARG10(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, ...) _10
#define M_RET_ARG10(...) M_RETI_ARG10(__VA_ARGS__,)

#define M_RETI_ARG11(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, ...) _11
#define M_RET_ARG11(...) M_RETI_ARG11(__VA_ARGS__,)

#define M_RETI_ARG12(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, ...) _12
#define M_RET_ARG12(...) M_RETI_ARG12(__VA_ARGS__,)

#define M_RETI_ARG13(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, ...) _13
#define M_RET_ARG13(...) M_RETI_ARG13(__VA_ARGS__,)

#define M_RETI_ARG14(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, ...) _14
#define M_RET_ARG14(...) M_RETI_ARG14(__VA_ARGS__,)

#define M_RETI_ARG15(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, ...) _15
#define M_RET_ARG15(...) M_RETI_ARG15(__VA_ARGS__,)

#define M_RETI_ARG16(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, ...) _16
#define M_RET_ARG16(...) M_RETI_ARG16(__VA_ARGS__,)

#define M_RETI_ARG17(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, ...) _17
#define M_RET_ARG17(...) M_RETI_ARG17(__VA_ARGS__,)

#define M_RETI_ARG18(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, ...) _18
#define M_RET_ARG18(...) M_RETI_ARG18(__VA_ARGS__,)

#define M_RETI_ARG19(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...) _19
#define M_RET_ARG19(...) M_RETI_ARG19(__VA_ARGS__,)

#define M_RETI_ARG20(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, ...) _20
#define M_RET_ARG20(...) M_RETI_ARG20(__VA_ARGS__,)

#define M_RETI_ARG21(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, ...) _21
#define M_RET_ARG21(...) M_RETI_ARG21(__VA_ARGS__,)

#define M_RETI_ARG22(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, ...) _22
#define M_RET_ARG22(...) M_RETI_ARG22(__VA_ARGS__,)

#define M_RETI_ARG23(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, ...) _23
#define M_RET_ARG23(...) M_RETI_ARG23(__VA_ARGS__,)

#define M_RETI_ARG24(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, ...) _24
#define M_RET_ARG24(...) M_RETI_ARG24(__VA_ARGS__,)

#define M_RETI_ARG25(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, ...) _25
#define M_RET_ARG25(...) M_RETI_ARG25(__VA_ARGS__,)

#define M_RETI_ARG26(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, ...) _26
#define M_RET_ARG26(...) M_RETI_ARG26(__VA_ARGS__,)

#define M_RETI_ARG27(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, ...) _27
#define M_RET_ARG27(...) M_RETI_ARG27(__VA_ARGS__,)

#define M_RETI_ARG28(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, ...) _28
#define M_RET_ARG28(...) M_RETI_ARG28(__VA_ARGS__,)

#define M_RETI_ARG29(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, ...) _29
#define M_RET_ARG29(...) M_RETI_ARG29(__VA_ARGS__,)

#define M_RETI_ARG30(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, ...) _30
#define M_RET_ARG30(...) M_RETI_ARG30(__VA_ARGS__,)

#define M_RETI_ARG31(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, ...) _31
#define M_RET_ARG31(...) M_RETI_ARG31(__VA_ARGS__,)

#define M_RETI_ARG32(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, ...) _32
#define M_RET_ARG32(...) M_RETI_ARG32(__VA_ARGS__,)

#define M_RETI_ARG33(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, ...) _33
#define M_RET_ARG33(...) M_RETI_ARG33(__VA_ARGS__,)

#define M_RETI_ARG34(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, ...) _34
#define M_RET_ARG34(...) M_RETI_ARG34(__VA_ARGS__,)

#define M_RETI_ARG35(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, ...) _35
#define M_RET_ARG35(...) M_RETI_ARG35(__VA_ARGS__,)

#define M_RETI_ARG36(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, ...) _36
#define M_RET_ARG36(...) M_RETI_ARG36(__VA_ARGS__,)

#define M_RETI_ARG37(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, ...) _37
#define M_RET_ARG37(...) M_RETI_ARG37(__VA_ARGS__,)

#define M_RETI_ARG38(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, ...) _38
#define M_RET_ARG38(...) M_RETI_ARG38(__VA_ARGS__,)

#define M_RETI_ARG39(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, ...) _39
#define M_RET_ARG39(...) M_RETI_ARG39(__VA_ARGS__,)

#define M_RETI_ARG40(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, ...) _40
#define M_RET_ARG40(...) M_RETI_ARG40(__VA_ARGS__,)

#define M_RETI_ARG41(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, ...) _41
#define M_RET_ARG41(...) M_RETI_ARG41(__VA_ARGS__,)

#define M_RETI_ARG42(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, ...) _42
#define M_RET_ARG42(...) M_RETI_ARG42(__VA_ARGS__,)

#define M_RETI_ARG43(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, ...) _43
#define M_RET_ARG43(...) M_RETI_ARG43(__VA_ARGS__,)

#define M_RETI_ARG44(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, ...) _44
#define M_RET_ARG44(...) M_RETI_ARG44(__VA_ARGS__,)

#define M_RETI_ARG45(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, ...) _45
#define M_RET_ARG45(...) M_RETI_ARG45(__VA_ARGS__,)

#define M_RETI_ARG46(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, ...) _46
#define M_RET_ARG46(...) M_RETI_ARG46(__VA_ARGS__,)

#define M_RETI_ARG47(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, ...) _47
#define M_RET_ARG47(...) M_RETI_ARG47(__VA_ARGS__,)

#define M_RETI_ARG48(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, ...) _48
#define M_RET_ARG48(...) M_RETI_ARG48(__VA_ARGS__,)

#define M_RETI_ARG49(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, ...) _49
#define M_RET_ARG49(...) M_RETI_ARG49(__VA_ARGS__,)

#define M_RETI_ARG50(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, ...) _50
#define M_RET_ARG50(...) M_RETI_ARG50(__VA_ARGS__,)

#define M_RETI_ARG51(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, ...) _51
#define M_RET_ARG51(...) M_RETI_ARG51(__VA_ARGS__,)

#define M_RETI_ARG52(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, ...) _52
#define M_RET_ARG52(...) M_RETI_ARG52(__VA_ARGS__,)

#define M_RETI_ARG53(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, ...) _53
#define M_RET_ARG53(...) M_RETI_ARG53(__VA_ARGS__,)

#define M_RETI_ARG54(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, ...) _54
#define M_RET_ARG54(...) M_RETI_ARG54(__VA_ARGS__,)

#define M_RETI_ARG55(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, ...) _55
#define M_RET_ARG55(...) M_RETI_ARG55(__VA_ARGS__,)

#define M_RETI_ARG56(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, ...) _56
#define M_RET_ARG56(...) M_RETI_ARG56(__VA_ARGS__,)

#define M_RETI_ARG57(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, ...) _57
#define M_RET_ARG57(...) M_RETI_ARG57(__VA_ARGS__,)

#define M_RETI_ARG58(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, ...) _58
#define M_RET_ARG58(...) M_RETI_ARG58(__VA_ARGS__,)

#define M_RETI_ARG59(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, ...) _59
#define M_RET_ARG59(...) M_RETI_ARG59(__VA_ARGS__,)

#define M_RETI_ARG60(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, ...) _60
#define M_RET_ARG60(...) M_RETI_ARG60(__VA_ARGS__,)

#define M_RETI_ARG61(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, ...) _61
#define M_RET_ARG61(...) M_RETI_ARG61(__VA_ARGS__,)

#define M_RETI_ARG62(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, ...) _62
#define M_RET_ARG62(...) M_RETI_ARG62(__VA_ARGS__,)

#define M_RETI_ARG63(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, ...) _63
#define M_RET_ARG63(...) M_RETI_ARG63(__VA_ARGS__,)

#define M_RETI_ARG64(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, ...) _64
#define M_RET_ARG64(...) M_RETI_ARG64(__VA_ARGS__,)

#define M_RETI_ARG65(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, ...) _65
#define M_RET_ARG65(...) M_RETI_ARG65(__VA_ARGS__,)

#define M_RETI_ARG66(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, ...) _66
#define M_RET_ARG66(...) M_RETI_ARG66(__VA_ARGS__,)

#define M_RETI_ARG67(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, ...) _67
#define M_RET_ARG67(...) M_RETI_ARG67(__VA_ARGS__,)

#define M_RETI_ARG68(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, ...) _68
#define M_RET_ARG68(...) M_RETI_ARG68(__VA_ARGS__,)

#define M_RETI_ARG69(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, ...) _69
#define M_RET_ARG69(...) M_RETI_ARG69(__VA_ARGS__,)

#define M_RETI_ARG70(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, ...) _70
#define M_RET_ARG70(...) M_RETI_ARG70(__VA_ARGS__,)

#define M_RETI_ARG71(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, ...) _71
#define M_RET_ARG71(...) M_RETI_ARG71(__VA_ARGS__,)

#define M_RETI_ARG72(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, ...) _72
#define M_RET_ARG72(...) M_RETI_ARG72(__VA_ARGS__,)

#define M_RETI_ARG73(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, ...) _73
#define M_RET_ARG73(...) M_RETI_ARG73(__VA_ARGS__,)

#define M_RETI_ARG74(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, ...) _74
#define M_RET_ARG74(...) M_RETI_ARG74(__VA_ARGS__,)

#define M_RETI_ARG75(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, ...) _75
#define M_RET_ARG75(...) M_RETI_ARG75(__VA_ARGS__,)

#define M_RETI_ARG76(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, ...) _76
#define M_RET_ARG76(...) M_RETI_ARG76(__VA_ARGS__,)

/* For a pair of argument (arg0, arg1), return either the first or the second.
   Needed in case where M_RET_ARGn cannot be used */
#define M_PAIR_1(a,b) a
#define M_PAIR_2(a,b) b

/* Skip the Nth first arguments
   Generated by:
   for i in $(seq 0 52) ; do printf "#define M_SKIPI_%d(" $i ; for j in $(seq 1 $i) ; do printf "_%d, " $j ; done ; printf "...) __VA_ARGS__\n"; done
 */
#define M_SKIPI(f, ...) f(__VA_ARGS__)
#define M_SKIP_ARGS(n, ...)  M_SKIPI(M_C(M_SKIPI_, n), __VA_ARGS__)

#define M_SKIPI_0(...) __VA_ARGS__
#define M_SKIPI_1(_1, ...) __VA_ARGS__
#define M_SKIPI_2(_1, _2, ...) __VA_ARGS__
#define M_SKIPI_3(_1, _2, _3, ...) __VA_ARGS__
#define M_SKIPI_4(_1, _2, _3, _4, ...) __VA_ARGS__
#define M_SKIPI_5(_1, _2, _3, _4, _5, ...) __VA_ARGS__
#define M_SKIPI_6(_1, _2, _3, _4, _5, _6, ...) __VA_ARGS__
#define M_SKIPI_7(_1, _2, _3, _4, _5, _6, _7, ...) __VA_ARGS__
#define M_SKIPI_8(_1, _2, _3, _4, _5, _6, _7, _8, ...) __VA_ARGS__
#define M_SKIPI_9(_1, _2, _3, _4, _5, _6, _7, _8, _9, ...) __VA_ARGS__
#define M_SKIPI_10(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, ...) __VA_ARGS__
#define M_SKIPI_11(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, ...) __VA_ARGS__
#define M_SKIPI_12(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, ...) __VA_ARGS__
#define M_SKIPI_13(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, ...) __VA_ARGS__
#define M_SKIPI_14(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, ...) __VA_ARGS__
#define M_SKIPI_15(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, ...) __VA_ARGS__
#define M_SKIPI_16(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, ...) __VA_ARGS__
#define M_SKIPI_17(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, ...) __VA_ARGS__
#define M_SKIPI_18(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, ...) __VA_ARGS__
#define M_SKIPI_19(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...) __VA_ARGS__
#define M_SKIPI_20(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, ...) __VA_ARGS__
#define M_SKIPI_21(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, ...) __VA_ARGS__
#define M_SKIPI_22(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, ...) __VA_ARGS__
#define M_SKIPI_23(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, ...) __VA_ARGS__
#define M_SKIPI_24(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, ...) __VA_ARGS__
#define M_SKIPI_25(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, ...) __VA_ARGS__
#define M_SKIPI_26(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, ...) __VA_ARGS__
#define M_SKIPI_27(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, ...) __VA_ARGS__
#define M_SKIPI_28(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, ...) __VA_ARGS__
#define M_SKIPI_29(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, ...) __VA_ARGS__
#define M_SKIPI_30(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, ...) __VA_ARGS__
#define M_SKIPI_31(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, ...) __VA_ARGS__
#define M_SKIPI_32(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, ...) __VA_ARGS__
#define M_SKIPI_33(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, ...) __VA_ARGS__
#define M_SKIPI_34(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, ...) __VA_ARGS__
#define M_SKIPI_35(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, ...) __VA_ARGS__
#define M_SKIPI_36(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, ...) __VA_ARGS__
#define M_SKIPI_37(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, ...) __VA_ARGS__
#define M_SKIPI_38(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, ...) __VA_ARGS__
#define M_SKIPI_39(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, ...) __VA_ARGS__
#define M_SKIPI_40(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, ...) __VA_ARGS__
#define M_SKIPI_41(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, ...) __VA_ARGS__
#define M_SKIPI_42(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, ...) __VA_ARGS__
#define M_SKIPI_43(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, ...) __VA_ARGS__
#define M_SKIPI_44(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, ...) __VA_ARGS__
#define M_SKIPI_45(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, ...) __VA_ARGS__
#define M_SKIPI_46(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, ...) __VA_ARGS__
#define M_SKIPI_47(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, ...) __VA_ARGS__
#define M_SKIPI_48(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, ...) __VA_ARGS__
#define M_SKIPI_49(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, ...) __VA_ARGS__
#define M_SKIPI_50(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, ...) __VA_ARGS__
#define M_SKIPI_51(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, ...) __VA_ARGS__
#define M_SKIPI_52(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, ...) __VA_ARGS__



/* Keep the nth first arguments.
   Generated by:
   for i in $(seq 0 52) ; do printf "#define M_KEEPI_%d(" $i ; for j in $(seq 1 $i) ; do printf "_%d, " $j ; done ; printf "...) " ; for j in $(seq 1 $(($i - 1))) ; do printf "_%d, " $j; done ; if test "$i" -ne 0 ; then printf "_$i" ; fi ; printf "\n" ; done */
#define M_KEEPI(f, ...) f(__VA_ARGS__)
#define M_KEEP_ARGS(n, ...)  M_KEEPI(M_C(M_KEEPI_, n), __VA_ARGS__)

#define M_KEEPI_0(...) 
#define M_KEEPI_1(_1, ...) _1
#define M_KEEPI_2(_1, _2, ...) _1, _2
#define M_KEEPI_3(_1, _2, _3, ...) _1, _2, _3
#define M_KEEPI_4(_1, _2, _3, _4, ...) _1, _2, _3, _4
#define M_KEEPI_5(_1, _2, _3, _4, _5, ...) _1, _2, _3, _4, _5
#define M_KEEPI_6(_1, _2, _3, _4, _5, _6, ...) _1, _2, _3, _4, _5, _6
#define M_KEEPI_7(_1, _2, _3, _4, _5, _6, _7, ...) _1, _2, _3, _4, _5, _6, _7
#define M_KEEPI_8(_1, _2, _3, _4, _5, _6, _7, _8, ...) _1, _2, _3, _4, _5, _6, _7, _8
#define M_KEEPI_9(_1, _2, _3, _4, _5, _6, _7, _8, _9, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9
#define M_KEEPI_10(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10
#define M_KEEPI_11(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11
#define M_KEEPI_12(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12
#define M_KEEPI_13(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13
#define M_KEEPI_14(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14
#define M_KEEPI_15(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15
#define M_KEEPI_16(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16
#define M_KEEPI_17(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17
#define M_KEEPI_18(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18
#define M_KEEPI_19(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19
#define M_KEEPI_20(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20
#define M_KEEPI_21(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21
#define M_KEEPI_22(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22
#define M_KEEPI_23(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23
#define M_KEEPI_24(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24
#define M_KEEPI_25(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25
#define M_KEEPI_26(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26
#define M_KEEPI_27(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27
#define M_KEEPI_28(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28
#define M_KEEPI_29(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29
#define M_KEEPI_30(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30
#define M_KEEPI_31(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31
#define M_KEEPI_32(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32
#define M_KEEPI_33(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33
#define M_KEEPI_34(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34
#define M_KEEPI_35(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35
#define M_KEEPI_36(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36
#define M_KEEPI_37(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37
#define M_KEEPI_38(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38
#define M_KEEPI_39(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39
#define M_KEEPI_40(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40
#define M_KEEPI_41(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41
#define M_KEEPI_42(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42
#define M_KEEPI_43(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43
#define M_KEEPI_44(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44
#define M_KEEPI_45(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45
#define M_KEEPI_46(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46
#define M_KEEPI_47(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47
#define M_KEEPI_48(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48
#define M_KEEPI_49(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49
#define M_KEEPI_50(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50
#define M_KEEPI_51(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51
#define M_KEEPI_52(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, ...) _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52

/* Keep the medium characters of a sequence */
#define M_MID_ARGS(first, len, ...) M_MIDI(M_KEEP_ARGS,len, M_SKIP_ARGS(first, __VA_ARGS__))
#define M_MIDI(f, ...)              f(__VA_ARGS__)


/* Return the value of the "array" associated to the given index (zero based).
   EXAMPLE: M_GET_AT((f_0,f_1,f_2),1) returns f_1
   Can be chained with NARGS().
 */
#define M_GET_AT(array, index)      M_GETI_AT0(M_C(M_RET_ARG, M_INC(index)), array)
#define M_GETI_AT0(func, array)     func ( M_ID array , dummy )


/* Convert an integer or a symbol into 0 (if 0) or 1 (if not 0).
   1 if symbol unknown */
#define M_TOBOOLI_0                 1, 0,
#define M_BOOL(x)                   M_RET_ARG2(M_C(M_TOBOOLI_, x), 1, useless)

/* Inverse 0 into 1 and 1 into 0 */
#define M_INVI_0                    1
#define M_INVI_1                    0
#define M_INV(x)                    M_C(M_INVI_, x)

/* Perform a AND between the boolean inputs */
#define M_ANDI_00                   0
#define M_ANDI_01                   0
#define M_ANDI_10                   0
#define M_ANDI_11                   1
#define M_AND(x,y)                  M_C3(M_ANDI_, x, y)

/* Perform a OR between the boolean inputs */
#define M_ORI_00                    0
#define M_ORI_01                    1
#define M_ORI_10                    1
#define M_ORI_11                    1
#define M_OR(x,y)                   M_C3(M_ORI_, x, y)


/* M_IF Macro : Perform an IF test at preprocessing time. 
   The condition is assumed to be true if unkown.
   USAGE
   M_IF(condition)(Block if true, Block if false)
   Example: M_IF(0)(true_action, false_action) --> false_action */
#define M_IFI_0(true_c, ...)        __VA_ARGS__
#define M_IFI_1(true_c, ...)        true_c
#define M_IF(c)                     M_C(M_IFI_, M_BOOL(c))

/* Return 1 if there is a comma inside the argument list,
   0 otherwise. */
#if defined(__clang__) && defined(_MSC_VER)
// CLANG on windows has a non compliant preprocessor. Workaround it (with issue)
#define M_COMMA_P(...)              M_RET_ARG76(__VA_ARGS__, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, useless)
#define M_COMMA_P_WORKAROUND 1
#else
#define M_COMMA_P(...)              M_RETI_ARG76(__VA_ARGS__, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, useless)
#endif

/* Return the string representation of the evaluated x. */
#define M_AS_STR(x)                 M_AS_STRI(x)
#define M_AS_STRI(x)                #x

/* Return 1 if the argument is empty (aka ''), 0 otherwise.
   Handle: EMPTY_P(), EMPTY_P(x), EMPTY_P(()) and EMPTY_P(,) and EMPTY_P(f) with #define f() 2,3 */
#define M_EMPTYI_DETECT(...)        ,
#define M_EMPTYI_P_C1(...)          M_COMMA_P(M_EMPTYI_DETECT __VA_ARGS__ () )
#define M_EMPTYI_P_C2(...)          M_COMMA_P(M_EMPTYI_DETECT __VA_ARGS__)
#define M_EMPTYI_P_C3(...)          M_COMMA_P(__VA_ARGS__ () )
#define M_EMPTY_P(...)              M_AND(M_EMPTYI_P_C1(__VA_ARGS__), M_INV(M_OR(M_OR(M_EMPTYI_P_C2(__VA_ARGS__), M_COMMA_P(__VA_ARGS__)),M_EMPTYI_P_C3(__VA_ARGS__))))

/* Generate a comma later in the next evaluation pass. */
#define M_DEFERRED_COMMA            ,

/* M_IF macro for empty arguments:
    M_IF_EMPTY(arguments)(action if empty, action if not empty) */
#define M_IF_EMPTY(...)             M_IF(M_EMPTY_P(__VA_ARGS__))

/* Return 1 if argument is "()" or "(x)"
 * Test if () or (x) can be transformed into a comma,
 * and that there is no (toplevel) comma
 * and that there is nothing beyond the parenthesis.
 */
#define M_PARENTHESISI_DETECT1(...)  ,
#define M_PARENTHESISI_DETECT2(...)
#define M_PARENTHESIS_P(...)                                                  \
  M_AND(M_AND(M_COMMA_P(M_PARENTHESISI_DETECT1 __VA_ARGS__), M_INV(M_COMMA_P(__VA_ARGS__))), \
        M_EMPTY_P(M_PARENTHESISI_DETECT2 __VA_ARGS__))


/* Return 1 if argument type or variable or function is the keyword.
    - refFunc is the reference keyword.
    It shall be either 'and', 'or', 'sum' (equivalent to 'add'), 'bool'
    - testedFunc is the tested keyword.
    It cannot start with a special character */
#define M_KEYWORD_P(refFunc, testedFunc)                                      \
  M_COMMA_P(M_C4(M_PATTERN_, refFunc, _, testedFunc))
/* The different special recognized patterns: */
#define M_PATTERN_and_and ,
#define M_PATTERN_or_or ,
#define M_PATTERN_sum_sum ,
#define M_PATTERN_sum_add ,
#define M_PATTERN_add_add ,
#define M_PATTERN_add_sum ,
#define M_PATTERN_product_product ,
#define M_PATTERN_product_mul ,
#define M_PATTERN_bool_bool ,
#define M_PATTERN_char_char ,
#define M_PATTERN_short_short ,
#define M_PATTERN_int_int ,
#define M_PATTERN_long_long ,
#define M_PATTERN_float_float ,
#define M_PATTERN_double_double ,
#define M_PATTERN_void_void ,
#define M_PATTERN_TYPE_TYPE ,
#define M_PATTERN_SUBTYPE_SUBTYPE ,
#define M_PATTERN_IT_TYPE_IT_TYPE ,
#define M_PATTERN_M_UNDERFLOW_M_UNDERFLOW ,
#define M_PATTERN_M_OVERFLOW_M_OVERFLOW ,
#define M_PATTERN_M_OVERFLOW_M_OVERFLOW ,
#define M_PATTERN_SEQUENCE_SEQUENCE ,
#define M_PATTERN_MAP_MAP ,
#define M_PATTERN_KEYVAL_KEYVAL ,
#define M_PATTERN_KEYVAL_PTR_KEYVAL_PTR ,
#define M_PATTERN_priority_priority ,

/* Necessary macros to handle recursivity,
   delaying the evaluation by one (or more) level of macro expansion.
   The argument is a macro-function which has to be deferred */
#define M_DELAY0()
#define M_DELAY1(...)               __VA_ARGS__ M_DELAY0 ()
#define M_DELAY2(...)               __VA_ARGS__ M_DELAY1 (M_DELAY0) ()
#define M_DELAY3(...)               __VA_ARGS__ M_DELAY2 (M_DELAY0) ()
#define M_DELAY4(...)               __VA_ARGS__ M_DELAY3 (M_DELAY0) ()
#define M_DELAY5(...)               __VA_ARGS__ M_DELAY4 (M_DELAY0) ()
#define M_DELAY6(...)               __VA_ARGS__ M_DELAY5 (M_DELAY0) ()
#define M_DELAY7(...)               __VA_ARGS__ M_DELAY6 (M_DELAY0) ()
#define M_DELAY8(...)               __VA_ARGS__ M_DELAY7 (M_DELAY0) ()
#define M_DELAY9(...)               __VA_ARGS__ M_DELAY8 (M_DELAY0) ()

/* Perform 3^5 evaluation,
   forcing the pre-processor to expand the given macro a lot of times. 
   NOTE: There can be only one EVAL macro per complete macro-evaluation pass.
   As such the given macro cannot contain M_EVAL itself,
   limiting a lot the usefullness of this macro.
   Moreover, it is quite slow.
*/
#define M_EVAL(...)                 M_EVAL1(M_EVAL1(M_EVAL1(__VA_ARGS__)))
#define M_EVAL1(...)                M_EVAL2(M_EVAL2(M_EVAL2(__VA_ARGS__)))
#define M_EVAL2(...)                M_EVAL3(M_EVAL3(M_EVAL3(__VA_ARGS__)))
#define M_EVAL3(...)                M_EVAL4(M_EVAL4(M_EVAL4(__VA_ARGS__)))
#define M_EVAL4(...)                M_EVAL0(M_EVAL0(M_EVAL0(__VA_ARGS__)))
#define M_EVAL0(...)                __VA_ARGS__

/* Apply Macro :
   It allows that a won't be evaluated until all arguments of 'a' have been already evaluated */
#define M_APPLY(a, ...)             a (__VA_ARGS__)

/* MAP: apply the given macro to all arguments.
   It is a non recursive version that is much faster than the recursive one.
   Example: 
   M_MAP(f,a, b, c) ==> f(a) f(b) f(c) */
/* Generated by:
   for i in $(seq 1 52) ; do printf "#define M_MAP_%d(f" $i ; for j in $(seq 1 $i) ; do printf ", _%d" $j ; done ; printf ") " ; for j in $(seq 1 $i) ; do printf "f(_%d) " $j ; done ; printf "\n"; done
*/
#define M_MAP(f, ...)  M_MAPI(M_C(M_MAP_, M_NARGS(__VA_ARGS__)), f, __VA_ARGS__)
#define M_MAPI(F,...)  F (__VA_ARGS__)

#define M_MAP_1(f, _1) f(_1) 
#define M_MAP_2(f, _1, _2) f(_1) f(_2) 
#define M_MAP_3(f, _1, _2, _3) f(_1) f(_2) f(_3) 
#define M_MAP_4(f, _1, _2, _3, _4) f(_1) f(_2) f(_3) f(_4) 
#define M_MAP_5(f, _1, _2, _3, _4, _5) f(_1) f(_2) f(_3) f(_4) f(_5) 
#define M_MAP_6(f, _1, _2, _3, _4, _5, _6) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) 
#define M_MAP_7(f, _1, _2, _3, _4, _5, _6, _7) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) 
#define M_MAP_8(f, _1, _2, _3, _4, _5, _6, _7, _8) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) 
#define M_MAP_9(f, _1, _2, _3, _4, _5, _6, _7, _8, _9) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) 
#define M_MAP_10(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) 
#define M_MAP_11(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) 
#define M_MAP_12(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) 
#define M_MAP_13(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) 
#define M_MAP_14(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) 
#define M_MAP_15(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) 
#define M_MAP_16(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) 
#define M_MAP_17(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) 
#define M_MAP_18(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) 
#define M_MAP_19(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) 
#define M_MAP_20(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) 
#define M_MAP_21(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) 
#define M_MAP_22(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) 
#define M_MAP_23(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) 
#define M_MAP_24(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) 
#define M_MAP_25(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) 
#define M_MAP_26(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) 
#define M_MAP_27(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) 
#define M_MAP_28(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) 
#define M_MAP_29(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) 
#define M_MAP_30(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) 
#define M_MAP_31(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) 
#define M_MAP_32(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) 
#define M_MAP_33(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) 
#define M_MAP_34(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) 
#define M_MAP_35(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) 
#define M_MAP_36(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) 
#define M_MAP_37(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) 
#define M_MAP_38(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) 
#define M_MAP_39(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) 
#define M_MAP_40(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) 
#define M_MAP_41(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) 
#define M_MAP_42(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) 
#define M_MAP_43(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) 
#define M_MAP_44(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) 
#define M_MAP_45(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) 
#define M_MAP_46(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) 
#define M_MAP_47(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) 
#define M_MAP_48(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) 
#define M_MAP_49(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) 
#define M_MAP_50(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) 
#define M_MAP_51(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) 
#define M_MAP_52(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) 


/* Map a macro to all given arguments with one additional fixed data (non recursive version) */
/* Example: M_MAP2(f, data, a, b, c) ==> f(data,a) f(data,b) f(data,c) */
#define M_MAP2(f, d, ...)  M_MAP2I(M_C(M_MAP2B_, M_NARGS(__VA_ARGS__)), f, d, __VA_ARGS__)
#define M_MAP2I(F,...)     F (__VA_ARGS__)

/* Duplicate for GET_METHOD as it may be called in context where a M_MAP2 is in progress (for oplist).
   NOTE: Increase number of arguments to 76 due to the number of available methods.
   NOTE: Rewrite with another approach much more verbose but also much faster for the compiler.
   This kind of approach seems ugly, but is in fact the fast one.
   NOTE: Do not call recursively the macro (example: M_MAP2B_10 expands f(d,e) then call M_MAP2B_9 on what remains to break as soon as possible the macro "chain".
   Generated by the following command:
   for i in $(seq 1 52) ; do printf "#define M_MAP2B_%d(f, d" $i ; for j in $(seq 1 $i) ; do printf ", _%d" $j ; done ; printf ") " ; for j in $(seq 1 $i) ; do printf "f(d, _%d) " $j ; done ; printf "\n"; done
*/
#define M_MAP2B(f, d, ...)  M_MAP2BI(M_C(M_MAP2B_, M_NARGS(__VA_ARGS__)), f, d, __VA_ARGS__)
#define M_MAP2BI(F,...)     F (__VA_ARGS__)

#define M_MAP2B_1(f, d, _1) f(d, _1) 
#define M_MAP2B_2(f, d, _1, _2) f(d, _1) f(d, _2) 
#define M_MAP2B_3(f, d, _1, _2, _3) f(d, _1) f(d, _2) f(d, _3) 
#define M_MAP2B_4(f, d, _1, _2, _3, _4) f(d, _1) f(d, _2) f(d, _3) f(d, _4) 
#define M_MAP2B_5(f, d, _1, _2, _3, _4, _5) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) 
#define M_MAP2B_6(f, d, _1, _2, _3, _4, _5, _6) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) 
#define M_MAP2B_7(f, d, _1, _2, _3, _4, _5, _6, _7) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) 
#define M_MAP2B_8(f, d, _1, _2, _3, _4, _5, _6, _7, _8) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) 
#define M_MAP2B_9(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) 
#define M_MAP2B_10(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) 
#define M_MAP2B_11(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) 
#define M_MAP2B_12(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) 
#define M_MAP2B_13(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) 
#define M_MAP2B_14(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) 
#define M_MAP2B_15(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) 
#define M_MAP2B_16(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) 
#define M_MAP2B_17(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) 
#define M_MAP2B_18(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) 
#define M_MAP2B_19(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) 
#define M_MAP2B_20(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) 
#define M_MAP2B_21(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) 
#define M_MAP2B_22(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) 
#define M_MAP2B_23(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) 
#define M_MAP2B_24(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) 
#define M_MAP2B_25(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) 
#define M_MAP2B_26(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) 
#define M_MAP2B_27(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) 
#define M_MAP2B_28(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) 
#define M_MAP2B_29(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) 
#define M_MAP2B_30(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) 
#define M_MAP2B_31(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) 
#define M_MAP2B_32(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) 
#define M_MAP2B_33(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) 
#define M_MAP2B_34(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) 
#define M_MAP2B_35(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) 
#define M_MAP2B_36(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) 
#define M_MAP2B_37(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) 
#define M_MAP2B_38(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) 
#define M_MAP2B_39(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) 
#define M_MAP2B_40(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) 
#define M_MAP2B_41(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) 
#define M_MAP2B_42(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) 
#define M_MAP2B_43(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) 
#define M_MAP2B_44(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) 
#define M_MAP2B_45(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) 
#define M_MAP2B_46(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) 
#define M_MAP2B_47(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) 
#define M_MAP2B_48(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) f(d, _48) 
#define M_MAP2B_49(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) f(d, _48) f(d, _49) 
#define M_MAP2B_50(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) f(d, _48) f(d, _49) f(d, _50) 
#define M_MAP2B_51(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) f(d, _48) f(d, _49) f(d, _50) f(d, _51) 
#define M_MAP2B_52(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) f(d, _48) f(d, _49) f(d, _50) f(d, _51) f(d, _52) 
#define M_MAP2B_53(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) f(d, _48) f(d, _49) f(d, _50) f(d, _51) f(d, _52) f(d, _53) 
#define M_MAP2B_54(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) f(d, _48) f(d, _49) f(d, _50) f(d, _51) f(d, _52) f(d, _53) f(d, _54) 
#define M_MAP2B_55(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) f(d, _48) f(d, _49) f(d, _50) f(d, _51) f(d, _52) f(d, _53) f(d, _54) f(d, _55) 
#define M_MAP2B_56(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) f(d, _48) f(d, _49) f(d, _50) f(d, _51) f(d, _52) f(d, _53) f(d, _54) f(d, _55) f(d, _56) 
#define M_MAP2B_57(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) f(d, _48) f(d, _49) f(d, _50) f(d, _51) f(d, _52) f(d, _53) f(d, _54) f(d, _55) f(d, _56) f(d, _57) 
#define M_MAP2B_58(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) f(d, _48) f(d, _49) f(d, _50) f(d, _51) f(d, _52) f(d, _53) f(d, _54) f(d, _55) f(d, _56) f(d, _57) f(d, _58) 
#define M_MAP2B_59(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) f(d, _48) f(d, _49) f(d, _50) f(d, _51) f(d, _52) f(d, _53) f(d, _54) f(d, _55) f(d, _56) f(d, _57) f(d, _58) f(d, _59) 
#define M_MAP2B_60(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) f(d, _48) f(d, _49) f(d, _50) f(d, _51) f(d, _52) f(d, _53) f(d, _54) f(d, _55) f(d, _56) f(d, _57) f(d, _58) f(d, _59) f(d, _60) 
#define M_MAP2B_61(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) f(d, _48) f(d, _49) f(d, _50) f(d, _51) f(d, _52) f(d, _53) f(d, _54) f(d, _55) f(d, _56) f(d, _57) f(d, _58) f(d, _59) f(d, _60) f(d, _61) 
#define M_MAP2B_62(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) f(d, _48) f(d, _49) f(d, _50) f(d, _51) f(d, _52) f(d, _53) f(d, _54) f(d, _55) f(d, _56) f(d, _57) f(d, _58) f(d, _59) f(d, _60) f(d, _61) f(d, _62) 
#define M_MAP2B_63(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) f(d, _48) f(d, _49) f(d, _50) f(d, _51) f(d, _52) f(d, _53) f(d, _54) f(d, _55) f(d, _56) f(d, _57) f(d, _58) f(d, _59) f(d, _60) f(d, _61) f(d, _62) f(d, _63) 
#define M_MAP2B_64(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) f(d, _48) f(d, _49) f(d, _50) f(d, _51) f(d, _52) f(d, _53) f(d, _54) f(d, _55) f(d, _56) f(d, _57) f(d, _58) f(d, _59) f(d, _60) f(d, _61) f(d, _62) f(d, _63) f(d, _64) 
#define M_MAP2B_65(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) f(d, _48) f(d, _49) f(d, _50) f(d, _51) f(d, _52) f(d, _53) f(d, _54) f(d, _55) f(d, _56) f(d, _57) f(d, _58) f(d, _59) f(d, _60) f(d, _61) f(d, _62) f(d, _63) f(d, _64) f(d, _65) 
#define M_MAP2B_66(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) f(d, _48) f(d, _49) f(d, _50) f(d, _51) f(d, _52) f(d, _53) f(d, _54) f(d, _55) f(d, _56) f(d, _57) f(d, _58) f(d, _59) f(d, _60) f(d, _61) f(d, _62) f(d, _63) f(d, _64) f(d, _65) f(d, _66) 
#define M_MAP2B_67(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) f(d, _48) f(d, _49) f(d, _50) f(d, _51) f(d, _52) f(d, _53) f(d, _54) f(d, _55) f(d, _56) f(d, _57) f(d, _58) f(d, _59) f(d, _60) f(d, _61) f(d, _62) f(d, _63) f(d, _64) f(d, _65) f(d, _66) f(d, _67) 
#define M_MAP2B_68(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) f(d, _48) f(d, _49) f(d, _50) f(d, _51) f(d, _52) f(d, _53) f(d, _54) f(d, _55) f(d, _56) f(d, _57) f(d, _58) f(d, _59) f(d, _60) f(d, _61) f(d, _62) f(d, _63) f(d, _64) f(d, _65) f(d, _66) f(d, _67) f(d, _68) 
#define M_MAP2B_69(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) f(d, _48) f(d, _49) f(d, _50) f(d, _51) f(d, _52) f(d, _53) f(d, _54) f(d, _55) f(d, _56) f(d, _57) f(d, _58) f(d, _59) f(d, _60) f(d, _61) f(d, _62) f(d, _63) f(d, _64) f(d, _65) f(d, _66) f(d, _67) f(d, _68) f(d, _69) 
#define M_MAP2B_70(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) f(d, _48) f(d, _49) f(d, _50) f(d, _51) f(d, _52) f(d, _53) f(d, _54) f(d, _55) f(d, _56) f(d, _57) f(d, _58) f(d, _59) f(d, _60) f(d, _61) f(d, _62) f(d, _63) f(d, _64) f(d, _65) f(d, _66) f(d, _67) f(d, _68) f(d, _69) f(d, _70) 
#define M_MAP2B_71(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) f(d, _48) f(d, _49) f(d, _50) f(d, _51) f(d, _52) f(d, _53) f(d, _54) f(d, _55) f(d, _56) f(d, _57) f(d, _58) f(d, _59) f(d, _60) f(d, _61) f(d, _62) f(d, _63) f(d, _64) f(d, _65) f(d, _66) f(d, _67) f(d, _68) f(d, _69) f(d, _70) f(d, _71) 
#define M_MAP2B_72(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) f(d, _48) f(d, _49) f(d, _50) f(d, _51) f(d, _52) f(d, _53) f(d, _54) f(d, _55) f(d, _56) f(d, _57) f(d, _58) f(d, _59) f(d, _60) f(d, _61) f(d, _62) f(d, _63) f(d, _64) f(d, _65) f(d, _66) f(d, _67) f(d, _68) f(d, _69) f(d, _70) f(d, _71) f(d, _72) 
#define M_MAP2B_73(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) f(d, _48) f(d, _49) f(d, _50) f(d, _51) f(d, _52) f(d, _53) f(d, _54) f(d, _55) f(d, _56) f(d, _57) f(d, _58) f(d, _59) f(d, _60) f(d, _61) f(d, _62) f(d, _63) f(d, _64) f(d, _65) f(d, _66) f(d, _67) f(d, _68) f(d, _69) f(d, _70) f(d, _71) f(d, _72) f(d, _73) 
#define M_MAP2B_74(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) f(d, _48) f(d, _49) f(d, _50) f(d, _51) f(d, _52) f(d, _53) f(d, _54) f(d, _55) f(d, _56) f(d, _57) f(d, _58) f(d, _59) f(d, _60) f(d, _61) f(d, _62) f(d, _63) f(d, _64) f(d, _65) f(d, _66) f(d, _67) f(d, _68) f(d, _69) f(d, _70) f(d, _71) f(d, _72) f(d, _73) f(d, _74) 
#define M_MAP2B_75(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75) f(d, _1) f(d, _2) f(d, _3) f(d, _4) f(d, _5) f(d, _6) f(d, _7) f(d, _8) f(d, _9) f(d, _10) f(d, _11) f(d, _12) f(d, _13) f(d, _14) f(d, _15) f(d, _16) f(d, _17) f(d, _18) f(d, _19) f(d, _20) f(d, _21) f(d, _22) f(d, _23) f(d, _24) f(d, _25) f(d, _26) f(d, _27) f(d, _28) f(d, _29) f(d, _30) f(d, _31) f(d, _32) f(d, _33) f(d, _34) f(d, _35) f(d, _36) f(d, _37) f(d, _38) f(d, _39) f(d, _40) f(d, _41) f(d, _42) f(d, _43) f(d, _44) f(d, _45) f(d, _46) f(d, _47) f(d, _48) f(d, _49) f(d, _50) f(d, _51) f(d, _52) f(d, _53) f(d, _54) f(d, _55) f(d, _56) f(d, _57) f(d, _58) f(d, _59) f(d, _60) f(d, _61) f(d, _62) f(d, _63) f(d, _64) f(d, _65) f(d, _66) f(d, _67) f(d, _68) f(d, _69) f(d, _70) f(d, _71) f(d, _72) f(d, _73) f(d, _74) f(d, _75) 

/* Map a macro to all given arguments with two additional fixed data (non recursive version): 
   one of the parameter is given and one numerical which is the argument number. */
/* Example: M_MAP3(f, data, a, b, c) ==> f(data,1,a) f(data,2,b) f(data,3,c) */
#define M_MAP3(f, d, ...)  M_MAP3I(M_C(M_MAP3I_, M_NARGS(__VA_ARGS__)), f, d, __VA_ARGS__)
#define M_MAP3I(F,...)     F (__VA_ARGS__)
/* Generated by the following command:
   for i in $(seq 1 52) ; do printf "#define M_MAP3I_%d(f, d" $i ; for j in $(seq 1 $i) ; do printf ", _%d" $j ; done ; printf ") " ; for j in $(seq 1 $i) ; do printf "f(d, %d, _%d) " $j $j ; done ; printf "\n"; done
*/
#define M_MAP3I_1(f, d, _1) f(d, 1, _1) 
#define M_MAP3I_2(f, d, _1, _2) f(d, 1, _1) f(d, 2, _2) 
#define M_MAP3I_3(f, d, _1, _2, _3) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) 
#define M_MAP3I_4(f, d, _1, _2, _3, _4) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) 
#define M_MAP3I_5(f, d, _1, _2, _3, _4, _5) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) 
#define M_MAP3I_6(f, d, _1, _2, _3, _4, _5, _6) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) 
#define M_MAP3I_7(f, d, _1, _2, _3, _4, _5, _6, _7) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) 
#define M_MAP3I_8(f, d, _1, _2, _3, _4, _5, _6, _7, _8) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) 
#define M_MAP3I_9(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) 
#define M_MAP3I_10(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) 
#define M_MAP3I_11(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) 
#define M_MAP3I_12(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) 
#define M_MAP3I_13(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) 
#define M_MAP3I_14(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) 
#define M_MAP3I_15(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) 
#define M_MAP3I_16(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) 
#define M_MAP3I_17(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) 
#define M_MAP3I_18(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) 
#define M_MAP3I_19(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) 
#define M_MAP3I_20(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) 
#define M_MAP3I_21(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) 
#define M_MAP3I_22(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) 
#define M_MAP3I_23(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) 
#define M_MAP3I_24(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) 
#define M_MAP3I_25(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) f(d, 25, _25) 
#define M_MAP3I_26(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) f(d, 25, _25) f(d, 26, _26) 
#define M_MAP3I_27(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) f(d, 25, _25) f(d, 26, _26) f(d, 27, _27) 
#define M_MAP3I_28(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) f(d, 25, _25) f(d, 26, _26) f(d, 27, _27) f(d, 28, _28) 
#define M_MAP3I_29(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) f(d, 25, _25) f(d, 26, _26) f(d, 27, _27) f(d, 28, _28) f(d, 29, _29) 
#define M_MAP3I_30(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) f(d, 25, _25) f(d, 26, _26) f(d, 27, _27) f(d, 28, _28) f(d, 29, _29) f(d, 30, _30) 
#define M_MAP3I_31(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) f(d, 25, _25) f(d, 26, _26) f(d, 27, _27) f(d, 28, _28) f(d, 29, _29) f(d, 30, _30) f(d, 31, _31) 
#define M_MAP3I_32(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) f(d, 25, _25) f(d, 26, _26) f(d, 27, _27) f(d, 28, _28) f(d, 29, _29) f(d, 30, _30) f(d, 31, _31) f(d, 32, _32) 
#define M_MAP3I_33(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) f(d, 25, _25) f(d, 26, _26) f(d, 27, _27) f(d, 28, _28) f(d, 29, _29) f(d, 30, _30) f(d, 31, _31) f(d, 32, _32) f(d, 33, _33) 
#define M_MAP3I_34(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) f(d, 25, _25) f(d, 26, _26) f(d, 27, _27) f(d, 28, _28) f(d, 29, _29) f(d, 30, _30) f(d, 31, _31) f(d, 32, _32) f(d, 33, _33) f(d, 34, _34) 
#define M_MAP3I_35(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) f(d, 25, _25) f(d, 26, _26) f(d, 27, _27) f(d, 28, _28) f(d, 29, _29) f(d, 30, _30) f(d, 31, _31) f(d, 32, _32) f(d, 33, _33) f(d, 34, _34) f(d, 35, _35) 
#define M_MAP3I_36(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) f(d, 25, _25) f(d, 26, _26) f(d, 27, _27) f(d, 28, _28) f(d, 29, _29) f(d, 30, _30) f(d, 31, _31) f(d, 32, _32) f(d, 33, _33) f(d, 34, _34) f(d, 35, _35) f(d, 36, _36) 
#define M_MAP3I_37(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) f(d, 25, _25) f(d, 26, _26) f(d, 27, _27) f(d, 28, _28) f(d, 29, _29) f(d, 30, _30) f(d, 31, _31) f(d, 32, _32) f(d, 33, _33) f(d, 34, _34) f(d, 35, _35) f(d, 36, _36) f(d, 37, _37) 
#define M_MAP3I_38(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) f(d, 25, _25) f(d, 26, _26) f(d, 27, _27) f(d, 28, _28) f(d, 29, _29) f(d, 30, _30) f(d, 31, _31) f(d, 32, _32) f(d, 33, _33) f(d, 34, _34) f(d, 35, _35) f(d, 36, _36) f(d, 37, _37) f(d, 38, _38) 
#define M_MAP3I_39(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) f(d, 25, _25) f(d, 26, _26) f(d, 27, _27) f(d, 28, _28) f(d, 29, _29) f(d, 30, _30) f(d, 31, _31) f(d, 32, _32) f(d, 33, _33) f(d, 34, _34) f(d, 35, _35) f(d, 36, _36) f(d, 37, _37) f(d, 38, _38) f(d, 39, _39) 
#define M_MAP3I_40(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) f(d, 25, _25) f(d, 26, _26) f(d, 27, _27) f(d, 28, _28) f(d, 29, _29) f(d, 30, _30) f(d, 31, _31) f(d, 32, _32) f(d, 33, _33) f(d, 34, _34) f(d, 35, _35) f(d, 36, _36) f(d, 37, _37) f(d, 38, _38) f(d, 39, _39) f(d, 40, _40) 
#define M_MAP3I_41(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) f(d, 25, _25) f(d, 26, _26) f(d, 27, _27) f(d, 28, _28) f(d, 29, _29) f(d, 30, _30) f(d, 31, _31) f(d, 32, _32) f(d, 33, _33) f(d, 34, _34) f(d, 35, _35) f(d, 36, _36) f(d, 37, _37) f(d, 38, _38) f(d, 39, _39) f(d, 40, _40) f(d, 41, _41) 
#define M_MAP3I_42(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) f(d, 25, _25) f(d, 26, _26) f(d, 27, _27) f(d, 28, _28) f(d, 29, _29) f(d, 30, _30) f(d, 31, _31) f(d, 32, _32) f(d, 33, _33) f(d, 34, _34) f(d, 35, _35) f(d, 36, _36) f(d, 37, _37) f(d, 38, _38) f(d, 39, _39) f(d, 40, _40) f(d, 41, _41) f(d, 42, _42) 
#define M_MAP3I_43(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) f(d, 25, _25) f(d, 26, _26) f(d, 27, _27) f(d, 28, _28) f(d, 29, _29) f(d, 30, _30) f(d, 31, _31) f(d, 32, _32) f(d, 33, _33) f(d, 34, _34) f(d, 35, _35) f(d, 36, _36) f(d, 37, _37) f(d, 38, _38) f(d, 39, _39) f(d, 40, _40) f(d, 41, _41) f(d, 42, _42) f(d, 43, _43) 
#define M_MAP3I_44(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) f(d, 25, _25) f(d, 26, _26) f(d, 27, _27) f(d, 28, _28) f(d, 29, _29) f(d, 30, _30) f(d, 31, _31) f(d, 32, _32) f(d, 33, _33) f(d, 34, _34) f(d, 35, _35) f(d, 36, _36) f(d, 37, _37) f(d, 38, _38) f(d, 39, _39) f(d, 40, _40) f(d, 41, _41) f(d, 42, _42) f(d, 43, _43) f(d, 44, _44) 
#define M_MAP3I_45(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) f(d, 25, _25) f(d, 26, _26) f(d, 27, _27) f(d, 28, _28) f(d, 29, _29) f(d, 30, _30) f(d, 31, _31) f(d, 32, _32) f(d, 33, _33) f(d, 34, _34) f(d, 35, _35) f(d, 36, _36) f(d, 37, _37) f(d, 38, _38) f(d, 39, _39) f(d, 40, _40) f(d, 41, _41) f(d, 42, _42) f(d, 43, _43) f(d, 44, _44) f(d, 45, _45) 
#define M_MAP3I_46(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) f(d, 25, _25) f(d, 26, _26) f(d, 27, _27) f(d, 28, _28) f(d, 29, _29) f(d, 30, _30) f(d, 31, _31) f(d, 32, _32) f(d, 33, _33) f(d, 34, _34) f(d, 35, _35) f(d, 36, _36) f(d, 37, _37) f(d, 38, _38) f(d, 39, _39) f(d, 40, _40) f(d, 41, _41) f(d, 42, _42) f(d, 43, _43) f(d, 44, _44) f(d, 45, _45) f(d, 46, _46) 
#define M_MAP3I_47(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) f(d, 25, _25) f(d, 26, _26) f(d, 27, _27) f(d, 28, _28) f(d, 29, _29) f(d, 30, _30) f(d, 31, _31) f(d, 32, _32) f(d, 33, _33) f(d, 34, _34) f(d, 35, _35) f(d, 36, _36) f(d, 37, _37) f(d, 38, _38) f(d, 39, _39) f(d, 40, _40) f(d, 41, _41) f(d, 42, _42) f(d, 43, _43) f(d, 44, _44) f(d, 45, _45) f(d, 46, _46) f(d, 47, _47) 
#define M_MAP3I_48(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) f(d, 25, _25) f(d, 26, _26) f(d, 27, _27) f(d, 28, _28) f(d, 29, _29) f(d, 30, _30) f(d, 31, _31) f(d, 32, _32) f(d, 33, _33) f(d, 34, _34) f(d, 35, _35) f(d, 36, _36) f(d, 37, _37) f(d, 38, _38) f(d, 39, _39) f(d, 40, _40) f(d, 41, _41) f(d, 42, _42) f(d, 43, _43) f(d, 44, _44) f(d, 45, _45) f(d, 46, _46) f(d, 47, _47) f(d, 48, _48) 
#define M_MAP3I_49(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) f(d, 25, _25) f(d, 26, _26) f(d, 27, _27) f(d, 28, _28) f(d, 29, _29) f(d, 30, _30) f(d, 31, _31) f(d, 32, _32) f(d, 33, _33) f(d, 34, _34) f(d, 35, _35) f(d, 36, _36) f(d, 37, _37) f(d, 38, _38) f(d, 39, _39) f(d, 40, _40) f(d, 41, _41) f(d, 42, _42) f(d, 43, _43) f(d, 44, _44) f(d, 45, _45) f(d, 46, _46) f(d, 47, _47) f(d, 48, _48) f(d, 49, _49) 
#define M_MAP3I_50(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) f(d, 25, _25) f(d, 26, _26) f(d, 27, _27) f(d, 28, _28) f(d, 29, _29) f(d, 30, _30) f(d, 31, _31) f(d, 32, _32) f(d, 33, _33) f(d, 34, _34) f(d, 35, _35) f(d, 36, _36) f(d, 37, _37) f(d, 38, _38) f(d, 39, _39) f(d, 40, _40) f(d, 41, _41) f(d, 42, _42) f(d, 43, _43) f(d, 44, _44) f(d, 45, _45) f(d, 46, _46) f(d, 47, _47) f(d, 48, _48) f(d, 49, _49) f(d, 50, _50) 
#define M_MAP3I_51(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) f(d, 25, _25) f(d, 26, _26) f(d, 27, _27) f(d, 28, _28) f(d, 29, _29) f(d, 30, _30) f(d, 31, _31) f(d, 32, _32) f(d, 33, _33) f(d, 34, _34) f(d, 35, _35) f(d, 36, _36) f(d, 37, _37) f(d, 38, _38) f(d, 39, _39) f(d, 40, _40) f(d, 41, _41) f(d, 42, _42) f(d, 43, _43) f(d, 44, _44) f(d, 45, _45) f(d, 46, _46) f(d, 47, _47) f(d, 48, _48) f(d, 49, _49) f(d, 50, _50) f(d, 51, _51) 
#define M_MAP3I_52(f, d, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52) f(d, 1, _1) f(d, 2, _2) f(d, 3, _3) f(d, 4, _4) f(d, 5, _5) f(d, 6, _6) f(d, 7, _7) f(d, 8, _8) f(d, 9, _9) f(d, 10, _10) f(d, 11, _11) f(d, 12, _12) f(d, 13, _13) f(d, 14, _14) f(d, 15, _15) f(d, 16, _16) f(d, 17, _17) f(d, 18, _18) f(d, 19, _19) f(d, 20, _20) f(d, 21, _21) f(d, 22, _22) f(d, 23, _23) f(d, 24, _24) f(d, 25, _25) f(d, 26, _26) f(d, 27, _27) f(d, 28, _28) f(d, 29, _29) f(d, 30, _30) f(d, 31, _31) f(d, 32, _32) f(d, 33, _33) f(d, 34, _34) f(d, 35, _35) f(d, 36, _36) f(d, 37, _37) f(d, 38, _38) f(d, 39, _39) f(d, 40, _40) f(d, 41, _41) f(d, 42, _42) f(d, 43, _43) f(d, 44, _44) f(d, 45, _45) f(d, 46, _46) f(d, 47, _47) f(d, 48, _48) f(d, 49, _49) f(d, 50, _50) f(d, 51, _51) f(d, 52, _52) 



/* Map a macro to all given pair of arguments (Using recursivity) */
/* Example: M_MAP_PAIR(f, a, b, c, d) ==> f(a,b) f(c,d) */
#define M_MAP_PAIR_L0_INDIRECT()      M_MAP_PAIR_L0
#define M_MAP_PAIR_L0(f, ...)         M_IF_NARGS_EQ2(__VA_ARGS__)( f(__VA_ARGS__) , M_MAP_PAIR_L1(f, __VA_ARGS__))
#define M_MAP_PAIR_L1(f, a, b, ...)   f(a,b) M_DELAY3(M_MAP_PAIR_L0_INDIRECT) () (f, __VA_ARGS__)
#define M_MAP_PAIR(f, ...)            M_IF_EMPTY(__VA_ARGS__)( /* end */, M_EVAL(M_MAP_PAIR_L0(f, __VA_ARGS__)))


/* Map a macro to all given arguments and reduce all theses computation
   with another reduce macro */
/* Example: M_REDUCE(f, g, a, b, c) ==> g( f(a), g( f(b), f(c)) */
/* Generated by:
   for i in $(seq 1 52) ; do printf "#define M_REDUCEI1_%d(f, g" $i ; for j in $(seq 1 $i) ; do printf ", _%d" $j ; done ; printf ") "; for j in $(seq 1 $(( $i - 1 )) ) ; do printf "g(f(_%d), " $j; done ; printf "f(_%d" $i; for j in $(seq 1 $i); do printf ")"; done ; printf "\n";  done
*/
#define M_REDUCE(f, g, ...)            M_REDUCEI1_0(M_C(M_REDUCEI1_, M_NARGS(__VA_ARGS__)), f, g, __VA_ARGS__)
#define M_REDUCEI1_0(F, ...)           F(__VA_ARGS__)
#define M_REDUCEI1_1(f, g, _1) f(_1)
#define M_REDUCEI1_2(f, g, _1, _2) g(f(_1), f(_2))
#define M_REDUCEI1_3(f, g, _1, _2, _3) g(f(_1), g(f(_2), f(_3)))
#define M_REDUCEI1_4(f, g, _1, _2, _3, _4) g(f(_1), g(f(_2), g(f(_3), f(_4))))
#define M_REDUCEI1_5(f, g, _1, _2, _3, _4, _5) g(f(_1), g(f(_2), g(f(_3), g(f(_4), f(_5)))))
#define M_REDUCEI1_6(f, g, _1, _2, _3, _4, _5, _6) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), f(_6))))))
#define M_REDUCEI1_7(f, g, _1, _2, _3, _4, _5, _6, _7) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), f(_7)))))))
#define M_REDUCEI1_8(f, g, _1, _2, _3, _4, _5, _6, _7, _8) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), f(_8))))))))
#define M_REDUCEI1_9(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), f(_9)))))))))
#define M_REDUCEI1_10(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), f(_10))))))))))
#define M_REDUCEI1_11(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), f(_11)))))))))))
#define M_REDUCEI1_12(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), f(_12))))))))))))
#define M_REDUCEI1_13(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), f(_13)))))))))))))
#define M_REDUCEI1_14(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), f(_14))))))))))))))
#define M_REDUCEI1_15(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), f(_15)))))))))))))))
#define M_REDUCEI1_16(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), f(_16))))))))))))))))
#define M_REDUCEI1_17(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), f(_17)))))))))))))))))
#define M_REDUCEI1_18(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), f(_18))))))))))))))))))
#define M_REDUCEI1_19(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), f(_19)))))))))))))))))))
#define M_REDUCEI1_20(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), f(_20))))))))))))))))))))
#define M_REDUCEI1_21(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), f(_21)))))))))))))))))))))
#define M_REDUCEI1_22(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), f(_22))))))))))))))))))))))
#define M_REDUCEI1_23(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), f(_23)))))))))))))))))))))))
#define M_REDUCEI1_24(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), f(_24))))))))))))))))))))))))
#define M_REDUCEI1_25(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), g(f(_24), f(_25)))))))))))))))))))))))))
#define M_REDUCEI1_26(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), g(f(_24), g(f(_25), f(_26))))))))))))))))))))))))))
#define M_REDUCEI1_27(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), g(f(_24), g(f(_25), g(f(_26), f(_27)))))))))))))))))))))))))))
#define M_REDUCEI1_28(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), g(f(_24), g(f(_25), g(f(_26), g(f(_27), f(_28))))))))))))))))))))))))))))
#define M_REDUCEI1_29(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), g(f(_24), g(f(_25), g(f(_26), g(f(_27), g(f(_28), f(_29)))))))))))))))))))))))))))))
#define M_REDUCEI1_30(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), g(f(_24), g(f(_25), g(f(_26), g(f(_27), g(f(_28), g(f(_29), f(_30))))))))))))))))))))))))))))))
#define M_REDUCEI1_31(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), g(f(_24), g(f(_25), g(f(_26), g(f(_27), g(f(_28), g(f(_29), g(f(_30), f(_31)))))))))))))))))))))))))))))))
#define M_REDUCEI1_32(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), g(f(_24), g(f(_25), g(f(_26), g(f(_27), g(f(_28), g(f(_29), g(f(_30), g(f(_31), f(_32))))))))))))))))))))))))))))))))
#define M_REDUCEI1_33(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), g(f(_24), g(f(_25), g(f(_26), g(f(_27), g(f(_28), g(f(_29), g(f(_30), g(f(_31), g(f(_32), f(_33)))))))))))))))))))))))))))))))))
#define M_REDUCEI1_34(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), g(f(_24), g(f(_25), g(f(_26), g(f(_27), g(f(_28), g(f(_29), g(f(_30), g(f(_31), g(f(_32), g(f(_33), f(_34))))))))))))))))))))))))))))))))))
#define M_REDUCEI1_35(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), g(f(_24), g(f(_25), g(f(_26), g(f(_27), g(f(_28), g(f(_29), g(f(_30), g(f(_31), g(f(_32), g(f(_33), g(f(_34), f(_35)))))))))))))))))))))))))))))))))))
#define M_REDUCEI1_36(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), g(f(_24), g(f(_25), g(f(_26), g(f(_27), g(f(_28), g(f(_29), g(f(_30), g(f(_31), g(f(_32), g(f(_33), g(f(_34), g(f(_35), f(_36))))))))))))))))))))))))))))))))))))
#define M_REDUCEI1_37(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), g(f(_24), g(f(_25), g(f(_26), g(f(_27), g(f(_28), g(f(_29), g(f(_30), g(f(_31), g(f(_32), g(f(_33), g(f(_34), g(f(_35), g(f(_36), f(_37)))))))))))))))))))))))))))))))))))))
#define M_REDUCEI1_38(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), g(f(_24), g(f(_25), g(f(_26), g(f(_27), g(f(_28), g(f(_29), g(f(_30), g(f(_31), g(f(_32), g(f(_33), g(f(_34), g(f(_35), g(f(_36), g(f(_37), f(_38))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI1_39(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), g(f(_24), g(f(_25), g(f(_26), g(f(_27), g(f(_28), g(f(_29), g(f(_30), g(f(_31), g(f(_32), g(f(_33), g(f(_34), g(f(_35), g(f(_36), g(f(_37), g(f(_38), f(_39)))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI1_40(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), g(f(_24), g(f(_25), g(f(_26), g(f(_27), g(f(_28), g(f(_29), g(f(_30), g(f(_31), g(f(_32), g(f(_33), g(f(_34), g(f(_35), g(f(_36), g(f(_37), g(f(_38), g(f(_39), f(_40))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI1_41(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), g(f(_24), g(f(_25), g(f(_26), g(f(_27), g(f(_28), g(f(_29), g(f(_30), g(f(_31), g(f(_32), g(f(_33), g(f(_34), g(f(_35), g(f(_36), g(f(_37), g(f(_38), g(f(_39), g(f(_40), f(_41)))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI1_42(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), g(f(_24), g(f(_25), g(f(_26), g(f(_27), g(f(_28), g(f(_29), g(f(_30), g(f(_31), g(f(_32), g(f(_33), g(f(_34), g(f(_35), g(f(_36), g(f(_37), g(f(_38), g(f(_39), g(f(_40), g(f(_41), f(_42))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI1_43(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), g(f(_24), g(f(_25), g(f(_26), g(f(_27), g(f(_28), g(f(_29), g(f(_30), g(f(_31), g(f(_32), g(f(_33), g(f(_34), g(f(_35), g(f(_36), g(f(_37), g(f(_38), g(f(_39), g(f(_40), g(f(_41), g(f(_42), f(_43)))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI1_44(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), g(f(_24), g(f(_25), g(f(_26), g(f(_27), g(f(_28), g(f(_29), g(f(_30), g(f(_31), g(f(_32), g(f(_33), g(f(_34), g(f(_35), g(f(_36), g(f(_37), g(f(_38), g(f(_39), g(f(_40), g(f(_41), g(f(_42), g(f(_43), f(_44))))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI1_45(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), g(f(_24), g(f(_25), g(f(_26), g(f(_27), g(f(_28), g(f(_29), g(f(_30), g(f(_31), g(f(_32), g(f(_33), g(f(_34), g(f(_35), g(f(_36), g(f(_37), g(f(_38), g(f(_39), g(f(_40), g(f(_41), g(f(_42), g(f(_43), g(f(_44), f(_45)))))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI1_46(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), g(f(_24), g(f(_25), g(f(_26), g(f(_27), g(f(_28), g(f(_29), g(f(_30), g(f(_31), g(f(_32), g(f(_33), g(f(_34), g(f(_35), g(f(_36), g(f(_37), g(f(_38), g(f(_39), g(f(_40), g(f(_41), g(f(_42), g(f(_43), g(f(_44), g(f(_45), f(_46))))))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI1_47(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), g(f(_24), g(f(_25), g(f(_26), g(f(_27), g(f(_28), g(f(_29), g(f(_30), g(f(_31), g(f(_32), g(f(_33), g(f(_34), g(f(_35), g(f(_36), g(f(_37), g(f(_38), g(f(_39), g(f(_40), g(f(_41), g(f(_42), g(f(_43), g(f(_44), g(f(_45), g(f(_46), f(_47)))))))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI1_48(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), g(f(_24), g(f(_25), g(f(_26), g(f(_27), g(f(_28), g(f(_29), g(f(_30), g(f(_31), g(f(_32), g(f(_33), g(f(_34), g(f(_35), g(f(_36), g(f(_37), g(f(_38), g(f(_39), g(f(_40), g(f(_41), g(f(_42), g(f(_43), g(f(_44), g(f(_45), g(f(_46), g(f(_47), f(_48))))))))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI1_49(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), g(f(_24), g(f(_25), g(f(_26), g(f(_27), g(f(_28), g(f(_29), g(f(_30), g(f(_31), g(f(_32), g(f(_33), g(f(_34), g(f(_35), g(f(_36), g(f(_37), g(f(_38), g(f(_39), g(f(_40), g(f(_41), g(f(_42), g(f(_43), g(f(_44), g(f(_45), g(f(_46), g(f(_47), g(f(_48), f(_49)))))))))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI1_50(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), g(f(_24), g(f(_25), g(f(_26), g(f(_27), g(f(_28), g(f(_29), g(f(_30), g(f(_31), g(f(_32), g(f(_33), g(f(_34), g(f(_35), g(f(_36), g(f(_37), g(f(_38), g(f(_39), g(f(_40), g(f(_41), g(f(_42), g(f(_43), g(f(_44), g(f(_45), g(f(_46), g(f(_47), g(f(_48), g(f(_49), f(_50))))))))))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI1_51(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), g(f(_24), g(f(_25), g(f(_26), g(f(_27), g(f(_28), g(f(_29), g(f(_30), g(f(_31), g(f(_32), g(f(_33), g(f(_34), g(f(_35), g(f(_36), g(f(_37), g(f(_38), g(f(_39), g(f(_40), g(f(_41), g(f(_42), g(f(_43), g(f(_44), g(f(_45), g(f(_46), g(f(_47), g(f(_48), g(f(_49), g(f(_50), f(_51)))))))))))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI1_52(f, g, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52) g(f(_1), g(f(_2), g(f(_3), g(f(_4), g(f(_5), g(f(_6), g(f(_7), g(f(_8), g(f(_9), g(f(_10), g(f(_11), g(f(_12), g(f(_13), g(f(_14), g(f(_15), g(f(_16), g(f(_17), g(f(_18), g(f(_19), g(f(_20), g(f(_21), g(f(_22), g(f(_23), g(f(_24), g(f(_25), g(f(_26), g(f(_27), g(f(_28), g(f(_29), g(f(_30), g(f(_31), g(f(_32), g(f(_33), g(f(_34), g(f(_35), g(f(_36), g(f(_37), g(f(_38), g(f(_39), g(f(_40), g(f(_41), g(f(_42), g(f(_43), g(f(_44), g(f(_45), g(f(_46), g(f(_47), g(f(_48), g(f(_49), g(f(_50), g(f(_51), f(_52))))))))))))))))))))))))))))))))))))))))))))))))))))


/* Map a macro to all given arguments and reduce all theses computation
   with another reduce macro with an argument:
   M_REDUCE2(f, g, data, a, b, c) ==> g( f(data,a), g( f(data, b), f(data, c)))
   Generated by:
   for i in $(seq 1 52) ; do printf "#define M_REDUCEI2_%d(f,g,d," $i ; for j in $(seq 1 $(($i - 1))) ; do printf "_%d, " $j ; done ; printf "_%d) " $i ; for j in $(seq 1 $(($i - 1))) ; do printf "g(f(d, _%d), " $j ; done ; printf "f(d, _%d)" $i ; for j in $(seq 1 $(( $i - 1 ))); do printf ")" ; done ; printf "\n" ; done
 */
#define M_REDUCE2(f, g, d, ...)        M_REDUCEI2_0(M_C(M_REDUCEI2_, M_NARGS(__VA_ARGS__)), f, g, d, __VA_ARGS__)
#define M_REDUCEI2_0(F, ...)           F(__VA_ARGS__)
#define M_REDUCEI2_1(f,g,d,_1) f(d, _1)
#define M_REDUCEI2_2(f,g,d,_1, _2) g(f(d, _1), f(d, _2))
#define M_REDUCEI2_3(f,g,d,_1, _2, _3) g(f(d, _1), g(f(d, _2), f(d, _3)))
#define M_REDUCEI2_4(f,g,d,_1, _2, _3, _4) g(f(d, _1), g(f(d, _2), g(f(d, _3), f(d, _4))))
#define M_REDUCEI2_5(f,g,d,_1, _2, _3, _4, _5) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), f(d, _5)))))
#define M_REDUCEI2_6(f,g,d,_1, _2, _3, _4, _5, _6) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), f(d, _6))))))
#define M_REDUCEI2_7(f,g,d,_1, _2, _3, _4, _5, _6, _7) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), f(d, _7)))))))
#define M_REDUCEI2_8(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), f(d, _8))))))))
#define M_REDUCEI2_9(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), f(d, _9)))))))))
#define M_REDUCEI2_10(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), f(d, _10))))))))))
#define M_REDUCEI2_11(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), f(d, _11)))))))))))
#define M_REDUCEI2_12(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), f(d, _12))))))))))))
#define M_REDUCEI2_13(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), f(d, _13)))))))))))))
#define M_REDUCEI2_14(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), f(d, _14))))))))))))))
#define M_REDUCEI2_15(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), f(d, _15)))))))))))))))
#define M_REDUCEI2_16(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), f(d, _16))))))))))))))))
#define M_REDUCEI2_17(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), f(d, _17)))))))))))))))))
#define M_REDUCEI2_18(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), f(d, _18))))))))))))))))))
#define M_REDUCEI2_19(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), f(d, _19)))))))))))))))))))
#define M_REDUCEI2_20(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), f(d, _20))))))))))))))))))))
#define M_REDUCEI2_21(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), f(d, _21)))))))))))))))))))))
#define M_REDUCEI2_22(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), f(d, _22))))))))))))))))))))))
#define M_REDUCEI2_23(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), f(d, _23)))))))))))))))))))))))
#define M_REDUCEI2_24(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), f(d, _24))))))))))))))))))))))))
#define M_REDUCEI2_25(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), g(f(d, _24), f(d, _25)))))))))))))))))))))))))
#define M_REDUCEI2_26(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), g(f(d, _24), g(f(d, _25), f(d, _26))))))))))))))))))))))))))
#define M_REDUCEI2_27(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), g(f(d, _24), g(f(d, _25), g(f(d, _26), f(d, _27)))))))))))))))))))))))))))
#define M_REDUCEI2_28(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), g(f(d, _24), g(f(d, _25), g(f(d, _26), g(f(d, _27), f(d, _28))))))))))))))))))))))))))))
#define M_REDUCEI2_29(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), g(f(d, _24), g(f(d, _25), g(f(d, _26), g(f(d, _27), g(f(d, _28), f(d, _29)))))))))))))))))))))))))))))
#define M_REDUCEI2_30(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), g(f(d, _24), g(f(d, _25), g(f(d, _26), g(f(d, _27), g(f(d, _28), g(f(d, _29), f(d, _30))))))))))))))))))))))))))))))
#define M_REDUCEI2_31(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), g(f(d, _24), g(f(d, _25), g(f(d, _26), g(f(d, _27), g(f(d, _28), g(f(d, _29), g(f(d, _30), f(d, _31)))))))))))))))))))))))))))))))
#define M_REDUCEI2_32(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), g(f(d, _24), g(f(d, _25), g(f(d, _26), g(f(d, _27), g(f(d, _28), g(f(d, _29), g(f(d, _30), g(f(d, _31), f(d, _32))))))))))))))))))))))))))))))))
#define M_REDUCEI2_33(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), g(f(d, _24), g(f(d, _25), g(f(d, _26), g(f(d, _27), g(f(d, _28), g(f(d, _29), g(f(d, _30), g(f(d, _31), g(f(d, _32), f(d, _33)))))))))))))))))))))))))))))))))
#define M_REDUCEI2_34(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), g(f(d, _24), g(f(d, _25), g(f(d, _26), g(f(d, _27), g(f(d, _28), g(f(d, _29), g(f(d, _30), g(f(d, _31), g(f(d, _32), g(f(d, _33), f(d, _34))))))))))))))))))))))))))))))))))
#define M_REDUCEI2_35(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), g(f(d, _24), g(f(d, _25), g(f(d, _26), g(f(d, _27), g(f(d, _28), g(f(d, _29), g(f(d, _30), g(f(d, _31), g(f(d, _32), g(f(d, _33), g(f(d, _34), f(d, _35)))))))))))))))))))))))))))))))))))
#define M_REDUCEI2_36(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), g(f(d, _24), g(f(d, _25), g(f(d, _26), g(f(d, _27), g(f(d, _28), g(f(d, _29), g(f(d, _30), g(f(d, _31), g(f(d, _32), g(f(d, _33), g(f(d, _34), g(f(d, _35), f(d, _36))))))))))))))))))))))))))))))))))))
#define M_REDUCEI2_37(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), g(f(d, _24), g(f(d, _25), g(f(d, _26), g(f(d, _27), g(f(d, _28), g(f(d, _29), g(f(d, _30), g(f(d, _31), g(f(d, _32), g(f(d, _33), g(f(d, _34), g(f(d, _35), g(f(d, _36), f(d, _37)))))))))))))))))))))))))))))))))))))
#define M_REDUCEI2_38(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), g(f(d, _24), g(f(d, _25), g(f(d, _26), g(f(d, _27), g(f(d, _28), g(f(d, _29), g(f(d, _30), g(f(d, _31), g(f(d, _32), g(f(d, _33), g(f(d, _34), g(f(d, _35), g(f(d, _36), g(f(d, _37), f(d, _38))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI2_39(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), g(f(d, _24), g(f(d, _25), g(f(d, _26), g(f(d, _27), g(f(d, _28), g(f(d, _29), g(f(d, _30), g(f(d, _31), g(f(d, _32), g(f(d, _33), g(f(d, _34), g(f(d, _35), g(f(d, _36), g(f(d, _37), g(f(d, _38), f(d, _39)))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI2_40(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), g(f(d, _24), g(f(d, _25), g(f(d, _26), g(f(d, _27), g(f(d, _28), g(f(d, _29), g(f(d, _30), g(f(d, _31), g(f(d, _32), g(f(d, _33), g(f(d, _34), g(f(d, _35), g(f(d, _36), g(f(d, _37), g(f(d, _38), g(f(d, _39), f(d, _40))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI2_41(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), g(f(d, _24), g(f(d, _25), g(f(d, _26), g(f(d, _27), g(f(d, _28), g(f(d, _29), g(f(d, _30), g(f(d, _31), g(f(d, _32), g(f(d, _33), g(f(d, _34), g(f(d, _35), g(f(d, _36), g(f(d, _37), g(f(d, _38), g(f(d, _39), g(f(d, _40), f(d, _41)))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI2_42(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), g(f(d, _24), g(f(d, _25), g(f(d, _26), g(f(d, _27), g(f(d, _28), g(f(d, _29), g(f(d, _30), g(f(d, _31), g(f(d, _32), g(f(d, _33), g(f(d, _34), g(f(d, _35), g(f(d, _36), g(f(d, _37), g(f(d, _38), g(f(d, _39), g(f(d, _40), g(f(d, _41), f(d, _42))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI2_43(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), g(f(d, _24), g(f(d, _25), g(f(d, _26), g(f(d, _27), g(f(d, _28), g(f(d, _29), g(f(d, _30), g(f(d, _31), g(f(d, _32), g(f(d, _33), g(f(d, _34), g(f(d, _35), g(f(d, _36), g(f(d, _37), g(f(d, _38), g(f(d, _39), g(f(d, _40), g(f(d, _41), g(f(d, _42), f(d, _43)))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI2_44(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), g(f(d, _24), g(f(d, _25), g(f(d, _26), g(f(d, _27), g(f(d, _28), g(f(d, _29), g(f(d, _30), g(f(d, _31), g(f(d, _32), g(f(d, _33), g(f(d, _34), g(f(d, _35), g(f(d, _36), g(f(d, _37), g(f(d, _38), g(f(d, _39), g(f(d, _40), g(f(d, _41), g(f(d, _42), g(f(d, _43), f(d, _44))))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI2_45(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), g(f(d, _24), g(f(d, _25), g(f(d, _26), g(f(d, _27), g(f(d, _28), g(f(d, _29), g(f(d, _30), g(f(d, _31), g(f(d, _32), g(f(d, _33), g(f(d, _34), g(f(d, _35), g(f(d, _36), g(f(d, _37), g(f(d, _38), g(f(d, _39), g(f(d, _40), g(f(d, _41), g(f(d, _42), g(f(d, _43), g(f(d, _44), f(d, _45)))))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI2_46(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), g(f(d, _24), g(f(d, _25), g(f(d, _26), g(f(d, _27), g(f(d, _28), g(f(d, _29), g(f(d, _30), g(f(d, _31), g(f(d, _32), g(f(d, _33), g(f(d, _34), g(f(d, _35), g(f(d, _36), g(f(d, _37), g(f(d, _38), g(f(d, _39), g(f(d, _40), g(f(d, _41), g(f(d, _42), g(f(d, _43), g(f(d, _44), g(f(d, _45), f(d, _46))))))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI2_47(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), g(f(d, _24), g(f(d, _25), g(f(d, _26), g(f(d, _27), g(f(d, _28), g(f(d, _29), g(f(d, _30), g(f(d, _31), g(f(d, _32), g(f(d, _33), g(f(d, _34), g(f(d, _35), g(f(d, _36), g(f(d, _37), g(f(d, _38), g(f(d, _39), g(f(d, _40), g(f(d, _41), g(f(d, _42), g(f(d, _43), g(f(d, _44), g(f(d, _45), g(f(d, _46), f(d, _47)))))))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI2_48(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), g(f(d, _24), g(f(d, _25), g(f(d, _26), g(f(d, _27), g(f(d, _28), g(f(d, _29), g(f(d, _30), g(f(d, _31), g(f(d, _32), g(f(d, _33), g(f(d, _34), g(f(d, _35), g(f(d, _36), g(f(d, _37), g(f(d, _38), g(f(d, _39), g(f(d, _40), g(f(d, _41), g(f(d, _42), g(f(d, _43), g(f(d, _44), g(f(d, _45), g(f(d, _46), g(f(d, _47), f(d, _48))))))))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI2_49(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), g(f(d, _24), g(f(d, _25), g(f(d, _26), g(f(d, _27), g(f(d, _28), g(f(d, _29), g(f(d, _30), g(f(d, _31), g(f(d, _32), g(f(d, _33), g(f(d, _34), g(f(d, _35), g(f(d, _36), g(f(d, _37), g(f(d, _38), g(f(d, _39), g(f(d, _40), g(f(d, _41), g(f(d, _42), g(f(d, _43), g(f(d, _44), g(f(d, _45), g(f(d, _46), g(f(d, _47), g(f(d, _48), f(d, _49)))))))))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI2_50(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), g(f(d, _24), g(f(d, _25), g(f(d, _26), g(f(d, _27), g(f(d, _28), g(f(d, _29), g(f(d, _30), g(f(d, _31), g(f(d, _32), g(f(d, _33), g(f(d, _34), g(f(d, _35), g(f(d, _36), g(f(d, _37), g(f(d, _38), g(f(d, _39), g(f(d, _40), g(f(d, _41), g(f(d, _42), g(f(d, _43), g(f(d, _44), g(f(d, _45), g(f(d, _46), g(f(d, _47), g(f(d, _48), g(f(d, _49), f(d, _50))))))))))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI2_51(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), g(f(d, _24), g(f(d, _25), g(f(d, _26), g(f(d, _27), g(f(d, _28), g(f(d, _29), g(f(d, _30), g(f(d, _31), g(f(d, _32), g(f(d, _33), g(f(d, _34), g(f(d, _35), g(f(d, _36), g(f(d, _37), g(f(d, _38), g(f(d, _39), g(f(d, _40), g(f(d, _41), g(f(d, _42), g(f(d, _43), g(f(d, _44), g(f(d, _45), g(f(d, _46), g(f(d, _47), g(f(d, _48), g(f(d, _49), g(f(d, _50), f(d, _51)))))))))))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI2_52(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52) g(f(d, _1), g(f(d, _2), g(f(d, _3), g(f(d, _4), g(f(d, _5), g(f(d, _6), g(f(d, _7), g(f(d, _8), g(f(d, _9), g(f(d, _10), g(f(d, _11), g(f(d, _12), g(f(d, _13), g(f(d, _14), g(f(d, _15), g(f(d, _16), g(f(d, _17), g(f(d, _18), g(f(d, _19), g(f(d, _20), g(f(d, _21), g(f(d, _22), g(f(d, _23), g(f(d, _24), g(f(d, _25), g(f(d, _26), g(f(d, _27), g(f(d, _28), g(f(d, _29), g(f(d, _30), g(f(d, _31), g(f(d, _32), g(f(d, _33), g(f(d, _34), g(f(d, _35), g(f(d, _36), g(f(d, _37), g(f(d, _38), g(f(d, _39), g(f(d, _40), g(f(d, _41), g(f(d, _42), g(f(d, _43), g(f(d, _44), g(f(d, _45), g(f(d, _46), g(f(d, _47), g(f(d, _48), g(f(d, _49), g(f(d, _50), g(f(d, _51), f(d, _52))))))))))))))))))))))))))))))))))))))))))))))))))))


/* Map a macro to all given arguments and reduce all theses computation
   with another reduce macro with an argument and a counter:
   M_REDUCE3(f, g, data, a, b, c) ==> g( f(data,1,a), g( f(data, 2, b), f(data, 3, c)))
   Generated by:
   for i in $(seq 1 52) ; do printf "#define M_REDUCEI3_%d(f,g,d," $i ; for j in $(seq 1 $(($i - 1))) ; do printf "_%d, " $j ; done ; printf "_%d) " $i ; for j in $(seq 1 $(($i - 1))) ; do printf "g(f(d, %d, _%d), " $j $j ; done ; printf "f(d, %d, _%d)" $i $i ; for j in $(seq 1 $(( $i - 1 ))); do printf ")" ; done ; printf "\n" ; done
 */
#define M_REDUCE3(f, g, d, ...)        M_REDUCEI3_0(M_C(M_REDUCEI3_, M_NARGS(__VA_ARGS__)), f, g, d, __VA_ARGS__)
#define M_REDUCEI3_0(F, ...)           F(__VA_ARGS__)
#define M_REDUCEI3_1(f,g,d,_1) f(d, 1, _1)
#define M_REDUCEI3_2(f,g,d,_1, _2) g(f(d, 1, _1), f(d, 2, _2))
#define M_REDUCEI3_3(f,g,d,_1, _2, _3) g(f(d, 1, _1), g(f(d, 2, _2), f(d, 3, _3)))
#define M_REDUCEI3_4(f,g,d,_1, _2, _3, _4) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), f(d, 4, _4))))
#define M_REDUCEI3_5(f,g,d,_1, _2, _3, _4, _5) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), f(d, 5, _5)))))
#define M_REDUCEI3_6(f,g,d,_1, _2, _3, _4, _5, _6) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), f(d, 6, _6))))))
#define M_REDUCEI3_7(f,g,d,_1, _2, _3, _4, _5, _6, _7) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), f(d, 7, _7)))))))
#define M_REDUCEI3_8(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), f(d, 8, _8))))))))
#define M_REDUCEI3_9(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), f(d, 9, _9)))))))))
#define M_REDUCEI3_10(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), f(d, 10, _10))))))))))
#define M_REDUCEI3_11(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), f(d, 11, _11)))))))))))
#define M_REDUCEI3_12(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), f(d, 12, _12))))))))))))
#define M_REDUCEI3_13(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), f(d, 13, _13)))))))))))))
#define M_REDUCEI3_14(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), f(d, 14, _14))))))))))))))
#define M_REDUCEI3_15(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), f(d, 15, _15)))))))))))))))
#define M_REDUCEI3_16(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), f(d, 16, _16))))))))))))))))
#define M_REDUCEI3_17(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), f(d, 17, _17)))))))))))))))))
#define M_REDUCEI3_18(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), f(d, 18, _18))))))))))))))))))
#define M_REDUCEI3_19(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), f(d, 19, _19)))))))))))))))))))
#define M_REDUCEI3_20(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), f(d, 20, _20))))))))))))))))))))
#define M_REDUCEI3_21(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), f(d, 21, _21)))))))))))))))))))))
#define M_REDUCEI3_22(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), f(d, 22, _22))))))))))))))))))))))
#define M_REDUCEI3_23(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), f(d, 23, _23)))))))))))))))))))))))
#define M_REDUCEI3_24(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), f(d, 24, _24))))))))))))))))))))))))
#define M_REDUCEI3_25(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), g(f(d, 24, _24), f(d, 25, _25)))))))))))))))))))))))))
#define M_REDUCEI3_26(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), g(f(d, 24, _24), g(f(d, 25, _25), f(d, 26, _26))))))))))))))))))))))))))
#define M_REDUCEI3_27(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), g(f(d, 24, _24), g(f(d, 25, _25), g(f(d, 26, _26), f(d, 27, _27)))))))))))))))))))))))))))
#define M_REDUCEI3_28(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), g(f(d, 24, _24), g(f(d, 25, _25), g(f(d, 26, _26), g(f(d, 27, _27), f(d, 28, _28))))))))))))))))))))))))))))
#define M_REDUCEI3_29(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), g(f(d, 24, _24), g(f(d, 25, _25), g(f(d, 26, _26), g(f(d, 27, _27), g(f(d, 28, _28), f(d, 29, _29)))))))))))))))))))))))))))))
#define M_REDUCEI3_30(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), g(f(d, 24, _24), g(f(d, 25, _25), g(f(d, 26, _26), g(f(d, 27, _27), g(f(d, 28, _28), g(f(d, 29, _29), f(d, 30, _30))))))))))))))))))))))))))))))
#define M_REDUCEI3_31(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), g(f(d, 24, _24), g(f(d, 25, _25), g(f(d, 26, _26), g(f(d, 27, _27), g(f(d, 28, _28), g(f(d, 29, _29), g(f(d, 30, _30), f(d, 31, _31)))))))))))))))))))))))))))))))
#define M_REDUCEI3_32(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), g(f(d, 24, _24), g(f(d, 25, _25), g(f(d, 26, _26), g(f(d, 27, _27), g(f(d, 28, _28), g(f(d, 29, _29), g(f(d, 30, _30), g(f(d, 31, _31), f(d, 32, _32))))))))))))))))))))))))))))))))
#define M_REDUCEI3_33(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), g(f(d, 24, _24), g(f(d, 25, _25), g(f(d, 26, _26), g(f(d, 27, _27), g(f(d, 28, _28), g(f(d, 29, _29), g(f(d, 30, _30), g(f(d, 31, _31), g(f(d, 32, _32), f(d, 33, _33)))))))))))))))))))))))))))))))))
#define M_REDUCEI3_34(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), g(f(d, 24, _24), g(f(d, 25, _25), g(f(d, 26, _26), g(f(d, 27, _27), g(f(d, 28, _28), g(f(d, 29, _29), g(f(d, 30, _30), g(f(d, 31, _31), g(f(d, 32, _32), g(f(d, 33, _33), f(d, 34, _34))))))))))))))))))))))))))))))))))
#define M_REDUCEI3_35(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), g(f(d, 24, _24), g(f(d, 25, _25), g(f(d, 26, _26), g(f(d, 27, _27), g(f(d, 28, _28), g(f(d, 29, _29), g(f(d, 30, _30), g(f(d, 31, _31), g(f(d, 32, _32), g(f(d, 33, _33), g(f(d, 34, _34), f(d, 35, _35)))))))))))))))))))))))))))))))))))
#define M_REDUCEI3_36(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), g(f(d, 24, _24), g(f(d, 25, _25), g(f(d, 26, _26), g(f(d, 27, _27), g(f(d, 28, _28), g(f(d, 29, _29), g(f(d, 30, _30), g(f(d, 31, _31), g(f(d, 32, _32), g(f(d, 33, _33), g(f(d, 34, _34), g(f(d, 35, _35), f(d, 36, _36))))))))))))))))))))))))))))))))))))
#define M_REDUCEI3_37(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), g(f(d, 24, _24), g(f(d, 25, _25), g(f(d, 26, _26), g(f(d, 27, _27), g(f(d, 28, _28), g(f(d, 29, _29), g(f(d, 30, _30), g(f(d, 31, _31), g(f(d, 32, _32), g(f(d, 33, _33), g(f(d, 34, _34), g(f(d, 35, _35), g(f(d, 36, _36), f(d, 37, _37)))))))))))))))))))))))))))))))))))))
#define M_REDUCEI3_38(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), g(f(d, 24, _24), g(f(d, 25, _25), g(f(d, 26, _26), g(f(d, 27, _27), g(f(d, 28, _28), g(f(d, 29, _29), g(f(d, 30, _30), g(f(d, 31, _31), g(f(d, 32, _32), g(f(d, 33, _33), g(f(d, 34, _34), g(f(d, 35, _35), g(f(d, 36, _36), g(f(d, 37, _37), f(d, 38, _38))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI3_39(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), g(f(d, 24, _24), g(f(d, 25, _25), g(f(d, 26, _26), g(f(d, 27, _27), g(f(d, 28, _28), g(f(d, 29, _29), g(f(d, 30, _30), g(f(d, 31, _31), g(f(d, 32, _32), g(f(d, 33, _33), g(f(d, 34, _34), g(f(d, 35, _35), g(f(d, 36, _36), g(f(d, 37, _37), g(f(d, 38, _38), f(d, 39, _39)))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI3_40(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), g(f(d, 24, _24), g(f(d, 25, _25), g(f(d, 26, _26), g(f(d, 27, _27), g(f(d, 28, _28), g(f(d, 29, _29), g(f(d, 30, _30), g(f(d, 31, _31), g(f(d, 32, _32), g(f(d, 33, _33), g(f(d, 34, _34), g(f(d, 35, _35), g(f(d, 36, _36), g(f(d, 37, _37), g(f(d, 38, _38), g(f(d, 39, _39), f(d, 40, _40))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI3_41(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), g(f(d, 24, _24), g(f(d, 25, _25), g(f(d, 26, _26), g(f(d, 27, _27), g(f(d, 28, _28), g(f(d, 29, _29), g(f(d, 30, _30), g(f(d, 31, _31), g(f(d, 32, _32), g(f(d, 33, _33), g(f(d, 34, _34), g(f(d, 35, _35), g(f(d, 36, _36), g(f(d, 37, _37), g(f(d, 38, _38), g(f(d, 39, _39), g(f(d, 40, _40), f(d, 41, _41)))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI3_42(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), g(f(d, 24, _24), g(f(d, 25, _25), g(f(d, 26, _26), g(f(d, 27, _27), g(f(d, 28, _28), g(f(d, 29, _29), g(f(d, 30, _30), g(f(d, 31, _31), g(f(d, 32, _32), g(f(d, 33, _33), g(f(d, 34, _34), g(f(d, 35, _35), g(f(d, 36, _36), g(f(d, 37, _37), g(f(d, 38, _38), g(f(d, 39, _39), g(f(d, 40, _40), g(f(d, 41, _41), f(d, 42, _42))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI3_43(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), g(f(d, 24, _24), g(f(d, 25, _25), g(f(d, 26, _26), g(f(d, 27, _27), g(f(d, 28, _28), g(f(d, 29, _29), g(f(d, 30, _30), g(f(d, 31, _31), g(f(d, 32, _32), g(f(d, 33, _33), g(f(d, 34, _34), g(f(d, 35, _35), g(f(d, 36, _36), g(f(d, 37, _37), g(f(d, 38, _38), g(f(d, 39, _39), g(f(d, 40, _40), g(f(d, 41, _41), g(f(d, 42, _42), f(d, 43, _43)))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI3_44(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), g(f(d, 24, _24), g(f(d, 25, _25), g(f(d, 26, _26), g(f(d, 27, _27), g(f(d, 28, _28), g(f(d, 29, _29), g(f(d, 30, _30), g(f(d, 31, _31), g(f(d, 32, _32), g(f(d, 33, _33), g(f(d, 34, _34), g(f(d, 35, _35), g(f(d, 36, _36), g(f(d, 37, _37), g(f(d, 38, _38), g(f(d, 39, _39), g(f(d, 40, _40), g(f(d, 41, _41), g(f(d, 42, _42), g(f(d, 43, _43), f(d, 44, _44))))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI3_45(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), g(f(d, 24, _24), g(f(d, 25, _25), g(f(d, 26, _26), g(f(d, 27, _27), g(f(d, 28, _28), g(f(d, 29, _29), g(f(d, 30, _30), g(f(d, 31, _31), g(f(d, 32, _32), g(f(d, 33, _33), g(f(d, 34, _34), g(f(d, 35, _35), g(f(d, 36, _36), g(f(d, 37, _37), g(f(d, 38, _38), g(f(d, 39, _39), g(f(d, 40, _40), g(f(d, 41, _41), g(f(d, 42, _42), g(f(d, 43, _43), g(f(d, 44, _44), f(d, 45, _45)))))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI3_46(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), g(f(d, 24, _24), g(f(d, 25, _25), g(f(d, 26, _26), g(f(d, 27, _27), g(f(d, 28, _28), g(f(d, 29, _29), g(f(d, 30, _30), g(f(d, 31, _31), g(f(d, 32, _32), g(f(d, 33, _33), g(f(d, 34, _34), g(f(d, 35, _35), g(f(d, 36, _36), g(f(d, 37, _37), g(f(d, 38, _38), g(f(d, 39, _39), g(f(d, 40, _40), g(f(d, 41, _41), g(f(d, 42, _42), g(f(d, 43, _43), g(f(d, 44, _44), g(f(d, 45, _45), f(d, 46, _46))))))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI3_47(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), g(f(d, 24, _24), g(f(d, 25, _25), g(f(d, 26, _26), g(f(d, 27, _27), g(f(d, 28, _28), g(f(d, 29, _29), g(f(d, 30, _30), g(f(d, 31, _31), g(f(d, 32, _32), g(f(d, 33, _33), g(f(d, 34, _34), g(f(d, 35, _35), g(f(d, 36, _36), g(f(d, 37, _37), g(f(d, 38, _38), g(f(d, 39, _39), g(f(d, 40, _40), g(f(d, 41, _41), g(f(d, 42, _42), g(f(d, 43, _43), g(f(d, 44, _44), g(f(d, 45, _45), g(f(d, 46, _46), f(d, 47, _47)))))))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI3_48(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), g(f(d, 24, _24), g(f(d, 25, _25), g(f(d, 26, _26), g(f(d, 27, _27), g(f(d, 28, _28), g(f(d, 29, _29), g(f(d, 30, _30), g(f(d, 31, _31), g(f(d, 32, _32), g(f(d, 33, _33), g(f(d, 34, _34), g(f(d, 35, _35), g(f(d, 36, _36), g(f(d, 37, _37), g(f(d, 38, _38), g(f(d, 39, _39), g(f(d, 40, _40), g(f(d, 41, _41), g(f(d, 42, _42), g(f(d, 43, _43), g(f(d, 44, _44), g(f(d, 45, _45), g(f(d, 46, _46), g(f(d, 47, _47), f(d, 48, _48))))))))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI3_49(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), g(f(d, 24, _24), g(f(d, 25, _25), g(f(d, 26, _26), g(f(d, 27, _27), g(f(d, 28, _28), g(f(d, 29, _29), g(f(d, 30, _30), g(f(d, 31, _31), g(f(d, 32, _32), g(f(d, 33, _33), g(f(d, 34, _34), g(f(d, 35, _35), g(f(d, 36, _36), g(f(d, 37, _37), g(f(d, 38, _38), g(f(d, 39, _39), g(f(d, 40, _40), g(f(d, 41, _41), g(f(d, 42, _42), g(f(d, 43, _43), g(f(d, 44, _44), g(f(d, 45, _45), g(f(d, 46, _46), g(f(d, 47, _47), g(f(d, 48, _48), f(d, 49, _49)))))))))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI3_50(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), g(f(d, 24, _24), g(f(d, 25, _25), g(f(d, 26, _26), g(f(d, 27, _27), g(f(d, 28, _28), g(f(d, 29, _29), g(f(d, 30, _30), g(f(d, 31, _31), g(f(d, 32, _32), g(f(d, 33, _33), g(f(d, 34, _34), g(f(d, 35, _35), g(f(d, 36, _36), g(f(d, 37, _37), g(f(d, 38, _38), g(f(d, 39, _39), g(f(d, 40, _40), g(f(d, 41, _41), g(f(d, 42, _42), g(f(d, 43, _43), g(f(d, 44, _44), g(f(d, 45, _45), g(f(d, 46, _46), g(f(d, 47, _47), g(f(d, 48, _48), g(f(d, 49, _49), f(d, 50, _50))))))))))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI3_51(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), g(f(d, 24, _24), g(f(d, 25, _25), g(f(d, 26, _26), g(f(d, 27, _27), g(f(d, 28, _28), g(f(d, 29, _29), g(f(d, 30, _30), g(f(d, 31, _31), g(f(d, 32, _32), g(f(d, 33, _33), g(f(d, 34, _34), g(f(d, 35, _35), g(f(d, 36, _36), g(f(d, 37, _37), g(f(d, 38, _38), g(f(d, 39, _39), g(f(d, 40, _40), g(f(d, 41, _41), g(f(d, 42, _42), g(f(d, 43, _43), g(f(d, 44, _44), g(f(d, 45, _45), g(f(d, 46, _46), g(f(d, 47, _47), g(f(d, 48, _48), g(f(d, 49, _49), g(f(d, 50, _50), f(d, 51, _51)))))))))))))))))))))))))))))))))))))))))))))))))))
#define M_REDUCEI3_52(f,g,d,_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52) g(f(d, 1, _1), g(f(d, 2, _2), g(f(d, 3, _3), g(f(d, 4, _4), g(f(d, 5, _5), g(f(d, 6, _6), g(f(d, 7, _7), g(f(d, 8, _8), g(f(d, 9, _9), g(f(d, 10, _10), g(f(d, 11, _11), g(f(d, 12, _12), g(f(d, 13, _13), g(f(d, 14, _14), g(f(d, 15, _15), g(f(d, 16, _16), g(f(d, 17, _17), g(f(d, 18, _18), g(f(d, 19, _19), g(f(d, 20, _20), g(f(d, 21, _21), g(f(d, 22, _22), g(f(d, 23, _23), g(f(d, 24, _24), g(f(d, 25, _25), g(f(d, 26, _26), g(f(d, 27, _27), g(f(d, 28, _28), g(f(d, 29, _29), g(f(d, 30, _30), g(f(d, 31, _31), g(f(d, 32, _32), g(f(d, 33, _33), g(f(d, 34, _34), g(f(d, 35, _35), g(f(d, 36, _36), g(f(d, 37, _37), g(f(d, 38, _38), g(f(d, 39, _39), g(f(d, 40, _40), g(f(d, 41, _41), g(f(d, 42, _42), g(f(d, 43, _43), g(f(d, 44, _44), g(f(d, 45, _45), g(f(d, 46, _46), g(f(d, 47, _47), g(f(d, 48, _48), g(f(d, 49, _49), g(f(d, 50, _50), g(f(d, 51, _51), f(d, 52, _52))))))))))))))))))))))))))))))))))))))))))))))))))))


/* M_MAP_C(f, a, b, c) ==> f(a), f(b), f(c) */
#define M_MAP_C(f, ...)   M_REDUCE(f, M_ID, __VA_ARGS__)

/* M_MAP2_C(f, d, a, b, c) ==> f(d, a), f(d, b), f(d, c) */
#define M_MAP2_C(f, d, ...)   M_REDUCE2(f, M_MAP2_C_ID, d, __VA_ARGS__)
#define M_MAP2_C_ID(...)      __VA_ARGS__

/* M_MAP3_C(f, d, a, b, c) ==> f(d, a), f(d, b), f(d, c) */
#define M_MAP3_C(f, d, ...)   M_REDUCE3(f, M_MAP3_C_ID, d, __VA_ARGS__)
#define M_MAP3_C_ID(...)      __VA_ARGS__


/* Sequence of numerical */
/* Example: 
   M_SEQ(init,end)==>init, init+1, ...end 
   M_MAP(f, M_SEQ(init, end)) ==> f(init) f(init+1) .... f(end) 
   M_MAP2(f, d, M_SEQ(init, end))
*/
#define M_SEQ(init, end) M_MID_ARGS(init, M_INC(M_SUB(end, init)), 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52)

/* Replicate the value 'N' times */
#define M_REPLICATE(N, value)                                                 \
  M_MAP2(M_RET_ARG1, value, M_SEQ(1, N))

/* Replicate the value 'N' times, separated by commas */
#define M_REPLICATE_C(N, value)                                               \
  M_MAP2_C(M_RET_ARG1, value, M_SEQ(1, N))

/* Filter the elements of the list so that only the elements
   that match the function f are returned.
   f takes as argument (extra, item) and shall return 0 or 1. */
#define M_FILTER(f, extra, ...)                                               \
  M_MAP2( M_FILTER_00, (f, extra), __VA_ARGS__)

/* Filter the elements of the list so that only the elements
   that match the function f are returned.
   f takes as argument (extra, item) and shall return 0 or 1. */
#define M_FILTER_C(f, extra, ...)                                             \
  M_REMOVE_PARENTHESIS (M_REDUCE2(M_FILTER_00, M_CAT_ARGLIST, (f, extra), __VA_ARGS__) )

/* d is (filter_function, extra_value) item is the element ot test */
#define M_FILTER_00(d, item)                                                  \
  M_IF(M_PAIR_1 d(M_PAIR_2 d, item))(item, )

/* Concatene two arglists 'a' and 'b',
   handling the case of empty arglist
   handling the cases where the arglists are within parenthesis.
   and returning an arglist within parenthesis if it concats.
   (internal macro)
*/
#define M_CAT_ARGLIST(a, b)                                                   \
  M_IF_EMPTY(a)(b, M_IF_EMPTY(b)(a,( M_REMOVE_PARENTHESIS (a) , M_REMOVE_PARENTHESIS (b))))

/* Merge two arglists 'a' and 'b'
   (a1, a2, a3) and (b1, b2, b3) ==> ( (a1, b1) , (b1, b2), (a3, b3) )
   They shall be non empty and of the same size.
   (internal macro)
   Generated by:
   for i in $(seq  52) ; do printf "#define M_MERGE_ARGLIST_%d_%d(" $i $i ; for j in $(seq 1 $i) ; do printf "a%d, " $j ; done ; for j in $(seq 1 $i) ; do printf "b%d" $j ; if test $i != $j ; then printf ", " ; fi ; done ; printf ") " ; for j in $(seq 1 $i) ; do printf "(a%d, b%d) " $j $j ; if (test $i != $j ) ; then printf ", " ; fi ; done ; printf "\n" ; done
*/
#define M_MERGE_ARGLIST(a, b) ( M_APPLY( M_C4(M_MERGE_ARGLIST_, M_NARGS a, _, M_NARGS b), M_OPFLAT a, M_OPFLAT b) )
#define M_MERGE_ARGLIST_1_1(a1, b1) (a1, b1) 
#define M_MERGE_ARGLIST_2_2(a1, a2, b1, b2) (a1, b1) , (a2, b2) 
#define M_MERGE_ARGLIST_3_3(a1, a2, a3, b1, b2, b3) (a1, b1) , (a2, b2) , (a3, b3) 
#define M_MERGE_ARGLIST_4_4(a1, a2, a3, a4, b1, b2, b3, b4) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) 
#define M_MERGE_ARGLIST_5_5(a1, a2, a3, a4, a5, b1, b2, b3, b4, b5) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) 
#define M_MERGE_ARGLIST_6_6(a1, a2, a3, a4, a5, a6, b1, b2, b3, b4, b5, b6) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) 
#define M_MERGE_ARGLIST_7_7(a1, a2, a3, a4, a5, a6, a7, b1, b2, b3, b4, b5, b6, b7) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) 
#define M_MERGE_ARGLIST_8_8(a1, a2, a3, a4, a5, a6, a7, a8, b1, b2, b3, b4, b5, b6, b7, b8) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) 
#define M_MERGE_ARGLIST_9_9(a1, a2, a3, a4, a5, a6, a7, a8, a9, b1, b2, b3, b4, b5, b6, b7, b8, b9) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) 
#define M_MERGE_ARGLIST_10_10(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) 
#define M_MERGE_ARGLIST_11_11(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) 
#define M_MERGE_ARGLIST_12_12(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) 
#define M_MERGE_ARGLIST_13_13(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) 
#define M_MERGE_ARGLIST_14_14(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) 
#define M_MERGE_ARGLIST_15_15(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) 
#define M_MERGE_ARGLIST_16_16(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) 
#define M_MERGE_ARGLIST_17_17(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) 
#define M_MERGE_ARGLIST_18_18(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) 
#define M_MERGE_ARGLIST_19_19(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) 
#define M_MERGE_ARGLIST_20_20(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) 
#define M_MERGE_ARGLIST_21_21(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) 
#define M_MERGE_ARGLIST_22_22(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) 
#define M_MERGE_ARGLIST_23_23(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) 
#define M_MERGE_ARGLIST_24_24(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) 
#define M_MERGE_ARGLIST_25_25(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) , (a25, b25) 
#define M_MERGE_ARGLIST_26_26(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) , (a25, b25) , (a26, b26) 
#define M_MERGE_ARGLIST_27_27(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) , (a25, b25) , (a26, b26) , (a27, b27) 
#define M_MERGE_ARGLIST_28_28(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) , (a25, b25) , (a26, b26) , (a27, b27) , (a28, b28) 
#define M_MERGE_ARGLIST_29_29(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28, b29) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) , (a25, b25) , (a26, b26) , (a27, b27) , (a28, b28) , (a29, b29) 
#define M_MERGE_ARGLIST_30_30(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28, b29, b30) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) , (a25, b25) , (a26, b26) , (a27, b27) , (a28, b28) , (a29, b29) , (a30, b30) 
#define M_MERGE_ARGLIST_31_31(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28, b29, b30, b31) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) , (a25, b25) , (a26, b26) , (a27, b27) , (a28, b28) , (a29, b29) , (a30, b30) , (a31, b31) 
#define M_MERGE_ARGLIST_32_32(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28, b29, b30, b31, b32) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) , (a25, b25) , (a26, b26) , (a27, b27) , (a28, b28) , (a29, b29) , (a30, b30) , (a31, b31) , (a32, b32) 
#define M_MERGE_ARGLIST_33_33(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, a33, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28, b29, b30, b31, b32, b33) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) , (a25, b25) , (a26, b26) , (a27, b27) , (a28, b28) , (a29, b29) , (a30, b30) , (a31, b31) , (a32, b32) , (a33, b33) 
#define M_MERGE_ARGLIST_34_34(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, a33, a34, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28, b29, b30, b31, b32, b33, b34) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) , (a25, b25) , (a26, b26) , (a27, b27) , (a28, b28) , (a29, b29) , (a30, b30) , (a31, b31) , (a32, b32) , (a33, b33) , (a34, b34) 
#define M_MERGE_ARGLIST_35_35(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, a33, a34, a35, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28, b29, b30, b31, b32, b33, b34, b35) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) , (a25, b25) , (a26, b26) , (a27, b27) , (a28, b28) , (a29, b29) , (a30, b30) , (a31, b31) , (a32, b32) , (a33, b33) , (a34, b34) , (a35, b35) 
#define M_MERGE_ARGLIST_36_36(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, a33, a34, a35, a36, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28, b29, b30, b31, b32, b33, b34, b35, b36) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) , (a25, b25) , (a26, b26) , (a27, b27) , (a28, b28) , (a29, b29) , (a30, b30) , (a31, b31) , (a32, b32) , (a33, b33) , (a34, b34) , (a35, b35) , (a36, b36) 
#define M_MERGE_ARGLIST_37_37(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, a33, a34, a35, a36, a37, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28, b29, b30, b31, b32, b33, b34, b35, b36, b37) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) , (a25, b25) , (a26, b26) , (a27, b27) , (a28, b28) , (a29, b29) , (a30, b30) , (a31, b31) , (a32, b32) , (a33, b33) , (a34, b34) , (a35, b35) , (a36, b36) , (a37, b37) 
#define M_MERGE_ARGLIST_38_38(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, a33, a34, a35, a36, a37, a38, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28, b29, b30, b31, b32, b33, b34, b35, b36, b37, b38) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) , (a25, b25) , (a26, b26) , (a27, b27) , (a28, b28) , (a29, b29) , (a30, b30) , (a31, b31) , (a32, b32) , (a33, b33) , (a34, b34) , (a35, b35) , (a36, b36) , (a37, b37) , (a38, b38) 
#define M_MERGE_ARGLIST_39_39(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, a33, a34, a35, a36, a37, a38, a39, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28, b29, b30, b31, b32, b33, b34, b35, b36, b37, b38, b39) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) , (a25, b25) , (a26, b26) , (a27, b27) , (a28, b28) , (a29, b29) , (a30, b30) , (a31, b31) , (a32, b32) , (a33, b33) , (a34, b34) , (a35, b35) , (a36, b36) , (a37, b37) , (a38, b38) , (a39, b39) 
#define M_MERGE_ARGLIST_40_40(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, a33, a34, a35, a36, a37, a38, a39, a40, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28, b29, b30, b31, b32, b33, b34, b35, b36, b37, b38, b39, b40) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) , (a25, b25) , (a26, b26) , (a27, b27) , (a28, b28) , (a29, b29) , (a30, b30) , (a31, b31) , (a32, b32) , (a33, b33) , (a34, b34) , (a35, b35) , (a36, b36) , (a37, b37) , (a38, b38) , (a39, b39) , (a40, b40) 
#define M_MERGE_ARGLIST_41_41(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, a33, a34, a35, a36, a37, a38, a39, a40, a41, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28, b29, b30, b31, b32, b33, b34, b35, b36, b37, b38, b39, b40, b41) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) , (a25, b25) , (a26, b26) , (a27, b27) , (a28, b28) , (a29, b29) , (a30, b30) , (a31, b31) , (a32, b32) , (a33, b33) , (a34, b34) , (a35, b35) , (a36, b36) , (a37, b37) , (a38, b38) , (a39, b39) , (a40, b40) , (a41, b41) 
#define M_MERGE_ARGLIST_42_42(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, a33, a34, a35, a36, a37, a38, a39, a40, a41, a42, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28, b29, b30, b31, b32, b33, b34, b35, b36, b37, b38, b39, b40, b41, b42) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) , (a25, b25) , (a26, b26) , (a27, b27) , (a28, b28) , (a29, b29) , (a30, b30) , (a31, b31) , (a32, b32) , (a33, b33) , (a34, b34) , (a35, b35) , (a36, b36) , (a37, b37) , (a38, b38) , (a39, b39) , (a40, b40) , (a41, b41) , (a42, b42) 
#define M_MERGE_ARGLIST_43_43(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, a33, a34, a35, a36, a37, a38, a39, a40, a41, a42, a43, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28, b29, b30, b31, b32, b33, b34, b35, b36, b37, b38, b39, b40, b41, b42, b43) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) , (a25, b25) , (a26, b26) , (a27, b27) , (a28, b28) , (a29, b29) , (a30, b30) , (a31, b31) , (a32, b32) , (a33, b33) , (a34, b34) , (a35, b35) , (a36, b36) , (a37, b37) , (a38, b38) , (a39, b39) , (a40, b40) , (a41, b41) , (a42, b42) , (a43, b43) 
#define M_MERGE_ARGLIST_44_44(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, a33, a34, a35, a36, a37, a38, a39, a40, a41, a42, a43, a44, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28, b29, b30, b31, b32, b33, b34, b35, b36, b37, b38, b39, b40, b41, b42, b43, b44) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) , (a25, b25) , (a26, b26) , (a27, b27) , (a28, b28) , (a29, b29) , (a30, b30) , (a31, b31) , (a32, b32) , (a33, b33) , (a34, b34) , (a35, b35) , (a36, b36) , (a37, b37) , (a38, b38) , (a39, b39) , (a40, b40) , (a41, b41) , (a42, b42) , (a43, b43) , (a44, b44) 
#define M_MERGE_ARGLIST_45_45(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, a33, a34, a35, a36, a37, a38, a39, a40, a41, a42, a43, a44, a45, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28, b29, b30, b31, b32, b33, b34, b35, b36, b37, b38, b39, b40, b41, b42, b43, b44, b45) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) , (a25, b25) , (a26, b26) , (a27, b27) , (a28, b28) , (a29, b29) , (a30, b30) , (a31, b31) , (a32, b32) , (a33, b33) , (a34, b34) , (a35, b35) , (a36, b36) , (a37, b37) , (a38, b38) , (a39, b39) , (a40, b40) , (a41, b41) , (a42, b42) , (a43, b43) , (a44, b44) , (a45, b45) 
#define M_MERGE_ARGLIST_46_46(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, a33, a34, a35, a36, a37, a38, a39, a40, a41, a42, a43, a44, a45, a46, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28, b29, b30, b31, b32, b33, b34, b35, b36, b37, b38, b39, b40, b41, b42, b43, b44, b45, b46) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) , (a25, b25) , (a26, b26) , (a27, b27) , (a28, b28) , (a29, b29) , (a30, b30) , (a31, b31) , (a32, b32) , (a33, b33) , (a34, b34) , (a35, b35) , (a36, b36) , (a37, b37) , (a38, b38) , (a39, b39) , (a40, b40) , (a41, b41) , (a42, b42) , (a43, b43) , (a44, b44) , (a45, b45) , (a46, b46) 
#define M_MERGE_ARGLIST_47_47(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, a33, a34, a35, a36, a37, a38, a39, a40, a41, a42, a43, a44, a45, a46, a47, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28, b29, b30, b31, b32, b33, b34, b35, b36, b37, b38, b39, b40, b41, b42, b43, b44, b45, b46, b47) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) , (a25, b25) , (a26, b26) , (a27, b27) , (a28, b28) , (a29, b29) , (a30, b30) , (a31, b31) , (a32, b32) , (a33, b33) , (a34, b34) , (a35, b35) , (a36, b36) , (a37, b37) , (a38, b38) , (a39, b39) , (a40, b40) , (a41, b41) , (a42, b42) , (a43, b43) , (a44, b44) , (a45, b45) , (a46, b46) , (a47, b47) 
#define M_MERGE_ARGLIST_48_48(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, a33, a34, a35, a36, a37, a38, a39, a40, a41, a42, a43, a44, a45, a46, a47, a48, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28, b29, b30, b31, b32, b33, b34, b35, b36, b37, b38, b39, b40, b41, b42, b43, b44, b45, b46, b47, b48) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) , (a25, b25) , (a26, b26) , (a27, b27) , (a28, b28) , (a29, b29) , (a30, b30) , (a31, b31) , (a32, b32) , (a33, b33) , (a34, b34) , (a35, b35) , (a36, b36) , (a37, b37) , (a38, b38) , (a39, b39) , (a40, b40) , (a41, b41) , (a42, b42) , (a43, b43) , (a44, b44) , (a45, b45) , (a46, b46) , (a47, b47) , (a48, b48) 
#define M_MERGE_ARGLIST_49_49(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, a33, a34, a35, a36, a37, a38, a39, a40, a41, a42, a43, a44, a45, a46, a47, a48, a49, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28, b29, b30, b31, b32, b33, b34, b35, b36, b37, b38, b39, b40, b41, b42, b43, b44, b45, b46, b47, b48, b49) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) , (a25, b25) , (a26, b26) , (a27, b27) , (a28, b28) , (a29, b29) , (a30, b30) , (a31, b31) , (a32, b32) , (a33, b33) , (a34, b34) , (a35, b35) , (a36, b36) , (a37, b37) , (a38, b38) , (a39, b39) , (a40, b40) , (a41, b41) , (a42, b42) , (a43, b43) , (a44, b44) , (a45, b45) , (a46, b46) , (a47, b47) , (a48, b48) , (a49, b49) 
#define M_MERGE_ARGLIST_50_50(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, a33, a34, a35, a36, a37, a38, a39, a40, a41, a42, a43, a44, a45, a46, a47, a48, a49, a50, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28, b29, b30, b31, b32, b33, b34, b35, b36, b37, b38, b39, b40, b41, b42, b43, b44, b45, b46, b47, b48, b49, b50) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) , (a25, b25) , (a26, b26) , (a27, b27) , (a28, b28) , (a29, b29) , (a30, b30) , (a31, b31) , (a32, b32) , (a33, b33) , (a34, b34) , (a35, b35) , (a36, b36) , (a37, b37) , (a38, b38) , (a39, b39) , (a40, b40) , (a41, b41) , (a42, b42) , (a43, b43) , (a44, b44) , (a45, b45) , (a46, b46) , (a47, b47) , (a48, b48) , (a49, b49) , (a50, b50) 
#define M_MERGE_ARGLIST_51_51(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, a33, a34, a35, a36, a37, a38, a39, a40, a41, a42, a43, a44, a45, a46, a47, a48, a49, a50, a51, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28, b29, b30, b31, b32, b33, b34, b35, b36, b37, b38, b39, b40, b41, b42, b43, b44, b45, b46, b47, b48, b49, b50, b51) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) , (a25, b25) , (a26, b26) , (a27, b27) , (a28, b28) , (a29, b29) , (a30, b30) , (a31, b31) , (a32, b32) , (a33, b33) , (a34, b34) , (a35, b35) , (a36, b36) , (a37, b37) , (a38, b38) , (a39, b39) , (a40, b40) , (a41, b41) , (a42, b42) , (a43, b43) , (a44, b44) , (a45, b45) , (a46, b46) , (a47, b47) , (a48, b48) , (a49, b49) , (a50, b50) , (a51, b51) 
#define M_MERGE_ARGLIST_52_52(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, a33, a34, a35, a36, a37, a38, a39, a40, a41, a42, a43, a44, a45, a46, a47, a48, a49, a50, a51, a52, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28, b29, b30, b31, b32, b33, b34, b35, b36, b37, b38, b39, b40, b41, b42, b43, b44, b45, b46, b47, b48, b49, b50, b51, b52) (a1, b1) , (a2, b2) , (a3, b3) , (a4, b4) , (a5, b5) , (a6, b6) , (a7, b7) , (a8, b8) , (a9, b9) , (a10, b10) , (a11, b11) , (a12, b12) , (a13, b13) , (a14, b14) , (a15, b15) , (a16, b16) , (a17, b17) , (a18, b18) , (a19, b19) , (a20, b20) , (a21, b21) , (a22, b22) , (a23, b23) , (a24, b24) , (a25, b25) , (a26, b26) , (a27, b27) , (a28, b28) , (a29, b29) , (a30, b30) , (a31, b31) , (a32, b32) , (a33, b33) , (a34, b34) , (a35, b35) , (a36, b36) , (a37, b37) , (a38, b38) , (a39, b39) , (a40, b40) , (a41, b41) , (a42, b42) , (a43, b43) , (a44, b44) , (a45, b45) , (a46, b46) , (a47, b47) , (a48, b48) , (a49, b49) , (a50, b50) , (a51, b51) , (a52, b52) 


/* Remove the parenthesis if needed
   (internal macro) */
#define M_REMOVE_PARENTHESIS(...)                                             \
  M_IF(M_PARENTHESIS_P(__VA_ARGS__))(M_REMOVE_PARENTHESIS_2, M_REMOVE_PARENTHESIS_3)(__VA_ARGS__)
#define M_REMOVE_PARENTHESIS_2(...) M_OPFLAT __VA_ARGS__
#define M_REMOVE_PARENTHESIS_3(...) __VA_ARGS__

/* Return the input (delay evaluation) */
#define M_ID(...)                 __VA_ARGS__

/* Globber the input */
#define M_EAT(...)

/* M_NARGS: Return number of argument.
   (don't work for empty arg) */
#define M_NARGS(...)                                                          \
  M_RET_ARG76(__VA_ARGS__, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65,        \
              64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53,                 \
              52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42,                     \
              41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27,     \
              26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,                     \
              15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, useless)

/* If the number of arguments is 1 */
#define M_IF_NARGS_EQ1(...)           M_IF(M_EQUAL(M_NARGS(__VA_ARGS__), 1))

/* If the number of arguments is 2 */
#define M_IF_NARGS_EQ2(...)           M_IF(M_EQUAL(M_NARGS(__VA_ARGS__), 2))

/* If NDEBUG macro is defined
        M_IF_DEBUG(code if NDEBUG is not defined)
   Note: not 100% robust */
#define M_TEST_NDEBUG_P()           M_C3(M_, NDEBUG, _TEST)
#define M_NDEBUG_TEST               0
#define M_IF_DEBUG(a)               M_IF(M_TEST_NDEBUG_P())(,a)


/* If the Function Object is included, expands the code,
   otherwise do nothing.
   M_FUNCOBJ_IS_NOT_DEFINED is defined to 0.
   NOTE: M_IF is the variable is not defined assummes yes.
*/
#define M_IF_FUNCOBJ(a)             M_IF(M_FUNCOBJ_IS_NOT_DEFINED)( ,a)


/* Helper macro to redefine a function with a default value :
   If there is only one variable as the __VA_ARGS__, print
   __VA_ARGS__ then ', value', instead only __VA_ARGS__.
   Example:
    int f(int a, int b);
    #define f(...) M_APPLY(f, M_IF_DEFAULT1(0, __VA_ARGS__))
   This need to be called within a M_APPLY macro.
*/
#define M_IF_DEFAULT1(value, ...)                                             \
  __VA_ARGS__ M_IF_NARGS_EQ1(__VA_ARGS__)(M_DEFERRED_COMMA value, )

/* Helper macro to redefine a function with a default values:
   Give the number of expected arguments, the value list of the
   default argument, and the arguments.
   It will complete the arguments with the value of the default
   argument to complete up to 'expected' arguments.
   Example:
   #define f(...) f(M_DEFAULT_ARGS(4, (0, 1, NULL), __VA_ARGS__))
*/
#define M_DEFAULT_ARGS2(expected, value, ...)                                 \
  __VA_ARGS__ M_IF(M_NOTEQUAL(M_NARGS(__VA_ARGS__), expected))(M_DEFERRED_COMMA, ) \
    M_REVERSE(M_KEEP_ARGS(M_SUB(expected, M_NARGS(__VA_ARGS__)), M_REVERSE value))
#define M_DEFAULT_ARGS_EVAL(...) __VA_ARGS__
#define M_DEFAULT_ARGS( ...) M_DEFAULT_ARGS_EVAL(M_DEFAULT_ARGS2(__VA_ARGS__))

/* NOTEQUAL(val1,val2) with val from [0 to 52[
   Return 1 or 0 if val1=val2
 */
#define M_NOTEQUAL_0_0 0
#define M_NOTEQUAL_1_1 0
#define M_NOTEQUAL_2_2 0
#define M_NOTEQUAL_3_3 0
#define M_NOTEQUAL_4_4 0
#define M_NOTEQUAL_5_5 0
#define M_NOTEQUAL_6_6 0
#define M_NOTEQUAL_7_7 0
#define M_NOTEQUAL_8_8 0
#define M_NOTEQUAL_9_9 0
#define M_NOTEQUAL_10_10 0
#define M_NOTEQUAL_11_11 0
#define M_NOTEQUAL_12_12 0
#define M_NOTEQUAL_13_13 0
#define M_NOTEQUAL_14_14 0
#define M_NOTEQUAL_15_15 0
#define M_NOTEQUAL_16_16 0
#define M_NOTEQUAL_17_17 0
#define M_NOTEQUAL_18_18 0
#define M_NOTEQUAL_19_19 0
#define M_NOTEQUAL_20_20 0
#define M_NOTEQUAL_21_21 0
#define M_NOTEQUAL_22_22 0
#define M_NOTEQUAL_23_23 0
#define M_NOTEQUAL_24_24 0
#define M_NOTEQUAL_25_25 0
#define M_NOTEQUAL_26_26 0
#define M_NOTEQUAL_27_27 0
#define M_NOTEQUAL_28_28 0
#define M_NOTEQUAL_29_29 0
#define M_NOTEQUAL_30_30 0
#define M_NOTEQUAL_31_31 0
#define M_NOTEQUAL_32_32 0
#define M_NOTEQUAL_33_33 0
#define M_NOTEQUAL_34_34 0
#define M_NOTEQUAL_35_35 0
#define M_NOTEQUAL_36_36 0
#define M_NOTEQUAL_37_37 0
#define M_NOTEQUAL_38_38 0
#define M_NOTEQUAL_39_39 0
#define M_NOTEQUAL_40_40 0
#define M_NOTEQUAL_41_41 0
#define M_NOTEQUAL_42_42 0
#define M_NOTEQUAL_43_43 0
#define M_NOTEQUAL_44_44 0
#define M_NOTEQUAL_45_45 0
#define M_NOTEQUAL_46_46 0
#define M_NOTEQUAL_47_47 0
#define M_NOTEQUAL_48_48 0
#define M_NOTEQUAL_49_49 0
#define M_NOTEQUAL_50_50 0
#define M_NOTEQUAL_51_51 0
#define M_NOTEQUAL_52_52 0
#define M_NOTEQUAL(x,y) M_BOOL(M_C4(M_NOTEQUAL_, x, _, y))

/* EQUAL(val1,val2)
   Return 1 if val1=val2 or 0
   Example: M_EQUAL(1,2) --> 0
 */
#define M_EQUAL(x,y) M_INV(M_NOTEQUAL(x,y))

/* Return 1 if a < b, 0 otherwise 
   a and b shall be in [0..M_MAX_NB_ARGUMENT] */
#define M_LESS_THAN_P(a, b) M_KEYWORD_P(M_UNDERFLOW, M_SUB(a, b))

/* Return 1 if a >= b, 0 otherwise 
   a and b shall be in [0..M_MAX_NB_ARGUMENT] */
#define M_GREATER_OR_EQUAL_P(a, b) M_INV(M_LESS_THAN_P(a, b))

/* Return 1 if a <= b, 0 otherwise 
   a and b shall be in [0..M_MAX_NB_ARGUMENT] */
#define M_LESS_OR_EQUAL_P(a, b) M_GREATER_OR_EQUAL_P(b, a)

/* Return 1 if a > b, 0 otherwise 
   a and b shall be in [0..M_MAX_NB_ARGUMENT] */
#define M_GREATER_THAN_P(a, b) M_LESS_THAN_P(b,a)

/* Revert the list given as parameter
   Example: M_REVERSE(a,b,c) --> c,b,a  */
/* Generated by:
   for i in $(seq 1 52) ; do printf "#define M_REVERSE_%d(" $i ; for j in $(seq 1 $i) ; do printf "_%d" $j ; if test $i -ne $j ; then printf "," ; fi ; done; printf ") " ; for j in $(seq $i -1 1) ; do printf "_%d" $j ; if test 1 -ne $j ; then printf "," ; fi ; done ; printf "\n" ; done */
#define M_REVERSE(...) M_C(M_REVERSE_, M_NARGS(__VA_ARGS__))(__VA_ARGS__)
#define M_REVERSE_0() ()
#define M_REVERSE_1(_1) _1
#define M_REVERSE_2(_1,_2) _2,_1
#define M_REVERSE_3(_1,_2,_3) _3,_2,_1
#define M_REVERSE_4(_1,_2,_3,_4) _4,_3,_2,_1
#define M_REVERSE_5(_1,_2,_3,_4,_5) _5,_4,_3,_2,_1
#define M_REVERSE_6(_1,_2,_3,_4,_5,_6) _6,_5,_4,_3,_2,_1
#define M_REVERSE_7(_1,_2,_3,_4,_5,_6,_7) _7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_8(_1,_2,_3,_4,_5,_6,_7,_8) _8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_9(_1,_2,_3,_4,_5,_6,_7,_8,_9) _9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_10(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10) _10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_11(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11) _11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_12(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12) _12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_13(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13) _13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_14(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14) _14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_15(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15) _15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_16(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16) _16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_17(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17) _17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_18(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18) _18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_19(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19) _19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_20(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20) _20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_21(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21) _21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_22(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22) _22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_23(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23) _23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_24(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24) _24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_25(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25) _25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_26(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26) _26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_27(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27) _27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_28(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28) _28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_29(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29) _29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_30(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30) _30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_31(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31) _31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_32(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32) _32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_33(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33) _33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_34(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34) _34,_33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_35(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35) _35,_34,_33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_36(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36) _36,_35,_34,_33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_37(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37) _37,_36,_35,_34,_33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_38(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38) _38,_37,_36,_35,_34,_33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_39(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39) _39,_38,_37,_36,_35,_34,_33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_40(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40) _40,_39,_38,_37,_36,_35,_34,_33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_41(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41) _41,_40,_39,_38,_37,_36,_35,_34,_33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_42(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42) _42,_41,_40,_39,_38,_37,_36,_35,_34,_33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_43(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43) _43,_42,_41,_40,_39,_38,_37,_36,_35,_34,_33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_44(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44) _44,_43,_42,_41,_40,_39,_38,_37,_36,_35,_34,_33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_45(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45) _45,_44,_43,_42,_41,_40,_39,_38,_37,_36,_35,_34,_33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_46(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46) _46,_45,_44,_43,_42,_41,_40,_39,_38,_37,_36,_35,_34,_33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_47(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47) _47,_46,_45,_44,_43,_42,_41,_40,_39,_38,_37,_36,_35,_34,_33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_48(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48) _48,_47,_46,_45,_44,_43,_42,_41,_40,_39,_38,_37,_36,_35,_34,_33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_49(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49) _49,_48,_47,_46,_45,_44,_43,_42,_41,_40,_39,_38,_37,_36,_35,_34,_33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_50(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50) _50,_49,_48,_47,_46,_45,_44,_43,_42,_41,_40,_39,_38,_37,_36,_35,_34,_33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_51(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51) _51,_50,_49,_48,_47,_46,_45,_44,_43,_42,_41,_40,_39,_38,_37,_36,_35,_34,_33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1
#define M_REVERSE_52(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52) _52,_51,_50,_49,_48,_47,_46,_45,_44,_43,_42,_41,_40,_39,_38,_37,_36,_35,_34,_33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1


/* Obsolete name : to be deleted */
#define M_INVERT(...) ( M_REVERSE (__VA_ARGS__) )




/***************************************************************/
/******************** Compile Times Macro **********************/
/***************************************************************/

/* C11 MACROS */

/* Maximum number of characters of an internal identifier (field name)
   including final null char. Bigger than 
   63 significant initial characters of C11 standard (§5.2.4.1)
   Can be overloaded by user if needed.
   NOTE: Used by variant & serial JSON to translate a field name into 
   a structure offset.
*/
#ifndef M_USE_IDENTIFIER_ALLOC
#define M_USE_IDENTIFIER_ALLOC 128 /* (including of final null char) */
#endif

/* Return the string format of a variable */
#define M_PRINTF_FORMAT(x)                                                    \
  _Generic(((void)0,(x)),                                                     \
           char: "%c",                                                        \
           bool: "%d",                                                        \
           signed char: "%hhd",                                               \
           unsigned char: "%hhu",                                             \
           signed short: "%hd",                                               \
           unsigned short: "%hu",                                             \
           signed int: "%d",                                                  \
           unsigned int: "%u",                                                \
           long int: "%ld",                                                   \
           unsigned long int: "%lu",                                          \
           long long int: "%lld",                                             \
           unsigned long long int: "%llu",                                    \
           float: "%f",                                                       \
           double: "%f",                                                      \
           long double: "%Lf",                                                \
           const char *: "%s",                                                \
           char *: "%s",                                                      \
           const void *: "%p",                                                \
           void *: "%p"                                                       \
           M_PRINTF_FORMAT_EXTEND() )

#define M_PRINTF_FORMAT_EXTEND() /* Nothing in m-core (see m-string.h) */

/* IF FILE is supported */
#if M_USE_STDIO

/* Define internal wrappers around Annex K functions or classic functions for:
 * - fopen
 * - fscanf
 * - strncpy
 * - strncat
 *
 * There is no real usage outside of MSVC of Annex K,
 * so the real standard compliant Annex K is not taken into account
 * by this specific wrapper.
 *
 * If Microsoft Visual Studio C Library
 * and the user wants to use the Annex K.
 * ==> Use Annex K like functions to avoid warnings.
 * 
 * Only these functions produce warning,
 * so we keep the wrapper as simple as possible by including only then.
 */
#if defined(_MSC_VER) && defined(__STDC_WANT_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__

/* Wrapper around fopen_s */
static inline FILE *
m_core_fopen(const char filename[], const char opt[])
{
    FILE *f;
    int err = fopen_s(&f, filename, opt);
    if (err) return NULL;
    return f;
}

/* Wrapper around strncpy */
#define m_core_strncpy(s1, alloc, s2, size) strncpy_s(s1, alloc, s2, size)

/* Wrapper around strncat */
#define m_core_strncat(s1, alloc, s2, size) strncat_s(s1, alloc, s2, size)

/* Wrapper around fscanf_s */
#define m_core_fscanf(...) fscanf_s(__VA_ARGS__)
/* Macro to be used in m_core_fscanf for argument associated
 * to the format %c, %s or %[
 * in order to specify the size of the argument */
#define m_core_arg_size(arg, size) arg, (unsigned) size

#else          /* Use classic C functions */

/* Wrapper around fopen */
#define m_core_fopen(...) fopen(__VA_ARGS__)
/* Wrapper around strncpy */
#define m_core_strncpy(s1, alloc, s2, size) strncpy(s1, s2, size)
/* Wrapper around strncat */
#define m_core_strncat(s1, alloc, s2, size) strncat(s1, s2, size)
/* Wrapper around fscanf */
#define m_core_fscanf(...) fscanf(__VA_ARGS__)

/* Macro to be used in m_core_fscanf for argument associated
 * to the format %c, %s or %[
 * in order to specify the size of the argument */
#define m_core_arg_size(arg, size) arg

#endif

/* Can be overloaded by m-string to support string output too */
#define M_CORE_PRINTF_ARG(x) x

/* Print a C variable if it is a standard type (or extension) to stdout
  If a variable is extended (i.e. like (x, type) ) it will use the 
  method associated to the OUT_STR operator.
*/
#define M_PRINT_ARG(x)                                                        \
  M_IF(M_PARENTHESIS_P(x))                                                    \
    ( M_FPRINT_ARG_OUT_STR(stdout, M_PAIR_2 x, M_PAIR_1 x),                   \
    printf(M_PRINTF_FORMAT(x), M_CORE_PRINTF_ARG(x) ) )

/* Print a C variable if it is a standard type to the given file 'f'.
  If a variable is extended (i.e. like (x, type) ) it will use the 
  method associated to the OUT_STR operator.
*/
#define M_FPRINT_ARG(f, x)                                                    \
  M_IF(M_PARENTHESIS_P(x))                                                    \
    ( M_FPRINT_ARG_OUT_STR(f, M_PAIR_2 x, M_PAIR_1 x),                        \
    fprintf(f, M_PRINTF_FORMAT(x), M_CORE_PRINTF_ARG(x) ) )

/* Internal macro to call the OUT_STR method */
#define M_FPRINT_ARG_OUT_STR(file, oplist, var)                               \
  M_CALL_OUT_STR( M_GLOBAL_OPLIST(oplist) , file, var)

/* Get a C variable if it is a standard type from the given file 'f'.*/
#define M_FSCAN_ARG(xptr, f)                                                  \
  _Generic(((void)0,*(xptr)),                                                 \
           bool: m_core_fscan_bool(M_AS_TYPE(bool*,xptr), f),                 \
           char: m_core_fscan_char(M_AS_TYPE(char*,xptr), f),                 \
           signed char: m_core_fscan_schar(M_AS_TYPE(signed char*,xptr),f),   \
           unsigned char: m_core_fscan_uchar(M_AS_TYPE(unsigned char*,xptr),f), \
           signed short: m_core_fscan_sshort(M_AS_TYPE(signed short*,xptr),f), \
           unsigned short: m_core_fscan_ushort(M_AS_TYPE(unsigned short*,xptr), f), \
           signed int: m_core_fscan_sint(M_AS_TYPE(signed int*,xptr), f),     \
           unsigned int: m_core_fscan_uint(M_AS_TYPE(unsigned int*,xptr), f), \
           long int: m_core_fscan_slong(M_AS_TYPE(long*,xptr), f),            \
           unsigned long int: m_core_fscan_ulong(M_AS_TYPE(unsigned long*,xptr), f), \
           long long int: m_core_fscan_sllong(M_AS_TYPE(long long*,xptr), f), \
           unsigned long long int: m_core_fscan_ullong(M_AS_TYPE(unsigned long long*,xptr),f), \
           float: m_core_fscan_float(M_AS_TYPE(float*,xptr), f),              \
           double: m_core_fscan_double(M_AS_TYPE(double*,xptr), f),           \
           long double: m_core_fscan_ldouble(M_AS_TYPE(long double*,xptr),f), \
           const char *: false /* unsupported */,                             \
           char *: false /* unsupported */,                                   \
           const void *: false /* unsupported */,                             \
           void *: false /* unsupported */)

/* Internal wrappers used by M_FSCAN_ARG : */
static inline bool
m_core_fscan_bool (bool *ptr, FILE *f)
{
  int c = fgetc(f);
  *ptr = (c == '1');
  return (c == '0' || c == '1');
}

static inline bool
m_core_fscan_char (char *ptr, FILE *f)
{
  int c = fgetc(f);
  *ptr = (char) c;
  return c != EOF;
}

#define M_FSCAN_DEFAULT_TYPE_DEF(name, type, format)                          \
  static inline bool                                                          \
  name (type *ptr, FILE *f)                                                   \
  {                                                                           \
    return m_core_fscanf(f, format, ptr) == 1;                                \
  }
M_FSCAN_DEFAULT_TYPE_DEF(m_core_fscan_schar, signed char, "%hhd")
M_FSCAN_DEFAULT_TYPE_DEF(m_core_fscan_uchar, unsigned char, "%hhu")
M_FSCAN_DEFAULT_TYPE_DEF(m_core_fscan_sshort, signed short, "%hd")
M_FSCAN_DEFAULT_TYPE_DEF(m_core_fscan_ushort, unsigned short, "%hu")
M_FSCAN_DEFAULT_TYPE_DEF(m_core_fscan_sint, signed int, "%d")
M_FSCAN_DEFAULT_TYPE_DEF(m_core_fscan_uint, unsigned int, "%u")
M_FSCAN_DEFAULT_TYPE_DEF(m_core_fscan_slong, signed long, "%ld")
M_FSCAN_DEFAULT_TYPE_DEF(m_core_fscan_ulong, unsigned long, "%lu")
M_FSCAN_DEFAULT_TYPE_DEF(m_core_fscan_sllong, signed long long, "%lld")
M_FSCAN_DEFAULT_TYPE_DEF(m_core_fscan_ullong, unsigned long long, "%llu")
M_FSCAN_DEFAULT_TYPE_DEF(m_core_fscan_float, float, "%f")
M_FSCAN_DEFAULT_TYPE_DEF(m_core_fscan_double, double, "%lf")
M_FSCAN_DEFAULT_TYPE_DEF(m_core_fscan_ldouble, long double, "%Lf")

#endif

/* Transform a C variable into a string_t (needs m-string.h) */
#define M_GET_STRING_ARG(str, x, append)                                      \
  (append ? string_cat_printf : string_printf) (str, M_PRINTF_FORMAT(x), M_CORE_PRINTF_ARG(x))

/* No use of GET_STR if no inclusion of m-string */
#define M_GET_STR_METHOD_FOR_DEFAULT_TYPE /* */

/* Parse string of default type */
#define M_PARSE_DEFAULT_TYPE(x, str, endptr)                                  \
  _Generic(((void)0,*(x)),                                                    \
           char: m_core_parse_char(M_AS_TYPE(char*,x),str,endptr),            \
           bool: m_core_parse_bool(M_AS_TYPE(bool*,x),str,endptr),            \
           signed char: m_core_parse_schar(M_AS_TYPE(signed char*,x),str,endptr), \
           unsigned char: m_core_parse_uchar(M_AS_TYPE(unsigned char*,x),str,endptr), \
           signed short: m_core_parse_sshort(M_AS_TYPE(signed short*,x),str,endptr), \
           unsigned short: m_core_parse_ushort(M_AS_TYPE(unsigned short*,x),str,endptr), \
           signed int: m_core_parse_sint(M_AS_TYPE(signed int*,x),str,endptr), \
           unsigned int: m_core_parse_uint(M_AS_TYPE(unsigned int*,x),str,endptr), \
           signed long: m_core_parse_slong(M_AS_TYPE(signed long *,x),str,endptr), \
           unsigned long: m_core_parse_ulong(M_AS_TYPE(unsigned long*,x),str,endptr), \
           signed long long: m_core_parse_sllong(M_AS_TYPE(signed long long*,x),str,endptr), \
           unsigned long long: m_core_parse_ullong(M_AS_TYPE(unsigned long long*,x),str,endptr), \
           float: m_core_parse_float(M_AS_TYPE(float*,x),str,endptr),         \
           double: m_core_parse_double(M_AS_TYPE(double*,x),str,endptr),      \
           long double: m_core_parse_ldouble(M_AS_TYPE(long double*,x),str,endptr), \
           const char *: false /* not supported */,                           \
           char *: false /* not supported */,                                 \
           const void *: false /* not supported */,                           \
           void *: false /* not supported */)

/* Internal wrappers used by M_PARSE_DEFAULT_TYPE : */
static inline bool
m_core_parse_char (char *ptr, const char str[], const char **endptr)
{
    *ptr = *str++;
    if (endptr != NULL) *endptr = str;
    return true;
}

static inline bool
m_core_parse_bool (bool *ptr, const char str[], const char **endptr)
{
  char c = *str++;
  *ptr = (c == '1');
  if (endptr != NULL) *endptr = str;
  return (c == '0' || c == '1');
}

#define M_PARSE_DEFAULT_TYPE_DEF(name, type, parse_func, extra_arg)           \
  static inline bool                                                          \
  name (type *ptr, const char str[], const char **endptr)                     \
  {                                                                           \
    char *end;                                                                \
    *ptr = (type) parse_func (str, &end extra_arg);                           \
    if (endptr != NULL) *endptr = end;                                        \
    return end != str;                                                        \
    }

M_PARSE_DEFAULT_TYPE_DEF(m_core_parse_schar, signed char, strtol, M_DEFERRED_COMMA 10)
M_PARSE_DEFAULT_TYPE_DEF(m_core_parse_uchar, unsigned char, strtoul, M_DEFERRED_COMMA 10)
M_PARSE_DEFAULT_TYPE_DEF(m_core_parse_sshort, signed short, strtol, M_DEFERRED_COMMA 10)
M_PARSE_DEFAULT_TYPE_DEF(m_core_parse_ushort, unsigned short, strtoul, M_DEFERRED_COMMA 10)
M_PARSE_DEFAULT_TYPE_DEF(m_core_parse_sint, signed int, strtol, M_DEFERRED_COMMA 10)
M_PARSE_DEFAULT_TYPE_DEF(m_core_parse_uint, unsigned int, strtoul, M_DEFERRED_COMMA 10)
M_PARSE_DEFAULT_TYPE_DEF(m_core_parse_slong, signed long, strtol, M_DEFERRED_COMMA 10)
M_PARSE_DEFAULT_TYPE_DEF(m_core_parse_ulong, unsigned long, strtoul, M_DEFERRED_COMMA 10)
M_PARSE_DEFAULT_TYPE_DEF(m_core_parse_sllong, signed long long, strtoll, M_DEFERRED_COMMA 10)
M_PARSE_DEFAULT_TYPE_DEF(m_core_parse_ullong, unsigned long long, strtoull, M_DEFERRED_COMMA 10)
M_PARSE_DEFAULT_TYPE_DEF(m_core_parse_float, float, strtof, )
M_PARSE_DEFAULT_TYPE_DEF(m_core_parse_double, double, strtod, )
M_PARSE_DEFAULT_TYPE_DEF(m_core_parse_ldouble, long double, strtold, )

/* Internal macro to separate two arguments by a semicolon */
#define M_SEPARATE_PER_SEMICOLON(a,b) a ; b

/* Generic PRINT macro: print all its inputs regardless of the type
   provided it is a generic "non-struct" type. */
#define M_PRINT(...)   do { M_REDUCE(M_PRINT_ARG, M_SEPARATE_PER_SEMICOLON, __VA_ARGS__); } while (0)

/* Generic FPRINT macro: print all its inputs regardless of the type
   provided it is a generic "non-struct" type into the file 'f'. */
#define M_FPRINT(f,...)  do { M_REDUCE2(M_FPRINT_ARG, M_SEPARATE_PER_SEMICOLON, f, __VA_ARGS__); } while (0)

/* Within a C11 _Generic statement, all expressions shall be valid C
   expression even if the case if always false, and is not executed.
   This can seriously limit the _Generic statement.
   This macro overcomes this limitation by returning :
   * either the input 'x' if it is of type 'type',
   * or the value 0 view as a type 'type'.
   NOTE: Not compatible with C++.
*/
#define M_AS_TYPE(type, x) _Generic(((void)0,(x)), type: (x), default: (type) {0})

/* Perform a C conditional operator with the following restriction:
 * - cond shall be a compile time constant.
 * However, true_expr and false_expr can be objects of different types.
 * The type of the returned expression will be the same as the
 * returned object without any promotion.
 * NOTE: The classic conditional operator can return different types
 * if and only both objects are pointers. If the selected pointer is
 * a null pointer constant, the returned type depends if the **other**
 * expression is itself a null pointer constant or not.
 */
#define M_CONDITIONAL(cond, true_expr, false_expr)                            \
  _Generic(1 ? (float *) 0 : (void *)(intptr_t) (cond),  float *: false_expr, void *: true_expr)

/* Return the minimum between x and y (computed in compile time) */
#define M_MIN(x, y) ((x) < (y) ? (x) : (y))

/* Return the maximum between x and y (computed in compile time) */
#define M_MAX(x, y) ((x) > (y) ? (x) : (y))

/* Is the number a power of 2? (computed in compile time) */
#define M_POWEROF2_P(n) (!((n)&((n)-1)))

/* Swap two objects x & y of the same type */
#define M_SWAP(type, x, y) do {                                               \
    type _tmp = (x);                                                          \
    (x) = (y);                                                                \
    (y) = _tmp;                                                               \
  } while (0)


/* Check if 'n' is assignable to an object of type 'type'.
   It is as if we create a temporary of type 'type' and assign 'n' to it.
   Return the object 'n' if it is possible.
   Two definitions: one with compound-literals for C, the other with static_cast for C++.
   NOTE: C definition is safer than the C++ one.
*/
#ifndef __cplusplus
/* Cast the object n into type only if it is C assignable */
# define M_ASSIGN_CAST(type, n)                 ((type) { 0 } = (n))
#else
/* Cast the object n into type using a static_cast */
# define M_ASSIGN_CAST(type, n)                 static_cast<type>(n)
#endif


/* Cast 'n' of type 'type*' into 'type const*'.
   This is like (type const*)p but safer as the type of 'n' is checked,
   and more robust for double arrays type.
   NOTE: Compliant with the C standard as in §6.2.5 Types:
   "Similarly, pointers to qualified or unqualified versions
   of compatible types shall have the same representation and
   alignment requirements."
*/
#ifndef __cplusplus
# define M_CONST_CAST(type, n)                                                \
  (((union { type *ptr; type const *cptr; }){n}).cptr)
#else
# define M_CONST_CAST(type, n)                  const_cast<type*>(n)
#endif


/*
 * From a pointer to the 'field' field of type 'field_type' of a 'type' structure,
 * return the pointer to the structure.
 * Used to handle intrusive structure.
 * NOTE: Use an ASSIGN_CAST to make the cast a little bit safer.
 */
#define M_TYPE_FROM_FIELD(type, ptr, field_type, field)                       \
  ((type *)(void*)( (char *)M_ASSIGN_CAST(field_type*, (ptr)) - offsetof(type, field) ))

#define M_CTYPE_FROM_FIELD(type, ptr, field_type, field)                      \
  ((type const *)(const void*)( (const char *)M_ASSIGN_CAST(field_type const *, (ptr)) - offsetof(type, field) ))


/* Use to generate a dummy alignment field for cache alignment within a structure 
   Take the name of the dummy field, and a list of the type of the fields that previously fill in the structure.
 */
#define M_APPLY_SIZEOF(id)           sizeof (id)
#define M_ADD_SIZE(a,b)              a + b
#define M_CACHELINE_ALIGN(name, ...)                                          \
  char name[M_ALIGN_FOR_CACHELINE_EXCLUSION > M_REDUCE(M_APPLY_SIZEOF, M_ADD_SIZE, __VA_ARGS__) \
            ? M_ALIGN_FOR_CACHELINE_EXCLUSION - M_REDUCE(M_APPLY_SIZEOF, M_ADD_SIZE, __VA_ARGS__) : 1]


/* C++ doesn't support flexible array within a structure.
   Let's define at least one element for an array. */
#ifdef __cplusplus
# define M_MIN_FLEX_ARRAY_SIZE 1
#else
# define M_MIN_FLEX_ARRAY_SIZE 
#endif

#if M_USE_STDARG

/* Define the allocation of the temporary string used by M_CSTR
   Default is 256 bytes (so 255 characters excluding the final null char).
   It can be overriden by users if needed.
*/
#ifndef M_USE_CSTR_ALLOC
#define M_USE_CSTR_ALLOC 256
#endif

/* Define M_CSTR for C++ & C */
#if defined(__cplusplus)
namespace m_lib {
  template <unsigned int N>
    struct m_char_array {
      char str[N];
      inline m_char_array(const char *format, ...)
      {
        va_list args;
        va_start (args, format);
        int ret = vsnprintf(str, N, format, args);
        (void) ret;
        va_end (args);
      }
      inline const char *m_get(void)
      {
        return str;
      }
    };
}

/* Return a constant string constructed based on the printf-liked formated string
   and its arguments.
   The string is constructed at run time and uses a temporary space on the stack.
   If the constructed string is longer than M_USE_CSTR_ALLOC (default 256),
   the string is truncated.
   Example:
    strlen( M_CSTR("Len=%d", 17) ) == 6
   NOTE: C++ definition using template as C++ doesn't support compound litteral.
*/
#define M_CSTR(...)                                                           \
  (m_lib::m_char_array<M_USE_CSTR_ALLOC>(__VA_ARGS__)).m_get()

#elif defined(__GNUC__) && !defined(M_ADDRESS_SANITIZER)

/* Return a constant string constructed based on the printf-liked formated string
   and its arguments.
   The string is constructed at run time and uses a temporary space on the stack.
   If the constructed string is longer than M_USE_CSTR_ALLOC (default 256),
   the string is truncated.
   Example:
    strlen( M_CSTR("Len=%d", 17) ) == 6
   NOTE: C definition using GNU C extension statement expression to produce
   smaller & faster code and enables the compiler to produce better warnings.
   In C, the return value is the array, not the pointer to the array
   (if I understand https://gcc.gnu.org/onlinedocs/gcc-10.2.0/gcc/Statement-Exprs.html
   correctly, it is specified that only G++ converts the final result value
   if it is an array to a pointer), so the return value is given back
   to the parent scope.
   However, address sanitizer doesn't see this particular corner case,
   and raise an error.
   So let's disable this implementation if address sanitizer is enabled.
*/
#define M_CSTR(...)                                                           \
  M_ATTR_EXTENSION ({char m_core_tmp[M_USE_CSTR_ALLOC];                       \
      int m_core_r = snprintf(m_core_tmp,  M_USE_CSTR_ALLOC, __VA_ARGS__);    \
      (void) m_core_r; m_core_tmp; })

#else

// Encapsulate snprintf to return the input buffer as argument (needed for M_CSTR)
static inline const char *
m_core_snprintf(char *str, size_t size, const char *format, ...)
{
  va_list args;
  va_start (args, format);
  int ret = vsnprintf(str, size, format, args);
  (void) ret;
  va_end (args);
  return str;
}

/* Return a constant string constructed based on the printf-liked formated string
   and its arguments.
   The string is constructed at run time and uses a temporary space on the stack.
   If the constructed string is longer than M_USE_CSTR_ALLOC (default 256),
   the string is truncated.
   Example:
    strlen( M_CSTR("Len=%d", 17) ) == 6
   NOTE: C definition using compound litteral (init to 0).
   This has a bad performance impact since it clears the memory before using it.
*/
#define M_CSTR(...)                                                           \
  m_core_snprintf( (char [M_USE_CSTR_ALLOC]){0}, M_USE_CSTR_ALLOC, __VA_ARGS__)

#endif

#endif // Have stdarg


/************************************************************/
/********************* HASH selection ***********************/
/************************************************************/

/* User code shall overwrite this macro by a random seed (of type size_t)
   so that the hash are not easily predictible by an attacker.
   See https://events.ccc.de/congress/2011/Fahrplan/attachments/2007_28C3_Effective_DoS_on_web_application_platforms.pdf
*/
#ifndef M_USE_HASH_SEED
# define M_USE_HASH_SEED 0UL
#endif

#if   defined(M_USE_DJB_HASH)
#define M_HASH_INIT 5381UL
#define M_HASH_CALC(h1,h2)  (((h1) * 33UL) + (h2))
#elif defined(M_USE_DJB_XOR_HASH)
#define M_HASH_INIT 5381UL
#define M_HASH_CALC(h1,h2)  (((h1) * 33UL) ^ (h2))
#elif defined(M_USE_JSHASH)
#define M_HASH_INIT 1315423911UL
#define M_HASH_CALC(h1,h2)  ((h1) ^ (((h1) << 5) + (h2) + ((h1) >> 2)))
#elif defined(M_USE_BKDRHASH)
#define M_HASH_INIT 0UL
#define M_HASH_CALC(h1,h2)  (((h1) * 131) + (h2))
#elif defined(M_USE_SDBMHASH)
#define M_HASH_INIT 0UL
#define M_HASH_CALC(h1,h2)  ((h2) + ((h1) << 6) + ((h1) << 16) - (h1))
#elif defined(M_USE_DEKHASH)
#define M_HASH_INIT 0UL /* should be len but not possible with interface */
#define M_HASH_CALC(h1,h2)  ( (((h1)<<5) | (h1 >> (CHAR_BIT*sizeof(size_t)-5))) ^(h2))
#elif defined(M_USE_BPHASH)
#define M_HASH_INIT 0UL
#define M_HASH_CALC(h1,h2)  (((h1) << 7) ^ (h2))
#else
//FNVHASH
#define M_HASH_INIT 0UL
#define M_HASH_CALC(h1,h2)  (((h1) * 0x811C9DC5UL) ^ (h2))
#endif

#define M_HASH_DECL(hash)   size_t hash = M_HASH_INIT ^ M_USE_HASH_SEED
#define M_HASH_UP(hash,h)   do { hash = (size_t) M_HASH_CALC(hash, (h)); } while (0)
#define M_HASH_FINAL(hash)  ( (hash) >> (sizeof(size_t)*CHAR_BIT/2) | (hash) )

/* Safe, efficient, and portable Rotate:
   It should be recognized by any compiler and generate a single roll instruction.
   If support for n==0 is needed, we can write:
   return (x<<n) | (x>>(-n&31));
   but compilers are less likely to generate a roll instruction.
 */
static inline uint32_t m_core_rotl32a (uint32_t x, uint32_t n)
{
  M_ASSERT (n > 0 && n<32);
  return (x<<n) | (x>>(32-n));
}
static inline uint64_t m_core_rotl64a (uint64_t x, uint32_t n)
{
  M_ASSERT (n > 0 && n<64);
  return (x<<n) | (x>>(64-n));
}

/* Round to the next highest power of 2.
   See https://web.archive.org/web/20160703165415/https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
*/
static inline uint64_t m_core_roundpow2(uint64_t v)
{
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v |= v >> 32;
  v++;
  return v;
}

/* Return the count leading zero of the argument */
#if defined(__GNUC__) && (__GNUC__*100 + __GNUC_MINOR__) >= 304
static inline unsigned int m_core_clz32(uint32_t limb)
{
  return (unsigned int) (M_UNLIKELY (limb == 0) ? sizeof(uint32_t)*CHAR_BIT : (size_t) __builtin_clzl(limb) - (sizeof(unsigned long) - sizeof(uint32_t)) * CHAR_BIT);
}
static inline unsigned int m_core_clz64(uint64_t limb)
{
  return (unsigned int) (M_UNLIKELY (limb == 0ULL) ? sizeof (uint64_t)*CHAR_BIT : (size_t) __builtin_clzll(limb) - (sizeof (unsigned long long) - sizeof (uint64_t)) * CHAR_BIT);
}

#elif defined(_MSC_VER) && (defined(_M_AMD64) || defined(_M_ARM64))
// NOTE: _BitScanReverse64 is 64-bits only (not compatible 32 bits).
#include <intrin.h>
static inline unsigned int m_core_clz32(uint32_t limb)
{
  unsigned long bit = 0;
  if (_BitScanReverse( &bit, limb ) != 0) {
    return 31 - bit;
  } else {
    return 32;
  }
}
static inline unsigned int m_core_clz64(uint64_t limb)
{
  unsigned long bit = 0;
  if (_BitScanReverse64( &bit, limb ) != 0) {
    return 63 - bit;
  } else {
    return 64;
  }
}

#else
// Emulation layer
#define M_CORE_CLZ_TAB "\010\07\06\06\05\05\05\05\04\04\04\04\04\04\04\04\03\03\03\03\03\03\03\03\03\03\03\03\03\03\03\03\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\02\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\01\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"

static inline unsigned int m_core_clz32(uint32_t limb)
{
  unsigned int shift = 0;
  /* This code should be branchless on most targets */
  /* Compute shift so that, it is
     + -3 if the highest 24 bits are clear,
     + -2 for the highest 16 bits,
     + -1 for the highest 8 bits,
     + 0 otherwise */
  shift = -(limb < 0x1000000UL);
  shift -= (limb < 0x10000UL);
  shift -= (limb < 0x100UL);
  shift  = shift*8 + 24;
  shift = 24 - shift + (unsigned int) M_CORE_CLZ_TAB[limb >> shift ];
  return shift;
}

static inline unsigned int m_core_clz64(uint64_t limb)
{
  unsigned int shift = 0;
  /* This code should be branchless on most targets */
  /* Compute shift so that, it is
     + -7 if the highest 56 bits are clear,
     + -6 for the highest 48 bits,
     + -5 for the highest 40 bits,
     + -4 for the highest 32 bits,
     + -3 for the highest 24 bits,
     + -2 for the highest 16 bits,
     + -1 for the highest 8 bits,
     + 0 otherwise */
  shift = -(limb < 0x100000000000000UL);
  shift -= (limb < 0x1000000000000UL);
  shift -= (limb < 0x10000000000UL);
  shift -= (limb < 0x100000000UL);
  shift -= (limb < 0x1000000UL);
  shift -= (limb < 0x10000UL);
  shift -= (limb < 0x100UL);
  shift  = shift*8 + 56;
  shift = 56 - shift + (unsigned int) M_CORE_CLZ_TAB[limb >> shift ];
  return shift;
}
#endif

/* Implement a kind of FNV1A Hash.
   Inspired by http://www.sanmayce.com/Fastest_Hash/ Jesteress and port to 64 bits.
   See https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-1a_hash
   NOTE: A lot of cast. Not really type nor alignement safe.
   NOTE: Can be reduced to very few instructions if constant size argument.
   FIXME: It is trivial for an attacker to generate collision and HASH_SEED doesn't prevent it.
 */
#if SIZE_MAX <= 4294967295U
/* 32 bits variant with an average measured avalanche effect of 16.056 bits */
static inline uint32_t
m_core_hash (const void *str, size_t length)
{
  const uint32_t prime = 709607U;
  uint32_t hash32 = 2166136261U ^ M_USE_HASH_SEED;
  const uint8_t *p = (const uint8_t *)str;

  M_ASSERT (str != NULL || length == 0);
  M_ASSERT ( (( (uintptr_t)p & (sizeof(uint32_t)-1) ) == 0) || (length <= sizeof(uint32_t)));

  while (length >= 2*sizeof(uint32_t)) {
    const uint32_t *ptr = (const uint32_t *) (uintptr_t) p;
    hash32 = (hash32 ^ (m_core_rotl32a(ptr[0], 5) ^ ptr[1])) * prime;
    length -= 2*sizeof(uint32_t);
    p += 2*sizeof(uint32_t);
  }
  // Cases: 0,1,2,3,4,5,6,7
  if (length & sizeof(uint32_t)) {
    const uint32_t *ptr = (const uint32_t *) (uintptr_t) p;
    hash32 = (hash32 ^ ptr[0]) * prime;
    p += sizeof(uint32_t);
    }
  if (length & sizeof(uint16_t)) {
    const uint16_t *ptr = (const uint16_t *) (uintptr_t) p;
    hash32 = (hash32 ^ ptr[0]) * prime;
    p += sizeof(uint16_t);
  }
  if (length & 1)
    hash32 = (hash32 ^ *p) * prime;
  return hash32 ^ (hash32 >> 16);
}
#else
/* 64 bits variant with an average measured avalanche effect of 31.862 bits */
static inline uint64_t
m_core_hash (const void *str, size_t length)
{
  const uint64_t prime = 1099511628211ULL;
  uint64_t hash64 = 14695981039346656037ULL ^ M_USE_HASH_SEED;
  const uint8_t *p = M_ASSIGN_CAST(const uint8_t *, str);

  M_ASSERT (str != NULL || length == 0);
  M_ASSERT ( (( (uintptr_t)p & (sizeof(uint64_t)-1) ) == 0) || (length <= sizeof(uint32_t)));

  while (length >= 2*sizeof(uint64_t)) {
    const uint64_t *ptr = (const uint64_t *) (uintptr_t) p;
    hash64 = (hash64 ^ (m_core_rotl64a(ptr[0], 5) ^ ptr[1])) * prime;
    length -= 2*sizeof(uint64_t);
    p += 2*sizeof(uint64_t);
  }
  //Cases: 0 to 15.
  if (length & sizeof(uint64_t)) {
    const uint64_t *ptr = (const uint64_t *) (uintptr_t) p;
    hash64 = (hash64 ^ ptr[0]) * prime;
    p += sizeof(uint64_t);
    }
  // Cases: 0,1,2,3,4,5,6,7
  if (length & sizeof(uint32_t)) {
    const uint32_t *ptr = (const uint32_t *) (uintptr_t) p;
    hash64 = (hash64 ^ ptr[0]) * prime;
    p += sizeof(uint32_t);
    }
  if (length & sizeof(uint16_t)) {
    const uint16_t *ptr = (const uint16_t *) (uintptr_t) p;
    hash64 = (hash64 ^ ptr[0]) * prime;
    p += sizeof(uint16_t);
  }
  if (length & 1)
    hash64 = (hash64 ^ *p) * prime;
  return hash64 ^ (hash64 >> 32);
}
#endif

/* HASH function for a C-string (to be used within oplist) */
static inline size_t m_core_cstr_hash(const char str[])
{
  M_HASH_DECL(hash);
  while (*str) {
    unsigned long u = (unsigned char) *str++;
    M_HASH_UP(hash, u);
  }
  return M_HASH_FINAL(hash);
}


/* Define default HASH function.
   Macro encapsulation for C11: use specialized version of the hash function
   if the type is recognized.
   NOTE: Default case is not safe if the type is defined with the '[1]' trick. */
#define M_HASH_POD_DEFAULT(a)   m_core_hash((const void*) &(a), sizeof (a))
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define M_HASH_INT32(a) ( (a) ^ ((a) << 11) ^ M_USE_HASH_SEED )
#define M_HASH_INT64(a) ( ( (a) >> 33 ) ^ (a) ^ ((a) << 11) ^ M_USE_HASH_SEED )
#define M_HASH_DEFAULT(a)                                                     \
  (size_t) _Generic((a)+0,                                                    \
           int32_t:  M_HASH_INT32((uint32_t) M_AS_TYPE(int32_t, a)),          \
           uint32_t: M_HASH_INT32(M_AS_TYPE(uint32_t, a)),                    \
           int64_t:  M_HASH_INT64((uint64_t) M_AS_TYPE(int64_t, a)),          \
           uint64_t: M_HASH_INT64(M_AS_TYPE(uint64_t, a)),                    \
           default:  M_HASH_POD_DEFAULT(a) )
#else
#define M_HASH_DEFAULT(a)       M_HASH_POD_DEFAULT(a)
#endif



/************************************************************/
/******************** METHODS handling **********************/
/************************************************************/


/* Helper internal macros to make M_GET_METHOD works.
   List of supported methods for an oplist */
#define M_INIT_INIT(a)           ,a,
#define M_INIT_SET_INIT_SET(a)   ,a,
#define M_INIT_MOVE_INIT_MOVE(a) ,a,
#define M_INIT_WITH_INIT_WITH(a) ,a,
#define M_SWAP_SWAP(a)           ,a,
#define M_SET_SET(a)             ,a,
#define M_MOVE_MOVE(a)           ,a,
#define M_CLEAR_CLEAR(a)         ,a,
#define M_NEW_NEW(a)             ,a,
#define M_DEL_DEL(a)             ,a,
#define M_REALLOC_REALLOC(a)     ,a,
#define M_FREE_FREE(a)           ,a,
#define M_MEMPOOL_MEMPOOL(a)     ,a,
#define M_MEMPOOL_LINKAGE_MEMPOOL_LINKAGE(a)     ,a,
#define M_HASH_HASH(a)           ,a,
#define M_EQUAL_EQUAL(a)         ,a,
#define M_CMP_CMP(a)             ,a,
#define M_TYPE_TYPE(a)           ,a,
#define M_SUBTYPE_SUBTYPE(a)     ,a,
#define M_NAME_NAME(a)           ,a,
#define M_OPLIST_OPLIST(a)       ,a,
#define M_SORT_SORT(a)           ,a,
#define M_UPDATE_UPDATE(a)       ,a,
#define M_SPLICE_BACK_SPLICE_BACK(a) ,a,
#define M_SPLICE_AT_SPLICE_AT(a) ,a,
#define M_IT_TYPE_IT_TYPE(a)     ,a,
#define M_IT_FIRST_IT_FIRST(a)   ,a,
#define M_IT_LAST_IT_LAST(a)     ,a,
#define M_IT_END_IT_END(a)       ,a,
#define M_IT_SET_IT_SET(a)       ,a,
#define M_IT_END_P_IT_END_P(a)   ,a,
#define M_IT_LAST_P_IT_LAST_P(a) ,a,
#define M_IT_EQUAL_P_IT_EQUAL_P(a) ,a,
#define M_IT_NEXT_IT_NEXT(a)     ,a,
#define M_IT_PREVIOUS_IT_PREVIOUS(a)     ,a,
#define M_IT_REF_IT_REF(a)       ,a,
#define M_IT_CREF_IT_CREF(a)     ,a,
#define M_IT_REMOVE_IT_REMOVE(a) ,a,
#define M_IT_INSERT_IT_INSERT(a) ,a,
#define M_EMPTY_P_EMPTY_P(a) ,a,
#define M_ADD_ADD(a)             ,a,
#define M_SUB_SUB(a)             ,a,
#define M_MUL_MUL(a)             ,a,
#define M_DIV_DIV(a)             ,a,
#define M_CLEAN_CLEAN(a)         ,a,
#define M_KEY_TYPE_KEY_TYPE(a)   ,a,
#define M_VALUE_TYPE_VALUE_TYPE(a) ,a,
#define M_KEY_OPLIST_KEY_OPLIST(a) ,a,
#define M_VALUE_OPLIST_VALUE_OPLIST(a) ,a,
#define M_GET_KEY_GET_KEY(a)     ,a,
#define M_SET_KEY_SET_KEY(a)     ,a,
#define M_GET_SET_KEY_GET_SET_KEY(a) ,a,
#define M_ERASE_KEY_ERASE_KEY(a) ,a,
#define M_GET_SIZE_GET_SIZE(a)   ,a,
#define M_PUSH_PUSH(a)           ,a,
#define M_POP_POP(a)             ,a,
#define M_PUSH_MOVE_PUSH_MOVE(a) ,a,
#define M_POP_MOVE_POP_MOVE(a)   ,a,
#define M_REVERSE_REVERSE(a)     ,a,
#define M_GET_STR_GET_STR(a)     ,a,
#define M_PARSE_STR_PARSE_STR(a) ,a,
#define M_OUT_STR_OUT_STR(a)     ,a,
#define M_IN_STR_IN_STR(a)       ,a,
#define M_OUT_SERIAL_OUT_SERIAL(a) ,a,
#define M_IN_SERIAL_IN_SERIAL(a) ,a,
#define M_SEPARATOR_SEPARATOR(a) ,a,
#define M_EXT_ALGO_EXT_ALGO(a)   ,a,
#define M_INC_ALLOC_INC_ALLOC(a) ,a,
#define M_OOR_SET_OOR_SET(a)     ,a,
#define M_OOR_EQUAL_OOR_EQUAL(a) ,a,
#define M_LIMITS_LIMITS(a)       ,a,
#define M_PROPERTIES_PROPERTIES(a) ,a,

/* From an oplist - an unorded list of methods : like "INIT(mpz_init),CLEAR(mpz_clear),SET(mpz_set)" -
   Return the given method in the oplist or the default method.
   Example: 
   M_GET_METHOD(INIT, my_default, INIT(mpz_init),CLEAR(mpz_clear),SET(mpz_set)) --> mpz_init
   M_GET_METHOD(INIT, my_default, CLEAR(mpz_clear),SET(mpz_set)) --> my_default */
#define M_GET_METHOD(method, method_default, ...)                             \
  M_RET_ARG2 (M_MAP2B(M_C, M_C3(M_, method, _), __VA_ARGS__), method_default,)

/* Get the given method */
#define M_GET_INIT(...)      M_GET_METHOD(INIT,        M_INIT_DEFAULT,     __VA_ARGS__)
#define M_GET_INIT_SET(...)  M_GET_METHOD(INIT_SET,    M_SET_DEFAULT,      __VA_ARGS__)
#define M_GET_INIT_MOVE(...) M_GET_METHOD(INIT_MOVE,   M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_INIT_WITH(...) M_GET_METHOD(INIT_WITH,   M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_SET(...)       M_GET_METHOD(SET,         M_SET_DEFAULT,      __VA_ARGS__)
#define M_GET_MOVE(...)      M_GET_METHOD(MOVE,        M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_SWAP(...)      M_GET_METHOD(SWAP,        M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_CLEAR(...)     M_GET_METHOD(CLEAR,       M_NOTHING_DEFAULT,  __VA_ARGS__)
#define M_GET_NEW(...)       M_GET_METHOD(NEW,         M_NEW_DEFAULT,      __VA_ARGS__)
#define M_GET_DEL(...)       M_GET_METHOD(DEL,         M_DEL_DEFAULT,      __VA_ARGS__)
#define M_GET_REALLOC(...)   M_GET_METHOD(REALLOC,     M_REALLOC_DEFAULT,  __VA_ARGS__)
#define M_GET_FREE(...)      M_GET_METHOD(FREE,        M_FREE_DEFAULT,     __VA_ARGS__)
#define M_GET_MEMPOOL(...)   M_GET_METHOD(MEMPOOL,     M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_MEMPOOL_LINKAGE(...)   M_GET_METHOD(MEMPOOL_LINKAGE, ,       __VA_ARGS__)
#define M_GET_HASH(...)      M_GET_METHOD(HASH,        M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_EQUAL(...)     M_GET_METHOD(EQUAL,       M_EQUAL_DEFAULT,    __VA_ARGS__)
#define M_GET_CMP(...)       M_GET_METHOD(CMP,         M_CMP_DEFAULT,      __VA_ARGS__)
#define M_GET_UPDATE(...)    M_GET_METHOD(UPDATE,      M_SET_DEFAULT,      __VA_ARGS__)
#define M_GET_TYPE(...)      M_GET_METHOD(TYPE,        M_NO_DEF_TYPE,      __VA_ARGS__)
#define M_GET_SUBTYPE(...)   M_GET_METHOD(SUBTYPE,     M_NO_DEF_TYPE,      __VA_ARGS__)
#define M_GET_NAME(...)      M_GET_METHOD(NAME,        M_NO_DEF_TYPE,      __VA_ARGS__)
#define M_GET_OPLIST(...)    M_GET_METHOD(OPLIST,      (),                 __VA_ARGS__)
#define M_GET_SORT(...)      M_GET_METHOD(SORT,        M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_SPLICE_BACK(...) M_GET_METHOD(SPLICE_BACK, M_NO_DEFAULT,     __VA_ARGS__)
#define M_GET_SPLICE_AT(...) M_GET_METHOD(SPLICE_AT,   M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_IT_TYPE(...)   M_GET_METHOD(IT_TYPE,     M_NO_DEF_TYPE,      __VA_ARGS__)
#define M_GET_IT_FIRST(...)  M_GET_METHOD(IT_FIRST,    M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_IT_LAST(...)   M_GET_METHOD(IT_LAST,     M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_IT_END(...)    M_GET_METHOD(IT_END,      M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_IT_SET(...)    M_GET_METHOD(IT_SET,      M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_IT_END_P(...)  M_GET_METHOD(IT_END_P,    M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_IT_LAST_P(...) M_GET_METHOD(IT_LAST_P,   M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_IT_EQUAL_P(...) M_GET_METHOD(IT_EQUAL_P, M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_IT_NEXT(...)   M_GET_METHOD(IT_NEXT,     M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_IT_PREVIOUS(...) M_GET_METHOD(IT_PREVIOUS, M_NO_DEFAULT,     __VA_ARGS__)
#define M_GET_IT_REF(...)    M_GET_METHOD(IT_REF,      M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_IT_CREF(...)   M_GET_METHOD(IT_CREF,     M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_IT_REMOVE(...) M_GET_METHOD(IT_REMOVE,   M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_IT_INSERT(...) M_GET_METHOD(IT_INSERT,   M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_EMPTY_P(...) M_GET_METHOD(EMPTY_P, M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_ADD(...)       M_GET_METHOD(ADD,         M_ADD_DEFAULT,      __VA_ARGS__)
#define M_GET_SUB(...)       M_GET_METHOD(SUB,         M_SUB_DEFAULT,      __VA_ARGS__)
#define M_GET_MUL(...)       M_GET_METHOD(MUL,         M_MUL_DEFAULT,      __VA_ARGS__)
#define M_GET_DIV(...)       M_GET_METHOD(DIV,         M_DIV_DEFAULT,      __VA_ARGS__)
#define M_GET_CLEAN(...)     M_GET_METHOD(CLEAN,       M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_KEY_TYPE(...)  M_GET_METHOD(KEY_TYPE,    M_NO_DEF_TYPE,      __VA_ARGS__)
#define M_GET_VALUE_TYPE(...) M_GET_METHOD(VALUE_TYPE, M_NO_DEF_TYPE,      __VA_ARGS__)
#define M_GET_KEY_OPLIST(...) M_GET_METHOD(KEY_OPLIST, M_NO_DEF_TYPE,      __VA_ARGS__)
#define M_GET_VALUE_OPLIST(...) M_GET_METHOD(VALUE_OPLIST, M_NO_DEF_TYPE,  __VA_ARGS__)
#define M_GET_GET_KEY(...)   M_GET_METHOD(GET_KEY,     M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_SET_KEY(...)   M_GET_METHOD(SET_KEY,     M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_GET_SET_KEY(...) M_GET_METHOD(GET_SET_KEY, M_NO_DEFAULT,     __VA_ARGS__)
#define M_GET_ERASE_KEY(...) M_GET_METHOD(ERASE_KEY,   M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_GET_SIZE(...)  M_GET_METHOD(GET_SIZE,    M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_PUSH(...)      M_GET_METHOD(PUSH,        M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_POP(...)       M_GET_METHOD(POP,         M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_PUSH_MOVE(...) M_GET_METHOD(PUSH_MOVE,   M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_POP_MOVE(...)  M_GET_METHOD(POP_MOVE,    M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_REVERSE(...)   M_GET_METHOD(REVERSE,     M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_GET_STR(...)   M_GET_METHOD(GET_STR,     M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_PARSE_STR(...) M_GET_METHOD(PARSE_STR,   M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_OUT_STR(...)   M_GET_METHOD(OUT_STR,     M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_IN_STR(...)    M_GET_METHOD(IN_STR,      M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_OUT_SERIAL(...) M_GET_METHOD(OUT_SERIAL, M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_IN_SERIAL(...) M_GET_METHOD(IN_SERIAL,   M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_SEPARATOR(...) M_GET_METHOD(SEPARATOR,   ',',                __VA_ARGS__)
#define M_GET_EXT_ALGO(...)  M_GET_METHOD(EXT_ALGO,    M_NO_EXT_ALGO,      __VA_ARGS__)
#define M_GET_INC_ALLOC(...) M_GET_METHOD(INC_ALLOC,   M_INC_ALLOC_DEFAULT, __VA_ARGS__)
#define M_GET_OOR_SET(...)   M_GET_METHOD(OOR_SET,     M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_OOR_EQUAL(...) M_GET_METHOD(OOR_EQUAL,   M_NO_DEFAULT,       __VA_ARGS__)
#define M_GET_LIMITS(...)    M_GET_METHOD(LIMITS,      M_LIMITS_DEFAULT,   __VA_ARGS__)
#define M_GET_PROPERTIES(...) M_GET_METHOD(PROPERTIES, (),   __VA_ARGS__)

// Calling method with support of defined transformation API
// operators that are not methods are commented
#define M_CALL_INIT(oplist, ...) M_APPLY_API(M_GET_INIT oplist, oplist, __VA_ARGS__)
#define M_CALL_INIT_SET(oplist, ...) M_APPLY_API(M_GET_INIT_SET oplist, oplist, __VA_ARGS__)
#define M_CALL_INIT_MOVE(oplist, ...) M_APPLY_API(M_GET_INIT_MOVE oplist, oplist, __VA_ARGS__)
#define M_CALL_INIT_WITH(oplist, ...) M_APPLY_API(M_GET_INIT_WITH oplist, oplist, __VA_ARGS__)
#define M_CALL_SET(oplist, ...) M_APPLY_API(M_GET_SET oplist, oplist, __VA_ARGS__)
#define M_CALL_MOVE(oplist, ...) M_APPLY_API(M_GET_MOVE oplist, oplist, __VA_ARGS__)
#define M_CALL_SWAP(oplist, ...) M_APPLY_API(M_GET_SWAP oplist, oplist, __VA_ARGS__)
#define M_CALL_CLEAR(oplist, ...) M_APPLY_API(M_GET_CLEAR oplist, oplist, __VA_ARGS__)
#define M_CALL_NEW(oplist, ...) M_APPLY_API(M_GET_NEW oplist, oplist, __VA_ARGS__)
#define M_CALL_DEL(oplist, ...) M_APPLY_API(M_GET_DEL oplist, oplist, __VA_ARGS__)
#define M_CALL_REALLOC(oplist, ...) M_APPLY_API(M_GET_REALLOC oplist, oplist, __VA_ARGS__)
#define M_CALL_FREE(oplist, ...) M_APPLY_API(M_GET_FREE oplist, oplist, __VA_ARGS__)
#define M_CALL_MEMPOOL(oplist, ...) M_APPLY_API(M_GET_MEMPOOL oplist, oplist, __VA_ARGS__)
#define M_CALL_MEMPOOL_LINKAGE(oplist, ...) M_APPLY_API(M_GET_MEMPOOL_LINKAGE oplist, oplist, __VA_ARGS__)
#define M_CALL_HASH(oplist, ...) M_APPLY_API(M_GET_HASH oplist, oplist, __VA_ARGS__)
#define M_CALL_EQUAL(oplist, ...) M_APPLY_API(M_GET_EQUAL oplist, oplist, __VA_ARGS__)
#define M_CALL_CMP(oplist, ...) M_APPLY_API(M_GET_CMP oplist, oplist, __VA_ARGS__)
#define M_CALL_UPDATE(oplist, ...) M_APPLY_API(M_GET_UPDATE oplist, oplist, __VA_ARGS__)
//#define M_CALL_TYPE(oplist, ...) M_APPLY_API(M_GET_TYPE oplist, oplist, __VA_ARGS__)
//#define M_CALL_SUBTYPE(oplist, ...) M_APPLY_API(M_GET_SUBTYPE oplist, oplist, __VA_ARGS__)
//#define M_CALL_NAME(oplist, ...) M_APPLY_API(M_GET_NAME oplist, oplist, __VA_ARGS__)
//#define M_CALL_OPLIST(oplist, ...) M_APPLY_API(M_GET_OPLIST oplist, oplist, __VA_ARGS__)
#define M_CALL_SORT(oplist, ...) M_APPLY_API(M_GET_SORT oplist, oplist, __VA_ARGS__)
#define M_CALL_SPLICE_BACK(oplist, ...) M_APPLY_API(M_GET_SPLICE_BACK oplist, oplist, __VA_ARGS__)
#define M_CALL_SPLICE_AT(oplist, ...) M_APPLY_API(M_GET_SPLICE_AT oplist, oplist, __VA_ARGS__)
//#define M_CALL_IT_TYPE(oplist, ...) M_APPLY_API(M_GET_IT_TYPE oplist, oplist, __VA_ARGS__)
#define M_CALL_IT_FIRST(oplist, ...) M_APPLY_API(M_GET_IT_FIRST oplist, oplist, __VA_ARGS__)
#define M_CALL_IT_LAST(oplist, ...) M_APPLY_API(M_GET_IT_LAST oplist, oplist, __VA_ARGS__)
#define M_CALL_IT_END(oplist, ...) M_APPLY_API(M_GET_IT_END oplist, oplist, __VA_ARGS__)
#define M_CALL_IT_SET(oplist, ...) M_APPLY_API(M_GET_IT_SET oplist, oplist, __VA_ARGS__)
#define M_CALL_IT_END_P(oplist, ...) M_APPLY_API(M_GET_IT_END_P oplist, oplist, __VA_ARGS__)
#define M_CALL_IT_LAST_P(oplist, ...) M_APPLY_API(M_GET_IT_LAST_P oplist, oplist, __VA_ARGS__)
#define M_CALL_IT_EQUAL_P(oplist, ...) M_APPLY_API(M_GET_IT_EQUAL_P oplist, oplist, __VA_ARGS__)
#define M_CALL_IT_NEXT(oplist, ...) M_APPLY_API(M_GET_IT_NEXT oplist, oplist, __VA_ARGS__)
#define M_CALL_IT_PREVIOUS(oplist, ...) M_APPLY_API(M_GET_IT_PREVIOUS oplist, oplist, __VA_ARGS__)
#define M_CALL_IT_REF(oplist, ...) M_APPLY_API(M_GET_IT_REF oplist, oplist, __VA_ARGS__)
#define M_CALL_IT_CREF(oplist, ...) M_APPLY_API(M_GET_IT_CREF oplist, oplist, __VA_ARGS__)
#define M_CALL_IT_REMOVE(oplist, ...) M_APPLY_API(M_GET_IT_REMOVE oplist, oplist, __VA_ARGS__)
#define M_CALL_IT_INSERT(oplist, ...) M_APPLY_API(M_GET_IT_INSERT oplist, oplist, __VA_ARGS__)
#define M_CALL_EMPTY_P(oplist, ...) M_APPLY_API(M_GET_EMPTY_P oplist, oplist, __VA_ARGS__)
#define M_CALL_ADD(oplist, ...) M_APPLY_API(M_GET_ADD oplist, oplist, __VA_ARGS__)
#define M_CALL_SUB(oplist, ...) M_APPLY_API(M_GET_SUB oplist, oplist, __VA_ARGS__)
#define M_CALL_MUL(oplist, ...) M_APPLY_API(M_GET_MUL oplist, oplist, __VA_ARGS__)
#define M_CALL_DIV(oplist, ...) M_APPLY_API(M_GET_DIV oplist, oplist, __VA_ARGS__)
#define M_CALL_CLEAN(oplist, ...) M_APPLY_API(M_GET_CLEAN oplist, oplist, __VA_ARGS__)
#define M_CALL_GET_KEY(oplist, ...) M_APPLY_API(M_GET_GET_KEY oplist, oplist, __VA_ARGS__)
#define M_CALL_SET_KEY(oplist, ...) M_APPLY_API(M_GET_SET_KEY oplist, oplist, __VA_ARGS__)
#define M_CALL_GET_SET_KEY(oplist, ...) M_APPLY_API(M_GET_GET_SET_KEY oplist, oplist, __VA_ARGS__)
#define M_CALL_ERASE_KEY(oplist, ...) M_APPLY_API(M_GET_ERASE_KEY oplist, oplist, __VA_ARGS__)
#define M_CALL_GET_SIZE(oplist, ...) M_APPLY_API(M_GET_GET_SIZE oplist, oplist, __VA_ARGS__)
#define M_CALL_PUSH(oplist, ...) M_APPLY_API(M_GET_PUSH oplist, oplist, __VA_ARGS__)
#define M_CALL_POP(oplist, ...) M_APPLY_API(M_GET_POP oplist, oplist, __VA_ARGS__)
#define M_CALL_PUSH_MOVE(oplist, ...) M_APPLY_API(M_GET_PUSH_MOVE oplist, oplist, __VA_ARGS__)
#define M_CALL_POP_MOVE(oplist, ...) M_APPLY_API(M_GET_POP_MOVE oplist, oplist, __VA_ARGS__)
#define M_CALL_REVERSE(oplist, ...) M_APPLY_API(M_GET_REVERSE oplist, oplist, __VA_ARGS__)
#define M_CALL_GET_STR(oplist, ...) M_APPLY_API(M_GET_GET_STR oplist, oplist, __VA_ARGS__)
#define M_CALL_PARSE_STR(oplist, ...) M_APPLY_API(M_GET_PARSE_STR oplist, oplist, __VA_ARGS__)
#define M_CALL_OUT_STR(oplist, ...) M_APPLY_API(M_GET_OUT_STR oplist, oplist, __VA_ARGS__)
#define M_CALL_IN_STR(oplist, ...) M_APPLY_API(M_GET_IN_STR oplist, oplist, __VA_ARGS__)
#define M_CALL_OUT_SERIAL(oplist, ...) M_APPLY_API(M_GET_OUT_SERIAL oplist, oplist, __VA_ARGS__)
#define M_CALL_IN_SERIAL(oplist, ...) M_APPLY_API(M_GET_IN_SERIAL oplist, oplist, __VA_ARGS__)
//#define M_CALL_SEPARATOR(oplist, ...) M_APPLY_API(M_GET_SEPARATOR oplist, oplist, __VA_ARGS__)
//#define M_CALL_EXT_ALGO(oplist, ...) M_APPLY_API(M_GET_EXT_ALGO oplist, oplist, __VA_ARGS__)
#define M_CALL_INC_ALLOC(oplist, ...) M_APPLY_API(M_GET_INC_ALLOC oplist, oplist, __VA_ARGS__)
#define M_CALL_OOR_SET(oplist, ...) M_APPLY_API(M_GET_OOR_SET oplist, oplist, __VA_ARGS__)
#define M_CALL_OOR_EQUAL(oplist, ...) M_APPLY_API(M_GET_OOR_EQUAL oplist, oplist, __VA_ARGS__)
//#define M_CALL_LIMITS(oplist, ...) M_APPLY_API(M_GET_LIMITS oplist, oplist, __VA_ARGS__)
//#define M_CALL_PROPERTIES(oplist, ...) M_APPLY_API(M_GET_PROPERTIES oplist, oplist, __VA_ARGS__)

/* API transformation support:
   transform the call to the method into the supported API by the method.
   Example of usage in oplist: ( INIT_WITH(API_1(method)) )
   NOTE: It uses M_RET_ARG3 instead of M_RET_ARG2 since M_GET_METHOD
   uses M_RET_ARG2: to be able to use M_GET_METHOD within a M_APPLY_API
   we need another macro to avoid being marked.
*/
#define M_APPLY_API(method, oplist, ...)                                      \
  M_RET_ARG3( , M_C(M_OPLAPI_INDIRECT_, method)(M_C(M_OPLAPI_EXTRACT_,method),oplist,__VA_ARGS__), method(__VA_ARGS__),)

/* API Transformation :
   0: Method has been disable. It shall not be called. Raised an error.
   API_0: default, API_1: oplist given,
   API_2: by addr for first argument, API_3: oplist given,
   API_4 :by affectation for first argument, 5: API_by affectation + oplist
   API_6 :by addr for two arguments, 5: API_by address for both + oplist

   NOTE: It starts by a comma, to ack the success of the transformation.
*/
#define M_OPLAPI_ERROR(method, oplist, ...)  , M_STATIC_FAILURE(M_LIB_DISABLED_METHOD, "The method has been explictly disabled for the requested operator, yet it has been called by the container")
#define M_OPLAPI_0(method, oplist, ...)      , M_DELAY3(method)(__VA_ARGS__)
#define M_OPLAPI_1(method, oplist, ...)      , M_DELAY3(method)(oplist, __VA_ARGS__)
#define M_OPLAPI_2(method, oplist, ...)      , M_DELAY3(method)(& __VA_ARGS__)
#define M_OPLAPI_3(method, oplist, ...)      , M_DELAY3(method)(oplist, &__VA_ARGS__)
#define M_OPLAPI_4(method, oplist, x, ...)   , x = M_DELAY3(method)(__VA_ARGS__)
#define M_OPLAPI_5(method, oplist, x, ...)   , x = M_DELAY3(method)(oplist, __VA_ARGS__)
#define M_OPLAPI_6(method, oplist, x, ...)   , M_DELAY3(method)(&x, &__VA_ARGS__)
#define M_OPLAPI_7(method, oplist, x, ...)   , M_DELAY3(method)(oplist, &x, &__VA_ARGS__)
/* API transformation support for indirection */
#define M_OPLAPI_INDIRECT_0          M_OPLAPI_ERROR
#define M_OPLAPI_INDIRECT_API_0(...) M_OPLAPI_0
#define M_OPLAPI_EXTRACT_API_0(...)  __VA_ARGS__
#define M_OPLAPI_INDIRECT_API_1(...) M_OPLAPI_1
#define M_OPLAPI_EXTRACT_API_1(...)  __VA_ARGS__
#define M_OPLAPI_INDIRECT_API_2(...) M_OPLAPI_2
#define M_OPLAPI_EXTRACT_API_2(...)  __VA_ARGS__
#define M_OPLAPI_INDIRECT_API_3(...) M_OPLAPI_3
#define M_OPLAPI_EXTRACT_API_3(...)  __VA_ARGS__
#define M_OPLAPI_INDIRECT_API_4(...) M_OPLAPI_4
#define M_OPLAPI_EXTRACT_API_4(...)  __VA_ARGS__
#define M_OPLAPI_INDIRECT_API_5(...) M_OPLAPI_5
#define M_OPLAPI_EXTRACT_API_5(...)  __VA_ARGS__
#define M_OPLAPI_INDIRECT_API_6(...) M_OPLAPI_6
#define M_OPLAPI_EXTRACT_API_6(...)  __VA_ARGS__
#define M_OPLAPI_INDIRECT_API_7(...) M_OPLAPI_7
#define M_OPLAPI_EXTRACT_API_7(...)  __VA_ARGS__


/* Define the no default function that generates a compiler error
   if the method is expanded. 
*/
#define M_NO_DEFAULT(...)                                                     \
  M_STATIC_FAILURE(M_LIB_MISSING_METHOD,                                      \
  "The requested operator has no method registered in the given OPLIST. ")

#define M_NO_DEF_TYPE                                                         \
  M_STATIC_FAILURE(M_LIB_MISSING_METHOD,                                      \
  "The requested operator has no type/subtype/suboplist registered in the given OPLIST. ")

/* Test if the given variable is a basic C variable:
   int, float, enum, bool or compatible.
   NOTE: Not perfect, but catch some errors */
#define M_CHECK_DEFAULT_TYPE(a)                                               \
  M_STATIC_ASSERT(sizeof (a) <= M_MAX(sizeof(long long),                      \
                                M_MAX(sizeof (long double),                   \
                                      sizeof (uintmax_t))),                   \
      M_LIB_NOT_A_DEFAULT_TYPE,                                               \
      "The given variable is too big to be a default type,"                   \
      "but the used macro can only be used with such one."                    \
      "It is likely the given oplist is not right.")

/* Check if both variables are of the same type.
   The test compare their size.
   NOTE: Not perfect but catch some errors */
#define M_CHECK_SAME(a, b)                                                    \
  M_STATIC_ASSERT(sizeof(a) == sizeof(b),                                     \
                  M_LIB_NOT_SAME_TYPE,                                        \
                  "The variable " M_AS_STR(a) " and " M_AS_STR(b)             \
                  " are not of same type.")

/* Check if the oplist is compatible with the type.
   The oplist should define a TYPE method, in which case it is tested.
   If it is not exported, do nothing.
   Compare the type in C11, the size in C99 or C++.
   - name is the base name of the container.
   - inst is a number which is incremented for each use of this macro
   within the named container.
   - type is the type to test
   - oplist of the oplist to test to.   
*/
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define M_CHECK_COMPATIBLE_OPLIST(name, inst, type, oplist)                   \
  M_IF_METHOD(TYPE, oplist)                                                   \
  (                                                                           \
   static inline void M_C3(m_c0re_ctype_, name, inst)(void)                   \
  {                                                                           \
    type x;                                                                   \
    /* Proper check using _Generic */                                         \
    M_STATIC_ASSERT(_Generic(&x,                                              \
                             M_GET_TYPE oplist *: 1 /* Ok, type matches */,   \
                             default: 0 /* NOK, type doesn't match */ ),      \
                    M_LIB_TYPE_MISTMACH,                                      \
                    "The given type " M_AS_STR(type)                          \
                    " and the type of the oplist does not match: "            \
                    M_AS_STR(oplist) );                                       \
  }                                                                           \
  , /* End of TYPE */)
#else
#define M_CHECK_COMPATIBLE_OPLIST(name, inst, type, oplist)                   \
  M_IF_METHOD(TYPE, oplist)                                                   \
  (                                                                           \
   static inline void M_C3(m_c0re_ctype_, name, inst)(void)                   \
  {                                                                           \
    /* Imperfect check using size of type */                                  \
    M_STATIC_ASSERT(sizeof (type) == sizeof (M_GET_TYPE oplist),              \
                    M_LIB_TYPE_MISTMACH,                                      \
                    "The given type " M_AS_STR(type)                          \
                    " and the type of the oplist does not match: "            \
                    M_AS_STR(oplist) );                                       \
  }                                                                           \
  , /* End of TYPE */)
#endif


/* Define the default method.
   NOTE: M_SET_DEFAULT may be called in conditions where a and b
   are different but compatible type.
 */
#define M_INIT_DEFAULT(a)       ((a) = 0)
#define M_SET_DEFAULT(a,b)      (M_CHECK_DEFAULT_TYPE(a), (a) = (b))
#define M_NOTHING_DEFAULT(...)  ((void)(__VA_ARGS__))
#define M_EMPTY_DEFAULT(...)    ((void)1)
#define M_TRUE_DEFAULT(...)     true
#define M_NEW_DEFAULT(a)        M_MEMORY_ALLOC(a)
#define M_DEL_DEFAULT(a)        M_MEMORY_DEL(a)
#define M_REALLOC_DEFAULT(t,p,s) M_MEMORY_REALLOC(t,p,s)
#define M_FREE_DEFAULT(a)       M_MEMORY_FREE(a)
#define M_EQUAL_DEFAULT(a,b)    ((a) == (b))
#define M_CMP_DEFAULT(a,b)      ((a) < (b) ? -1 : (a) > (b))
#define M_ADD_DEFAULT(a,b,c)    ((a) = (b) + (c))
#define M_SUB_DEFAULT(a,b,c)    ((a) = (b) - (c))
#define M_MUL_DEFAULT(a,b,c)    ((a) = (b) * (c))
#define M_DIV_DEFAULT(a,b,c)    ((a) = (b) / (c))
#define M_AND_DEFAULT(a,b,c)    ((a) = (b) & (c))
#define M_OR_DEFAULT(a,b,c)     ((a) = (b) | (c))
#define M_NO_EXT_ALGO(n,co,to)
#define M_INC_ALLOC_DEFAULT(n)   (M_MAX(8, (n))*2)

/* Define the default limits:
 * - default maximum size of the basic type in "limb"
 * - default maximum size of the iterator type in "limb"
 * - default type for IT_REF (0: classic, 2: struct of { key_ptr, value_ptr} )
 * The default are fine for a lot of container except:
 * - dequee (basic type is bigger)
 * - rbtree
 * - B+Tree (it_ref is 2)
 */
#define M_LIMIT_BASE_TYPE_DFT   6
#define M_LIMIT_IT_TYPE_DFT     4
#define M_LIMIT_IT_REF_DFT      0
#define M_LIMITS_DEFAULT        ( M_LIMIT_BASE_TYPE_DFT, M_LIMIT_IT_TYPE_DFT, M_LIMIT_IT_REF_DFT)

/* NOTE: Theses operators are NOT compatible with the '[1]' tricks
   if the variable is defined as a parameter of a function
   (sizeof (a) is not portable). */
#define M_MOVE_DEFAULT(a,b)     (M_CHECK_SAME(a, b), M_MEMCPY_DEFAULT(a, b), memset(&(b), 0, sizeof (a)))
#define M_MEMCPY_DEFAULT(a,b)   (M_CHECK_SAME(a, b), memcpy(&(a), &(b), sizeof (a)))
#define M_MEMSET_DEFAULT(a)     (memset(&(a), 0, sizeof (a)))
#define M_MEMCMP1_DEFAULT(a,b)  (M_CHECK_SAME(a, b), memcmp(&(a), &(b), sizeof (a)) == 0)
#define M_MEMCMP2_DEFAULT(a,b)  (M_CHECK_SAME(a, b), memcmp(&(a), &(b), sizeof (a)))
#define M_SWAP_DEFAULT(el1, el2) do {                                         \
    char _tmp[sizeof (el1)];                                                  \
    M_CHECK_SAME(el1, el2);                                                   \
    memcpy(&_tmp, &(el1), sizeof (el1));                                      \
    memcpy(&(el1), &(el2), sizeof (el1));                                     \
    memcpy(&(el2), &_tmp, sizeof (el1));                                      \
  } while (0)

/* NOTE: Theses operators are to be used with the '[1]' tricks
   if the variable is defined as a parameter of a function
   (sizeof (a) is not portable). */
#define M_MOVE_A1_DEFAULT(a,b)     (M_CHECK_SAME(a[0], b[0]), M_MEMCPY_A1_DEFAULT(a, b), M_MEMSET_A1_DEFAULT(b))
#define M_MEMCPY_A1_DEFAULT(a,b)   (M_CHECK_SAME(a[0], b[0]), memcpy(&(a[0]), &(b[0]), sizeof (a[0])))
#define M_MEMSET_A1_DEFAULT(a)     (memset(&(a[0]), 0, sizeof (a[0])))
#define M_MEMCMP1_A1_DEFAULT(a,b)  (M_CHECK_SAME(a[0],b[0]), memcmp(&(a[0]), &(b[0]), sizeof (a[0])) == 0)
#define M_MEMCMP2_A1_DEFAULT(a,b)  (M_CHECK_SAME(a[0], b[0]), memcmp(&(a[0]), &(b[0]), sizeof (a[0])))
#define M_HASH_A1_DEFAULT(a)       (m_core_hash((const void*) &(a[0]), sizeof (a[0])) )


/* Default oplist for plain structure */
#define M_POD_OPLIST                                                          \
  (INIT(M_MEMSET_DEFAULT), INIT_SET(M_MEMCPY_DEFAULT), SET(M_MEMCPY_DEFAULT), \
   CLEAR(M_NOTHING_DEFAULT), EQUAL(M_MEMCMP1_DEFAULT), CMP(M_MEMCMP2_DEFAULT), \
   HASH(M_HASH_POD_DEFAULT), SWAP(M_SWAP_DEFAULT))


/* Default oplist for a structure defined with an array of size 1 */
#define M_A1_OPLIST                                                           \
  (INIT(M_MEMSET_A1_DEFAULT), INIT_SET(M_MEMCPY_A1_DEFAULT), SET(M_MEMCPY_A1_DEFAULT), \
   CLEAR(M_NOTHING_DEFAULT), EQUAL(M_MEMCMP1_A1_DEFAULT), CMP(M_MEMCMP2_A1_DEFAULT), \
   HASH(M_HASH_A1_DEFAULT))


/* Oplist for a type that does nothing and shall not be instanciated */
#define M_EMPTY_OPLIST                                                        \
  (INIT(M_EMPTY_DEFAULT), INIT_SET(M_EMPTY_DEFAULT),                          \
   SET(M_EMPTY_DEFAULT), CLEAR(M_EMPTY_DEFAULT),                              \
   INIT_MOVE(M_EMPTY_DEFAULT), MOVE(M_EMPTY_DEFAULT),                         \
   EQUAL(M_TRUE_DEFAULT), GET_STR(M_EMPTY_DEFAULT),                           \
   OUT_STR(M_EMPTY_DEFAULT), IN_STR(M_TRUE_DEFAULT),                          \
   OUT_SERIAL(M_EMPTY_DEFAULT), IN_SERIAL(M_TRUE_DEFAULT),                    \
   PARSE_STR(M_TRUE_DEFAULT))


/* Default oplist for C standard types (int & float).
   Implement generic out_str/in_str/parse_str/get_str function if using C11.
   Add FILE I/O if stdio.h has been included
*/
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L

# if M_USE_STDIO
/* C11 + FILE support */
#  define M_DEFAULT_OPLIST                                                    \
  (INIT(M_INIT_DEFAULT), INIT_SET(M_SET_DEFAULT), SET(M_SET_DEFAULT),         \
   CLEAR(M_NOTHING_DEFAULT), EQUAL(M_EQUAL_DEFAULT), CMP(M_CMP_DEFAULT),      \
   INIT_MOVE(M_MOVE_DEFAULT), MOVE(M_MOVE_DEFAULT) ,                          \
   ADD(M_ADD_DEFAULT), SUB(M_SUB_DEFAULT),                                    \
   MUL(M_MUL_DEFAULT), DIV(M_DIV_DEFAULT),                                    \
   HASH(M_HASH_DEFAULT), SWAP(M_SWAP_DEFAULT) ,                               \
   IN_STR(M_FSCAN_ARG M_IPTR), OUT_STR(M_FPRINT_ARG),                         \
   IN_SERIAL(M_IN_SERIAL_DEFAULT_ARG M_IPTR), OUT_SERIAL(M_OUT_SERIAL_DEFAULT_ARG), \
   PARSE_STR(M_PARSE_DEFAULT_TYPE M_IPTR), M_GET_STR_METHOD_FOR_DEFAULT_TYPE)
# else
/* C11 + No FILE support */
#   define M_DEFAULT_OPLIST                                                   \
  (INIT(M_INIT_DEFAULT), INIT_SET(M_SET_DEFAULT), SET(M_SET_DEFAULT),         \
   CLEAR(M_NOTHING_DEFAULT), EQUAL(M_EQUAL_DEFAULT), CMP(M_CMP_DEFAULT),      \
   INIT_MOVE(M_MOVE_DEFAULT), MOVE(M_MOVE_DEFAULT) ,                          \
   ADD(M_ADD_DEFAULT), SUB(M_SUB_DEFAULT),                                    \
   MUL(M_MUL_DEFAULT), DIV(M_DIV_DEFAULT),                                    \
   HASH(M_HASH_DEFAULT), SWAP(M_SWAP_DEFAULT) ,                               \
   IN_SERIAL(M_IN_SERIAL_DEFAULT_ARG M_IPTR), OUT_SERIAL(M_OUT_SERIAL_DEFAULT_ARG), \
   PARSE_STR(M_PARSE_DEFAULT_TYPE M_IPTR), M_GET_STR_METHOD_FOR_DEFAULT_TYPE)
# endif
#else
/* C99 */
# define M_DEFAULT_OPLIST                                                     \
  (INIT(M_INIT_DEFAULT), INIT_SET(M_SET_DEFAULT), SET(M_SET_DEFAULT),         \
   CLEAR(M_NOTHING_DEFAULT), EQUAL(M_EQUAL_DEFAULT), CMP(M_CMP_DEFAULT),      \
   INIT_MOVE(M_MOVE_DEFAULT), MOVE(M_MOVE_DEFAULT) ,                          \
   ADD(M_ADD_DEFAULT), SUB(M_SUB_DEFAULT),                                    \
   MUL(M_MUL_DEFAULT), DIV(M_DIV_DEFAULT),                                    \
   HASH(M_HASH_DEFAULT), SWAP(M_SWAP_DEFAULT)                         )
#endif


/* Specialized oplist for a boolean.
 * M_DEFAULT_OPLIST is nearly ok, except for ADD/SUB/MUL/DIV
 * that generates warnings with boolean.
 */
#define M_BOOL_OPLIST                                                         \
  M_OPEXTEND(M_DEFAULT_OPLIST, ADD(M_OR_DEFAULT), MUL(M_AND_DEFAULT),         \
              SUB(0), DIV(0))


/* Specialized oplist for an enumerate.
 * M_DEFAULT_OPLIST is nearly ok, except if build in C++ mode.
 * Also I/O are specialized and arithmetics are removed
 * OPLIST doesn't store an oplist but an additional parameter
 * (the initial value)
 */
#if M_USE_STDIO
/*  FILE support */
#define M_ENUM_OPLIST(type, init)                                             \
  (INIT(API_1(M_ENUM_INIT)), INIT_SET(M_SET_DEFAULT),                         \
   SET(M_SET_DEFAULT), CLEAR(M_NOTHING_DEFAULT),                              \
   EQUAL(M_EQUAL_DEFAULT), CMP(M_CMP_DEFAULT),                                \
   INIT_MOVE(M_MOVE_DEFAULT), MOVE(M_MOVE_DEFAULT) ,                          \
   HASH(M_HASH_POD_DEFAULT), SWAP(M_SWAP_DEFAULT),                            \
   TYPE(type), OPLIST(init),                                                  \
   IN_STR(API_1(M_ENUM_FSCAN)), OUT_STR(M_ENUM_FPRINT),                       \
   IN_SERIAL(API_1(M_ENUM_IN_SERIAL)), OUT_SERIAL(M_ENUM_OUT_SERIAL),         \
   PARSE_STR(API_1(M_ENUM_PARSE)), M_GET_STR_METHOD_FOR_ENUM_TYPE             \
  )
#else
/* No File support */
#define M_ENUM_OPLIST(type, init)                                             \
  (INIT(API_1(M_ENUM_INIT)), INIT_SET(M_SET_DEFAULT),                         \
   SET(M_SET_DEFAULT), CLEAR(M_NOTHING_DEFAULT),                              \
   EQUAL(M_EQUAL_DEFAULT), CMP(M_CMP_DEFAULT),                                \
   INIT_MOVE(M_MOVE_DEFAULT), MOVE(M_MOVE_DEFAULT) ,                          \
   HASH(M_HASH_POD_DEFAULT), SWAP(M_SWAP_DEFAULT),                            \
   TYPE(type), OPLIST(init),                                                  \
   IN_SERIAL(API_1(M_ENUM_IN_SERIAL)), OUT_SERIAL(M_ENUM_OUT_SERIAL),         \
   PARSE_STR(API_1(M_ENUM_PARSE)), M_GET_STR_METHOD_FOR_ENUM_TYPE             \
   )
#endif /* M_USE_STDIO */

/* It will be overloaded if m-string.h is included */
#define M_GET_STR_METHOD_FOR_ENUM_TYPE

/* Initialize an enum to its init value */
#define M_ENUM_INIT(oplist, var)                                              \
  ((var) = M_GET_OPLIST oplist )

/* Define helper functions for enum oplist */
/* Input/Output an enumerate.
   It is stored as a long long integer for best compatibility. */
#if M_USE_STDIO
static inline long long
m_core_fscan_enum (FILE *f)
{
  long long ret;
  int s = m_core_fscanf(f, "%lld", &ret) == 1;
  /* HACK: Push back the return code in FILE stream,
     so that it can be popped later with a fgetc */
  ungetc(s, f);
  return ret;
}
#define M_ENUM_FPRINT(f, var) fprintf( (f), "%lld", (long long) (var))
#define M_ENUM_FSCAN(oplist, var, f)                                          \
  ( var = (M_GET_TYPE oplist) (true ? m_core_fscan_enum(f) : 0), fgetc(f))
#endif /* M_USE_STDIO */

/* HACK: Parse two times to convert and then compute
   if the conversion succeeds for PARSE */
static inline long long
m_core_parse1_enum (const char str[])
{
  return strtoll(str, NULL, 10);
}
static inline bool
m_core_parse2_enum (const char str[], const char **endptr)
{
  char *end;
  strtoll(str, &end, 10);
  if (endptr != NULL) *endptr = end;
  return end != str;
}

#define M_ENUM_OUT_SERIAL(serial, var)                                        \
  ((serial)->m_interface->write_integer(serial, (long long) (var), sizeof (var)))
#define M_ENUM_IN_SERIAL(oplist, var, serial)                                 \
  ( var = (M_GET_TYPE oplist)(true ? m_core_in_serial_enum(serial) : 0), (serial)->tmp.r)
#define M_ENUM_GET_STR(str, var, append)                                      \
  ((append ? string_cat_printf : string_printf) (str, "%lld", (long long) (var) ))
#define M_ENUM_PARSE(oplist, var, str, endptr)                                \
  ( var = (M_GET_TYPE oplist) (true ? m_core_parse1_enum(str) : 0), m_core_parse2_enum(str, endptr))


/* Default oplist for standard types of pointers.
 */
#define M_PTR_OPLIST                                                          \
  (INIT(M_INIT_DEFAULT), INIT_SET(M_SET_DEFAULT), SET(M_SET_DEFAULT),         \
   CLEAR(M_NOTHING_DEFAULT), EQUAL(M_EQUAL_DEFAULT),                          \
   INIT_MOVE(M_MOVE_DEFAULT), MOVE(M_MOVE_DEFAULT) ,                          \
   SWAP(M_SWAP_DEFAULT)                         )


/* Default oplist for complex objects with "classic" names for methods.
 */
#define M_CLASSIC_OPLIST(name) (                                              \
  INIT(M_C(name, _init)),                                                     \
  INIT_SET(M_C(name, _init_set)),                                             \
  SET(M_C(name, _set)),                                                       \
  CLEAR(M_C(name, _clear)),                                                   \
  TYPE(M_C(name, _t)) )


/* OPLIST for 'const char *' string (with NO memory allocation).
 */
#define M_CSTR_EQUAL(a,b) (strcmp((a),(b)) == 0)
#define M_CSTR_OUT_STR(file, str) fprintf(file, "%s", str)
#define M_CSTR_OPLIST (INIT(M_INIT_DEFAULT), INIT_SET(M_SET_DEFAULT),         \
                       SET(M_SET_DEFAULT), CLEAR(M_NOTHING_DEFAULT),          \
                       HASH(m_core_cstr_hash), EQUAL(M_CSTR_EQUAL),           \
                       CMP(strcmp), TYPE(const char *),                       \
                       OUT_STR(M_CSTR_OUT_STR) )


/* From an oplist (...) return ... */
#define M_OPFLAT(...)     __VA_ARGS__

/* Concat two oplists in one. op1 will have higher priority to op2 */
#define M_OPCAT(op1,op2) (M_OPFLAT op1, M_OPFLAT op2)

/* Apply an oplist */
#define M_OPAPPLY(a, oplist)  a oplist

/* Extend an oplist by adding some methods */
#define M_OPEXTEND(op, ...) (__VA_ARGS__, M_OPFLAT op)

/* Return the content of a property named 'propname' 
   in the PROPERTIES field of oplist
   or 0, if it is not defined */
#define M_GET_OPPROPERTY(oplist, propname)                                    \
  M_GET_METHOD (propname, 0, M_OPFLAT M_GET_PROPERTIES oplist)

/* Test if a method is present in an oplist.
   Return 0 (method is absent or disabled) or 1 (method is present and not disabled).
   NOTE: Does not work if method is OPLIST, KEY_OPLIST or VALUE_OPLIST.
 */
#define M_TEST_METHOD_P(method, oplist)                                       \
  M_BOOL(M_GET_METHOD (method, 0, M_OPFLAT oplist))

/* Test if a method is disabled in an oplist.
   To disable an oplist, just put '0' in the method like this:
   Example: (INIT(0), CLEAR(clear))
   Here INIT is disabled, whereas CLEAR is not.
   Return 1 (method is disabled) or 0 (method is not disabled - absent or present) */
#define M_TEST_DISABLED_METHOD_P(method, oplist)                              \
  M_INV(M_BOOL(M_GET_METHOD (method, 1, M_OPFLAT oplist)))

/* Perfom a preprocessing M_IF, if the method is present in the oplist.
   Example: M_IF_METHOD(HASH, oplist)(define function with HASH method, ) */
#define M_IF_METHOD(method, oplist) M_IF(M_TEST_METHOD_P(method, oplist))

/* Perfom a preprocessing M_IF, if the method is disabled in the oplist.
   Example: M_IF_DISABLED_METHOD(HASH, oplist)(define function without HASH method, ) */
#define M_IF_DISABLED_METHOD(method, oplist) M_IF(M_TEST_DISABLED_METHOD_P(method, oplist))

/* Perform a preprocessing M_IF if the method exists in both oplist.
   Example: M_IF_METHOD_BOTH(HASH, oplist1, oplist2) (define function with HASH method, ) */
#define M_IF_METHOD_BOTH(method, oplist1, oplist2)                            \
  M_IF(M_AND(M_TEST_METHOD_P(method, oplist1), M_TEST_METHOD_P(method, oplist2)))

/* Perform a preprocessing M_IF if both methods exist in the oplist.
   Example: M_IF_METHOD2(HASH, CMP, oplist) (define function with HASH & CMP method, ) */
#define M_IF_METHOD2(method1, method2, oplist)                                \
  M_IF(M_AND(M_TEST_METHOD_P(method1, oplist), M_TEST_METHOD_P(method2, oplist)))

/* Perform a preprocessing M_IF if at least one method exists in the oplist.
   Example: M_IF_AT_LEAST_METHOD(HASH, CMP, oplist) (define function with HASH or CMP method, ) */
#define M_IF_AT_LEAST_METHOD(method1, method2, oplist)                        \
  M_IF(M_OR(M_TEST_METHOD_P(method1, oplist), M_TEST_METHOD_P(method2, oplist)))

/* Perform a preprocessing M_IF if both methods exists in both oplist.
   Example: M_IF_METHOD2_BOTH(HASH, INIT, oplist1, oplist2) (define function with HASH & INIT method, ) */
#define M_IF_METHOD2_BOTH(method1, method2, oplist1, oplist2)                 \
  M_IF(M_AND(M_AND(M_TEST_METHOD_P(method1, oplist1), M_TEST_METHOD_P(method1, oplist2)), \
             M_AND(M_TEST_METHOD_P(method2, oplist1), M_TEST_METHOD_P(method2, oplist2))))

/* Perform a preprocessing M_IF if the method exists for all oplist.
   Example: M_IF_METHOD_ALL(HASH, oplist1, oplist2) (define function with HASH method, ) */
#define M_IF_METHOD_ALL(method, ...)                                          \
  M_IF(M_REDUCE2(M_TEST_METHOD_P, M_AND, method, __VA_ARGS__))

/* Perform a preprocessing M_IF if both methods exist for all oplist.
   Example: M_IF_METHOD2_ALL(HASH, INIT, oplist1, oplist2) (define function with HASH & INIT methods,  ) */
#define M_IF_METHOD2_ALL(method1, method2, ...)                               \
  M_IF(M_REDUCE2(M_TEST_METHOD2_P, M_AND, (method1, method2), __VA_ARGS__))
#define M_TEST_METHOD2_P(method_pair, oplist)                                 \
  M_AND(M_TEST_METHOD_P(M_PAIR_1 method_pair, oplist), M_TEST_METHOD_P(M_PAIR_2 method_pair, oplist))

/* By putting this after a method in an oplist, we transform the argument list
   so that the first argument becomes a pointer to the destination. */
#define M_IPTR(...) ( & __VA_ARGS__ )

/* Perform an INIT_MOVE if present, or emulate it using INIT_SET/CLEAR */
#define M_DO_INIT_MOVE(oplist, dest, src) do {                                \
    M_IF_METHOD(INIT_MOVE, oplist)(M_CALL_INIT_MOVE(oplist, (dest), (src)),   \
                                   M_CALL_INIT_SET(oplist, (dest), (src)) ;   \
                                   M_CALL_CLEAR(oplist, (src) ));             \
  } while (0)

/* Perform a MOVE if present, or emulate it using CLEAR/INIT_MOVE
   if possible, or with SET/CLEAR otherwise                            */
#define M_DO_MOVE(oplist, dest, src) do {                                     \
    M_IF_METHOD(MOVE, oplist)       (M_CALL_MOVE(oplist, (dest), (src)),      \
        M_IF_METHOD(INIT_MOVE, oplist)(M_CALL_CLEAR(oplist, (dest));          \
                       M_CALL_INIT_MOVE(oplist, (dest), (src)),               \
                                       M_CALL_SET(oplist, (dest), (src));     \
                                       M_CALL_CLEAR(oplist, (src))            \
                                   ));                                        \
  } while (0)

/* Test if the argument is a valid oplist.
   NOTE: Incomplete test.
*/
#define M_OPLIST_P(a)                                                         \
  M_AND(M_PARENTHESIS_P(a), M_INV(M_PARENTHESIS_P (M_OPFLAT a)))

#define M_IF_OPLIST(a) M_IF(M_OPLIST_P(a))

/* If 'a' seems to be an oplist, it returns a,
   else if a symbol composed of M_OPL_##a() exists and is defined as an oplist, it returns it
   else it returns a.
   In short, if a global oplist is defined for the argument, it returns it
   otherwise it returns the argument.
   Global oplist is limited to typedef types.
*/
#define M_GLOBAL_OPLIST(a)                                                    \
  M_IF( M_OPLIST_P(a))(M_GLOBALI_ID, M_GLOBALI_OPLIST_ELSE)(a)
#define M_GLOBALI_ID(a)                     a
#define M_GLOBALI_OPLIST_ELSE(a)            M_GLOBALI_OPLIST_ELSE2(a, M_C(M_OPL_, a)())
#define M_GLOBALI_OPLIST_ELSE2(a, op)       M_IF( M_OPLIST_P (op))(op, a)

#define M_GLOBAL_TYPE(a)                                                      \
  M_IF( M_OPLIST_P(a))(M_GLOBALI_TYPE_GET, M_GLOBALI_ID)(a)
#define M_GLOBALI_TYPE_GET(a)              M_GET_TYPE a

/* If a symbol composed of M_OPL_##a() exists and is defined as an oplist,
   it returns it otherwise it returns M_DEFAULT_OPLIST.
   Global oplist is limited to typedef types.
   NOTE1: It first tests if the type doesn't start with a parenthesis,
   in which case concatenation cannot be used.
   NOTE: It doesn't test if M_OPL_##a() is exactly an oplist (M_OPLIST_P)
   but rather than if it starts with parenthesis: this is to allow
   M_OPL_a() to expand into an invalid oplist ((M_LIB_ERRROR()))
*/
#define M_GLOBAL_OPLIST_OR_DEF(a)                                             \
  M_IF( M_PARENTHESIS_P(a))(M_GLOBALI_OPLIST_DEFAULT1, M_GLOBALI_OPLIST_OR_DEF_ELSE)(a)
#define M_GLOBALI_OPLIST_DEFAULT1(a)          M_GLOBALI_OPLIST_DEFAULT2
#define M_GLOBALI_OPLIST_DEFAULT2()           M_DEFAULT_OPLIST
#define M_GLOBALI_OPLIST_OR_DEF_ELSE(a)       M_GLOBALI_OPLIST_OR_DEF_ELSE2(a, M_C(M_OPL_, a)())
#define M_GLOBALI_OPLIST_OR_DEF_ELSE2(a, op)  M_IF( M_PARENTHESIS_P(op))(M_C(M_OPL_, a), M_GLOBALI_OPLIST_DEFAULT2)


/* Register simple classic C types (no qualifier) */
#define M_OPL_char() M_DEFAULT_OPLIST
#define M_OPL_short() M_DEFAULT_OPLIST
#define M_OPL_int() M_DEFAULT_OPLIST
#define M_OPL_long() M_DEFAULT_OPLIST
#define M_OPL_float() M_DEFAULT_OPLIST
#define M_OPL_double() M_DEFAULT_OPLIST

/* Add as suffix for the given function the number of arguments of the calls.
   Can be used to call different function in function of the number of arguments. */
#define M_SUFFIX_FUNCTION_BY_NARGS(function, ...) M_C3(function, _, M_NARGS(__VA_ARGS__))

/* Call different INIT_WITH method in function of the number of arguments of the call,
 * to be used in an OPLIST.
 * Shall be used with API_1 call (example INIT_WITH(API_1(M_INIT_WITH_NVAR)) )
 * Shall define a NAME base method
 * All INIT_WITH methods shall be named as name ## _init_with_ ## NARGS
 */
#define M_INIT_WITH_NVAR(oplist, ...)                                         \
  M_SUFFIX_FUNCTION_BY_NARGS(M_C(M_GET_NAME oplist, _init_with), __VA_ARGS__)(__VA_ARGS__)


/************************************************************/
/******************** Syntax Enhancing **********************/
/************************************************************/

/* M_EACH enables iterating over a container using a for loop.
   First argument will be a created pointer var to the underlying type.
   Second argument is the container to iterate.
   Third argument can be the oplist of the list or the type of the list if a global
   oplist has been recorded.
   USAGE: for M_EACH(item, list, LIST_OPLIST) { M_LETaction; } 
*/
#define M_EACH(item, container, oplist)                                       \
  M_EACHI_OPLIST(item, container, M_GLOBAL_OPLIST(oplist))

/* Internal for M_EACH */
#define M_EACHI_OPLIST(item, container, oplist)                               \
  M_IF_METHOD(IT_REF, oplist)(M_EACHI, M_EACHI_CONST)                         \
  (item, container, oplist, M_C(local_iterator_, __LINE__),                   \
   M_C(local_cont_, __LINE__))

/* Internal for M_EACH with M_GET_IT_REF operator */
#define M_EACHI(item,container,oplist, iterator, cont)                        \
  (bool cont = true; cont; cont = false)                                      \
  for(M_GET_SUBTYPE oplist *item; cont ; cont = false)                        \
    for(M_GET_IT_TYPE oplist iterator; cont ; cont = false)                   \
      for(M_GET_IT_FIRST oplist (iterator, container) ;                       \
          !M_GET_IT_END_P oplist (iterator)                                   \
            && (item = M_GET_IT_REF oplist (iterator), true) ;                \
          M_GET_IT_NEXT oplist (iterator))

/* Internal for M_EACH with M_GET_IT_CREF operator */
#define M_EACHI_CONST(item,container,oplist, iterator, cont)                  \
  (bool cont = true; cont; cont = false)                                      \
  for(const M_GET_SUBTYPE oplist *item; cont ; cont = false)                  \
    for(M_GET_IT_TYPE oplist iterator; cont ; cont = false)                   \
      for(M_GET_IT_FIRST oplist (iterator, container) ;                       \
          !M_GET_IT_END_P oplist (iterator)                                   \
            && (item = M_GET_IT_CREF oplist (iterator), true) ;               \
          M_GET_IT_NEXT oplist (iterator))


/* M_LET defines, initializes & clears an object available within the next bracket.
   USAGE:
   M_LET(variable_list, variable_oplist|variable_type) { code }
   * variable_list can be a comma separated list of variable.
   * A variable alone is initialized to its default value.
   * A variable with initializer (like '(var, init value...)' 
      is initialized with this init values.
   NOTE: The code within {} can not perform return or goto command.
   break is supported and will exit the braket (calling the clear method)
   Last argument can be the oplist or the type itself if a global
   oplist has been recorded for this type.
 */
#define M_LET(a, ...)                                                         \
  M_ID(M_LETI0 ((M_REVERSE(a, __VA_ARGS__,                                    \
                        M_IF(M_PARENTHESIS_P(a))(M_LETI_VAR_NAME_A,           \
                                                 M_LETI_VAR_NAME_B)(a) ))))

// 1b. Generate a unique name based on the first variable and the line number
#define M_LETI_VAR_NAME_A(var) M_C3(_local_cont_, M_RET_ARG1 var, __LINE__)
#define M_LETI_VAR_NAME_B(var) M_C3(_local_cont_, var, __LINE__)
// 2. Evaluate with or without and inject oplist
#define M_LETI0(list)                                                         \
  M_LETI1 list
#define M_LETI1(cont, oplist, ...)                                            \
  M_LETI2(cont, M_GLOBAL_OPLIST(oplist), __VA_ARGS__)
// 3. Validate oplist before going any further
#define M_LETI2(cont, oplist, ...)                                            \
  M_IF_OPLIST(oplist)(M_LETI3, M_LETI2_FAILURE)(cont, oplist, __VA_ARGS__)
// Stop with a failure (invalid oplist)
#define M_LETI2_FAILURE(cont, oplist, ...)                                    \
  M_STATIC_FAILURE(M_LIB_NOT_AN_OPLIST, "(M_LET): the given argument is not a valid oplist: " M_AS_STR(oplist))
// 4. Map all variables to their own LET
#define M_LETI3(cont, oplist, ...)                                            \
  for(bool cont = true; cont ; /* unused */)                                  \
    M_MAP2(M_LETI_SINGLE, (cont, oplist), __VA_ARGS__)                        \
    for(;cont;cont = false)
// 5. Dispatch the right LET in function of having or not arguments 
#define M_LETI_SINGLE(data, name)                                             \
  M_IF(M_PARENTHESIS_P(name))(M_LETI_SINGLE2_SET,M_LETI_SINGLE2)              \
         (M_PAIR_1 data, M_PAIR_2 data, name, M_RET_ARG1 name, M_SKIPI_1 name)
// 6a. Define without argument ==> use the INIT operator
#define M_LETI_SINGLE2(cont, oplist, name, ...)                               \
  for(M_GET_TYPE oplist name;                                                 \
      cont && (M_CALL_INIT(oplist, name), true);                              \
      (M_CALL_CLEAR(oplist, name), cont = false))
// 6b. Define with arguments ==> use the INIT_SET or INIT_WITH operator (if defined).
#define M_LETI_SINGLE2_SET(cont, oplist, params, name, ...)                   \
  for(M_GET_TYPE oplist name;                                                 \
      cont && (M_LETI_SINGLE2_INIT(oplist, name, __VA_ARGS__), true);         \
      (M_CALL_CLEAR(oplist, name), cont = false))
// 6c. Use of INIT_SET or INIT_WITH.
#define M_LETI_SINGLE2_INIT(oplist, name, ...)                                \
  M_IF_METHOD(INIT_WITH,oplist)(M_LETI_SINGLE3_INIT, M_CALL_INIT_SET)(oplist, name, __VA_ARGS__)
#define M_LETI_SINGLE3_INIT(oplist, name, ...)                                \
  M_IF(M_NOTEQUAL( M_NARGS(__VA_ARGS__), 1))(M_CALL_INIT_WITH, M_LETI_SINGLE4_INIT)(oplist, name, __VA_ARGS__)
#define M_LETI_SINGLE4_INIT(oplist, name, arg)                                \
  M_IF(M_PARENTHESIS_P( arg ) )(M_CALL_INIT_WITH, M_CALL_INIT_SET)(oplist, name, M_REMOVE_PARENTHESIS (arg))


/* Transform the va list by adding their number as the first argument of
   the list.
   Example:   M_VA(a,b,c,d,e) ==> 5,a,b,c,d,e
   TODO: Use of a structure instead of an integer to ensure limited syntax control.
*/
#define M_VA(...) M_NARGS(__VA_ARGS__), __VA_ARGS__


/* Defer the evaluation of the given expression until the closing brace.
   M_DEFER(code) { }
   Example:
     M_DEFER(free(p)) {
             // code using p
     } // Here p is free
*/
#define M_DEFER(clear)                                                        \
  M_DEFER_INTERNAL(clear, m_var_ ## __LINE__)

#define M_DEFER_INTERNAL(clear, cont)                                         \
  for(bool cont = true; cont; cont = false)                                   \
    for( (void) 0; cont ; (clear), cont = false)                              \
      for( (void) 0; cont; cont = false)                                      \


/* Declare a variable, initialize it, continue if the initialization succeeds,
   and clears the variable afterwards.
   Otherwise, stop the execution and execute else_code if defined.
   M_LET_IF(init code, test code, clear code[, else_code]) { code using the variable }
   Example:
   M_LET_IF(void * p = malloc(100), p!=0, free(p)) {
      // code using p
   } // Here p is free
*/
#define M_LET_IF(init, test, ...)                                             \
  M_IF_NARGS_EQ1(__VA_ARGS__)                                                 \
  (                                                                           \
   M_LET_IF_INTERNAL (init, test, __VA_ARGS__, (void)0, M_C(m_var_, __LINE__)), \
   M_LET_IF_INTERNAL (init, test, __VA_ARGS__, M_C(m_var_, __LINE__))         \
  )

#define M_LET_IF_INTERNAL(init, test, clear, else_a, cont)                    \
  for(bool cont = true; cont; cont = false)                                   \
    for( init ; cont && ( (test) || (else_a, false) ); (clear), cont = false) \
      for( (void) 0; cont; cont = false)                                      \


/* Initialize the container 'dest' as per 'oplist' INIT operator
   and fill it with the given VA arguments with the PUSH operator.
   NOTE: If the REVERSE operator exists, it assumes a list,
   so it reverses the final order.
*/
#define M_INIT_VAI(oplist, dest, ...)                                         \
  (void)(M_GET_INIT oplist (dest) ,                                           \
         M_MAP2_C(M_INIT_VAI_FUNC, (dest, M_GET_PUSH oplist) , __VA_ARGS__)   \
         M_IF_METHOD(REVERSE, oplist)(M_DEFERRED_COMMA M_GET_REVERSE oplist (dest), ) \
         )
#define M_INIT_VAI_FUNC(d, a)                                                 \
  M_PAIR_2 d (M_PAIR_1 d, a)


/* Initialize the container 'dest' as per 'oplist' INIT operator
   and fill it with the given VA arguments with the _push_raw method
   allowing INIT_WITH operator for the pushed argument.
   NOTE: If the REVERSE operator exists, it assumes a list,
   so it reverses the final order.
*/
#define M_INIT_WITH_VAI(oplist, dest, ...)                                    \
  (void)(M_GET_INIT oplist (dest) ,                                           \
         M_MAP2_C(M_INIT_WITH_VAI22_FUNC, (dest, oplist) , __VA_ARGS__)       \
         M_IF_METHOD(REVERSE, oplist)(M_DEFERRED_COMMA M_GET_REVERSE oplist(dest), ) \
         )

#define M_INIT_WITH_VAI22_FUNC(pair, a)                                       \
  M_INIT_WITH_VAI23_FUNC(M_PAIR_1 pair, M_PAIR_2 pair, a)

#define M_INIT_WITH_VAI23_FUNC(d, op, a)                                      \
  M_IF(M_PARENTHESIS_P( a ))(M_CALL_INIT_WITH, M_CALL_INIT_SET)(M_GET_OPLIST op, *M_C( M_GET_NAME op , _push_raw)(d), M_REMOVE_PARENTHESIS (a))


/* Initialize the container 'dest' as per 'oplist' INIT operator
   and fill it with the given VA argument
   assumed to be pair (key,value) with the SET_KEY operator.
*/
#define M_INIT_KEY_VAI(oplist, dest, ...)                                     \
  (void)(M_GET_INIT oplist (dest) ,                                           \
         M_MAP2_C(M_INIT_KEY_VAI_FUNC, (dest, M_GET_SET_KEY oplist) , __VA_ARGS__))
#define M_INIT_KEY_VAI_FUNC(d, a)                                             \
  M_PAIR_2 d (M_PAIR_1 d, M_PAIR_1 a, M_PAIR_2 a)


/************************************************************/
/******************* Exponential Backoff ********************/
/************************************************************/

/* Can be increased / decreased by user code if needed
   to increase / decrease backoff of code */
#ifndef M_USE_BACKOFF_MAX_COUNT
#define M_USE_BACKOFF_MAX_COUNT 6
#endif

/* Exponential backoff object.
 * An exponential backoff object will increase its wait time
 * each time it is called with a pseudo random wait.
 * It is used to avoid too many threads trying to grab some atomic
 * variables at the same times (it makes the threads waits randomly).
 */
typedef struct m_core_backoff_s {
  unsigned int count;               // Number of times it has been run
  unsigned int seed;                // Initial seed
} m_core_backoff_ct[1];

/* Initialize a backoff object.
 * Use the C function rand to initialize its internal seed.
 * It should be good enough for the purpose of the backoff */
static inline void
m_core_backoff_init(m_core_backoff_ct backoff)
{
  backoff->count = 0;
  backoff->seed  = (unsigned int) rand();
}

/* Reset the count of the backoff object */
static inline void
m_core_backoff_reset(m_core_backoff_ct backoff)
{
  backoff->count = 0;
}

/* Wait a few nanoseconds using an active loop,
 * generating a random number of nanosecond to wait,
 * and increment the number of times wait has been called */
static inline void
m_core_backoff_wait(m_core_backoff_ct backoff)
{
  /* x is qualified as volatile to avoid being optimized away
     by the compiler in the active sleep loop */
  volatile int x = 0;
  /* Cheap but fast pseudo random. */
  backoff->seed = backoff->seed * 34721 + 17449;
  const unsigned int mask = (1U << backoff->count) -1;
  const unsigned int count = mask & (backoff->seed >> 8);
  /* Active sleep for 'count' times */
  for(unsigned int i = 0; i <= count; i++)
    x = 0;
  (void) x;
  /* Increment count for next step if needed */
  backoff->count += (backoff->count < M_USE_BACKOFF_MAX_COUNT);
}

/* Clear the backoff object */
static inline void
m_core_backoff_clear(m_core_backoff_ct backoff)
{
  // Nothing to do
  (void) backoff;
}


/************************************************************/
/********************** Serialization ***********************/
/************************************************************/

/* Forward declaration of string_t defined in m-string.h */
struct string_s;

/* Serialization Return code:
 * - OK & done (object is fully parsed),
 * - OK & continue parsing (object is partially parsed)
 * - Fail parsing
 */
typedef enum m_serial_return_code_e {
  M_SERIAL_OK_DONE = 0, M_SERIAL_OK_CONTINUE = 1, M_SERIAL_FAIL = 2
} m_serial_return_code_t;

#if defined(__cplusplus)
// C++ doesn't allow mixing enum and integer, so we provided overloaded operators.
inline void operator|(m_serial_return_code_t &a, m_serial_return_code_t b)
{
  a = static_cast<m_serial_return_code_t>(static_cast<int>(a) | static_cast<int>(b));
}
inline m_serial_return_code_t operator|=(m_serial_return_code_t a, m_serial_return_code_t b)
{
  return static_cast<m_serial_return_code_t>(static_cast<int>(a) | static_cast<int>(b));
}
inline m_serial_return_code_t operator&(m_serial_return_code_t a, m_serial_return_code_t b)
{
  return static_cast<m_serial_return_code_t>(static_cast<int>(a) & static_cast<int>(b));
}
#endif

/* Maximum data size of a serializator structure
 * Can be overloaded by user */
#ifndef M_USE_SERIAL_MAX_DATA_SIZE
#define M_USE_SERIAL_MAX_DATA_SIZE 4
#endif

/* Different types of types that can be stored in a serial object to represent it:
 * a boolean
 * different kind of integers
 * different kind of floats
 * a size
 * a pointer to something.
 * a serial return code
 */
typedef union m_serial_ll_u {
  bool                   b;
  char                   c;
  int                    i;
  long                   l;
  long long              ll;
  float                  f;
  double                 d;
  long double            e;
  size_t                 s;
  uintptr_t              u;
  void                  *p;
  const char            *cstr;
  m_serial_return_code_t r;
} m_serial_ll_ct;

/* Object to handle the construction of a serial write/read of an object
   that needs multiple calls (array, map, ...)
   It is common to all calls to the same object.
   It shall be used as a local state of the object being parsed */
typedef struct m_serial_local_s {
  m_serial_ll_ct data[M_USE_SERIAL_MAX_DATA_SIZE];
} m_serial_local_t[1];

/* Object to handle the generic serial read of an object:
 *  - m_interface is the pointer to the constant interface object that has all callbacks
 *  - tmp is temporary variable used localy by the non recursive serializer (not to be used by serializer)
 *  - data is user defined data to use by the serialization object as it wants
 * NOTE: 'interface' word cannot be used as a field name as some system headers
 * define it as a macro.
 * */
typedef struct m_serial_read_s {
  const struct m_serial_read_interface_s *m_interface;
  m_serial_ll_ct                          tmp;
  m_serial_ll_ct                          data[M_USE_SERIAL_MAX_DATA_SIZE];
} m_serial_read_t[1];

/* Interface that has to be exported by the serial read object.
 * All function pointers shall be not null.
 */
typedef struct m_serial_read_interface_s {
  m_serial_return_code_t (*read_boolean)(m_serial_read_t,bool *);
  m_serial_return_code_t (*read_integer)(m_serial_read_t, long long *, const size_t size_of_type);
  m_serial_return_code_t (*read_float)(m_serial_read_t, long double *, const size_t size_of_type);
  m_serial_return_code_t (*read_string)(m_serial_read_t, struct string_s *); 
  m_serial_return_code_t (*read_array_start)(m_serial_local_t, m_serial_read_t, size_t *);
  m_serial_return_code_t (*read_array_next)(m_serial_local_t, m_serial_read_t);
  m_serial_return_code_t (*read_map_start)(m_serial_local_t, m_serial_read_t, size_t *);
  m_serial_return_code_t (*read_map_value)(m_serial_local_t, m_serial_read_t);
  m_serial_return_code_t (*read_map_next)(m_serial_local_t, m_serial_read_t);
  m_serial_return_code_t (*read_tuple_start)(m_serial_local_t, m_serial_read_t);
  m_serial_return_code_t (*read_tuple_id)(m_serial_local_t, m_serial_read_t, const char *const field_name [], const int max, int *);
  m_serial_return_code_t (*read_variant_start)(m_serial_local_t, m_serial_read_t, const char *const field_name[], const int max, int*);
  m_serial_return_code_t (*read_variant_end)(m_serial_local_t, m_serial_read_t);
} m_serial_read_interface_t;


/* Object to handle the generic serial write of an object:
 *  - m_interface is the pointer to the constant interface object that has all callbacks
 *  - tmp is temporary variable used localy by the non recursive serializer
 *  - data is user defined data to use by the serialization object as it wants
 * NOTE: 'interface' word cannot be used as a field name as some system headers
 * define it as a macro.
 * */
typedef struct m_serial_write_s {
  const struct m_serial_write_interface_s *m_interface;
  m_serial_ll_ct                           tmp;
  m_serial_ll_ct                           data[M_USE_SERIAL_MAX_DATA_SIZE];
} m_serial_write_t[1];

/* Interface that has to be exported by the serial write object.
 * All function pointers shall be not null.
 */
typedef struct m_serial_write_interface_s {
  m_serial_return_code_t (*write_boolean)(m_serial_write_t,const bool data);
  m_serial_return_code_t (*write_integer)(m_serial_write_t,const long long data, const size_t size_of_type);
  m_serial_return_code_t (*write_float)(m_serial_write_t, const long double data, const size_t size_of_type);
  m_serial_return_code_t (*write_string)(m_serial_write_t,const char data[], size_t len); 
  m_serial_return_code_t (*write_array_start)(m_serial_local_t, m_serial_write_t, const size_t number_of_elements);
  m_serial_return_code_t (*write_array_next)(m_serial_local_t, m_serial_write_t);
  m_serial_return_code_t (*write_array_end)(m_serial_local_t, m_serial_write_t);
  m_serial_return_code_t (*write_map_start)(m_serial_local_t, m_serial_write_t, const size_t number_of_elements);
  m_serial_return_code_t (*write_map_value)(m_serial_local_t, m_serial_write_t);
  m_serial_return_code_t (*write_map_next)(m_serial_local_t, m_serial_write_t);
  m_serial_return_code_t (*write_map_end)(m_serial_local_t, m_serial_write_t);
  m_serial_return_code_t (*write_tuple_start)(m_serial_local_t, m_serial_write_t);
  m_serial_return_code_t (*write_tuple_id)(m_serial_local_t, m_serial_write_t, const char * const field_name[], const int max, const int index);
  m_serial_return_code_t (*write_tuple_end)(m_serial_local_t, m_serial_write_t);
  m_serial_return_code_t (*write_variant_start)(m_serial_local_t, m_serial_write_t, const char * const field_name[], const int max, const int index);
  m_serial_return_code_t (*write_variant_end)(m_serial_local_t, m_serial_write_t);
} m_serial_write_interface_t;


/* Convert a C default variale (bool, integer, float) to a Serialized data
 * by calling the serializer interface associated to the type of the object.
 * NOTE: Supports only C11.
*/
#define M_OUT_SERIAL_DEFAULT_ARG(serial, x)                                   \
  _Generic(((void)0,(x)),                                                     \
           bool: (serial)->m_interface->write_boolean(serial, M_AS_TYPE(bool, (x))), \
           char: (serial)->m_interface->write_integer(serial, M_AS_TYPE(char,(x)), sizeof (x)), \
           signed char: (serial)->m_interface->write_integer(serial, M_AS_TYPE(signed char,(x)), sizeof (x)), \
           unsigned char: (serial)->m_interface->write_integer(serial, (long long) M_AS_TYPE(unsigned char,(x)), sizeof (x)), \
           signed short: (serial)->m_interface->write_integer(serial, M_AS_TYPE(signed short,(x)), sizeof (x)), \
           unsigned short: (serial)->m_interface->write_integer(serial, (long long) M_AS_TYPE(unsigned short,(x)), sizeof (x)), \
           signed int: (serial)->m_interface->write_integer(serial, M_AS_TYPE(signed int,(x)), sizeof (x)), \
           unsigned int: (serial)->m_interface->write_integer(serial, (long long) M_AS_TYPE(unsigned int,(x)), sizeof (x)), \
           long int: (serial)->m_interface->write_integer(serial, M_AS_TYPE(long,(x)), sizeof (x)), \
           unsigned long int: (serial)->m_interface->write_integer(serial, (long long) M_AS_TYPE(unsigned long,(x)), sizeof (x)), \
           long long int: (serial)->m_interface->write_integer(serial, M_AS_TYPE(long long,(x)), sizeof (x)), \
           unsigned long long int: (serial)->m_interface->write_integer(serial, (long long) M_AS_TYPE(unsigned long long,(x)), sizeof (x)), \
           float: (serial)->m_interface->write_float(serial, M_AS_TYPE(float,(x)), sizeof (x)), \
           double: (serial)->m_interface->write_float(serial, M_AS_TYPE(double,(x)), sizeof (x)), \
           long double: (serial)->m_interface->write_float(serial, M_AS_TYPE(long double,(x)), sizeof (x)), \
           const char *: (serial)->m_interface->write_string(serial, M_AS_TYPE(const char *,(x)), m_core_out_serial_strlen(M_AS_TYPE(const char *,(x))) ), \
           char *: (serial)->m_interface->write_string(serial, M_AS_TYPE(char *,(x)), m_core_out_serial_strlen(M_AS_TYPE(const char *,(x))) ), \
           const void *: M_SERIAL_FAIL /* unsupported */,                     \
           void *: M_SERIAL_FAIL /* unsupported */)

/* Convert a Serialized data to a C default variale (bool, integer, float)
 * by calling the serializer interface associated to the type of the object.
 * NOTE: Supports only C11.
*/
#define M_IN_SERIAL_DEFAULT_ARG(xptr, serial)                                 \
  _Generic(((void)0,*(xptr)),                                                 \
           bool: (serial)->m_interface->read_boolean(serial, M_AS_TYPE(bool *, xptr)), \
           char: m_core_in_serial_char(serial, M_AS_TYPE(char*,xptr)),        \
           signed char: m_core_in_serial_schar(serial, M_AS_TYPE(signed char*,xptr)), \
           unsigned char: m_core_in_serial_uchar(serial, M_AS_TYPE(unsigned char*,xptr)), \
           signed short: m_core_in_serial_sshort(serial, M_AS_TYPE(signed short*,xptr)), \
           unsigned short: m_core_in_serial_ushort(serial, M_AS_TYPE(unsigned short*,xptr)), \
           signed int: m_core_in_serial_sint(serial, M_AS_TYPE(signed int*,xptr)), \
           unsigned int: m_core_in_serial_uint(serial, M_AS_TYPE(unsigned int*,xptr)), \
           long int: m_core_in_serial_slong(serial, M_AS_TYPE(long*,xptr)),   \
           unsigned long int: m_core_in_serial_ulong(serial, M_AS_TYPE(unsigned long*,xptr)), \
           long long int: m_core_in_serial_sllong(serial, M_AS_TYPE(long long*,xptr)), \
           unsigned long long int: m_core_in_serial_ullong(serial, M_AS_TYPE(unsigned long long*,xptr)), \
           float: m_core_in_serial_float(serial, M_AS_TYPE(float*,xptr)),     \
           double: m_core_in_serial_double(serial, M_AS_TYPE(double*,xptr)),  \
           long double: m_core_in_serial_ldouble(serial, M_AS_TYPE(long double*,xptr)), \
           const char *: M_SERIAL_FAIL /* unsupported (size unknown) */,      \
           char *: M_SERIAL_FAIL /* unsupported  (size unknown) */,           \
           const void *: M_SERIAL_FAIL /* unsupported */,                     \
           void *: M_SERIAL_FAIL /* unsupported */)

/* Helper functions for M_IN_SERIAL_DEFAULT_ARG
   as we need to define a function per supported type in the generic expression */
#define M_IN_SERIAL_DEFAULT_TYPE_DEF(name, type, func, promoted_type)         \
  static inline m_serial_return_code_t                                        \
  name (m_serial_read_t serial, type *ptr)                                    \
  {                                                                           \
    promoted_type i;                                                          \
    m_serial_return_code_t r;                                                 \
    r = serial->m_interface->func(serial, &i, sizeof (type));                 \
    *ptr = (type) i;                                                          \
    return r;                                                                 \
  }

M_IN_SERIAL_DEFAULT_TYPE_DEF(m_core_in_serial_char, char, read_integer, long long)
M_IN_SERIAL_DEFAULT_TYPE_DEF(m_core_in_serial_schar, signed char, read_integer, long long)
M_IN_SERIAL_DEFAULT_TYPE_DEF(m_core_in_serial_uchar, unsigned char, read_integer, long long)
M_IN_SERIAL_DEFAULT_TYPE_DEF(m_core_in_serial_sshort, signed short, read_integer, long long)
M_IN_SERIAL_DEFAULT_TYPE_DEF(m_core_in_serial_ushort, unsigned short, read_integer, long long)
M_IN_SERIAL_DEFAULT_TYPE_DEF(m_core_in_serial_sint, signed int, read_integer, long long)
M_IN_SERIAL_DEFAULT_TYPE_DEF(m_core_in_serial_uint, unsigned int, read_integer, long long)
M_IN_SERIAL_DEFAULT_TYPE_DEF(m_core_in_serial_slong, signed long, read_integer, long long)
M_IN_SERIAL_DEFAULT_TYPE_DEF(m_core_in_serial_ulong, unsigned long, read_integer, long long)
M_IN_SERIAL_DEFAULT_TYPE_DEF(m_core_in_serial_sllong, signed long long, read_integer, long long)
M_IN_SERIAL_DEFAULT_TYPE_DEF(m_core_in_serial_ullong, unsigned long long, read_integer, long long)
M_IN_SERIAL_DEFAULT_TYPE_DEF(m_core_in_serial_float, float, read_float, long double)
M_IN_SERIAL_DEFAULT_TYPE_DEF(m_core_in_serial_double, double, read_float, long double)
M_IN_SERIAL_DEFAULT_TYPE_DEF(m_core_in_serial_ldouble, long double, read_float, long double)

/* Helper function for M_ENUM_IN_SERIAL */
static inline long long
m_core_in_serial_enum(m_serial_read_t serial)
{
  long long i;
  /* Store the return code temporary in the serialize object */
  serial->tmp.r = serial->m_interface->read_integer(serial, &i, sizeof (long long));
  return i;
}

/* Encapsulation of strlen to avoid warnings in M_OUT_SERIAL_DEFAULT_ARG
 * because of expanded code will call strlen with NULL (which is illegal)
 * However, the branch where it is called is unreachable, so the warning
 * is not justified. */
static inline size_t
m_core_out_serial_strlen(const char s[])
{
  M_ASSERT(s != NULL);
  return strlen(s);
}

/* Encapsulation of returning error,
 * A serializer should return 'm_core_serial_fail()' instead of directly
 * M_SERIAL_FAIL 
 * so that a breakpoint can be put on this function for debugging purpose.
 */
static inline m_serial_return_code_t
m_core_serial_fail(void)
{
  return M_SERIAL_FAIL;
}

M_END_PROTECTED_CODE

#endif
