/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 Arash Massoudieh
 * 
 * This file is part of OpenHydroQual.
 * 
 * OpenHydroQual is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * If you use this file in a commercial product, you must purchase a
 * commercial license. Contact arash.massoudieh@enviroinformatics.co for details.
 */


// Copyright (C) 2008-2015 Conrad Sanderson
// Copyright (C) 2008-2015 NICTA (www.nicta.com.au)
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.



#undef arma_hot
#undef arma_cold
#undef arma_pure
#undef arma_const
#undef arma_aligned
#undef arma_align_mem
#undef arma_warn_unused
#undef arma_deprecated
#undef arma_malloc
#undef arma_inline
#undef arma_noinline
#undef arma_ignore

#define arma_hot
#define arma_cold
#define arma_pure
#define arma_const
#define arma_aligned
#define arma_align_mem
#define arma_warn_unused
#define arma_deprecated
#define arma_malloc
#define arma_inline            inline
#define arma_noinline
#define arma_ignore(variable)  ((void)(variable))


#undef arma_fortran_noprefix
#undef arma_fortran_prefix

#undef arma_fortran2_noprefix
#undef arma_fortran2_prefix
 
#if defined(ARMA_BLAS_UNDERSCORE)
  #define arma_fortran2_noprefix(function) function##_
  #define arma_fortran2_prefix(function)   wrapper_##function##_
#else
  #define arma_fortran2_noprefix(function) function
  #define arma_fortran2_prefix(function)   wrapper_##function
#endif

#if defined(ARMA_USE_WRAPPER)
  #define arma_fortran(function) arma_fortran2_prefix(function)
  #define arma_wrapper(function) wrapper_##function
#else
  #define arma_fortran(function) arma_fortran2_noprefix(function)
  #define arma_wrapper(function) function
#endif

#define arma_fortran_prefix(function)   arma_fortran2_prefix(function)
#define arma_fortran_noprefix(function) arma_fortran2_noprefix(function)

#undef  ARMA_INCFILE_WRAP
#define ARMA_INCFILE_WRAP(x) <x>


#if defined(__CYGWIN__)
  #if defined(ARMA_USE_CXX11)
    #undef ARMA_USE_CXX11
    #undef ARMA_USE_EXTERN_CXX11_RNG
    #pragma message ("WARNING: disabled use of C++11 features in Armadillo, due to incomplete support for C++11 by Cygwin")
  #endif
#endif


#if defined(ARMA_USE_CXX11)
  
  #undef  ARMA_USE_U64S64
  #define ARMA_USE_U64S64
  
  #if !defined(ARMA_32BIT_WORD)
    #undef  ARMA_64BIT_WORD
    #define ARMA_64BIT_WORD
  #endif
  
  #if defined(ARMA_64BIT_WORD) && defined(SIZE_MAX)
    #if (SIZE_MAX < 0xFFFFFFFFFFFFFFFFull)
      #pragma message ("WARNING: disabled use of 64 bit integers, as std::size_t is smaller than 64 bits")
      #undef ARMA_64BIT_WORD
    #endif
  #endif
  
#endif


#if defined(ARMA_64BIT_WORD)
  #undef  ARMA_USE_U64S64
  #define ARMA_USE_U64S64
#endif


// most compilers can't vectorise slightly elaborate loops;
// for example clang: http://llvm.org/bugs/show_bug.cgi?id=16358
#undef  ARMA_SIMPLE_LOOPS
#define ARMA_SIMPLE_LOOPS

#undef ARMA_GOOD_COMPILER

#undef ARMA_HAVE_TR1
#undef ARMA_HAVE_GETTIMEOFDAY
#undef ARMA_HAVE_SNPRINTF
#undef ARMA_HAVE_ISFINITE
#undef ARMA_HAVE_LOG1P
#undef ARMA_HAVE_ISINF
#undef ARMA_HAVE_ISNAN


#if (defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 200112L))
  #define ARMA_HAVE_GETTIMEOFDAY
#endif


// posix_memalign() is part of IEEE standard 1003.1
// http://pubs.opengroup.org/onlinepubs/009696899/functions/posix_memalign.html
// http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/unistd.h.html
// http://sourceforge.net/p/predef/wiki/Standards/
#if ( defined(_POSIX_ADVISORY_INFO) && (_POSIX_ADVISORY_INFO >= 200112L) )
  #undef  ARMA_HAVE_POSIX_MEMALIGN
  #define ARMA_HAVE_POSIX_MEMALIGN
#endif


#if defined(__APPLE__)
  #undef  ARMA_BLAS_SDOT_BUG
  #define ARMA_BLAS_SDOT_BUG
  #undef  ARMA_HAVE_POSIX_MEMALIGN
#endif


#if defined(__MINGW32__)
  #undef ARMA_HAVE_POSIX_MEMALIGN
#endif


#undef ARMA_FNSIG

#if defined (__GNUG__)
  #define ARMA_FNSIG  __PRETTY_FUNCTION__
#elif defined (_MSC_VER)
  #define ARMA_FNSIG  __FUNCSIG__ 
#elif defined(__INTEL_COMPILER)
  #define ARMA_FNSIG  __FUNCTION__
#elif defined(ARMA_USE_CXX11)
  #define ARMA_FNSIG  __func__
#else 
  #define ARMA_FNSIG  "(unknown)"
#endif


#if (defined(__GNUG__) || defined(__GNUC__)) && (defined(__clang__) || defined(__INTEL_COMPILER) || defined(__NVCC__) || defined(__CUDACC__) || defined(__PGI) || defined(__PATHSCALE__))
  #undef  ARMA_FAKE_GCC
  #define ARMA_FAKE_GCC
#endif


#if defined(__GNUG__) && !defined(ARMA_FAKE_GCC)
  
  #undef  ARMA_GCC_VERSION
  #define ARMA_GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
  
  #if (ARMA_GCC_VERSION < 40200)
    #error "*** Need a newer compiler ***"
  #endif
  
  #if ( (ARMA_GCC_VERSION >= 40700) && (ARMA_GCC_VERSION <= 40701) )
    #error "gcc versions 4.7.0 and 4.7.1 are unsupported; use 4.7.2 or later"
    // due to http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53549
  #endif
  
  #define ARMA_GOOD_COMPILER
  
  #undef  arma_pure
  #undef  arma_const
  #undef  arma_aligned
  #undef  arma_align_mem
  #undef  arma_warn_unused
  #undef  arma_deprecated
  #undef  arma_malloc
  #undef  arma_inline
  #undef  arma_noinline
  
  #define arma_pure               __attribute__((__pure__))
  #define arma_const              __attribute__((__const__))
  #define arma_aligned            __attribute__((__aligned__))
  #define arma_align_mem          __attribute__((__aligned__(16)))
  #define arma_warn_unused        __attribute__((__warn_unused_result__))
  #define arma_deprecated         __attribute__((__deprecated__))
  #define arma_malloc             __attribute__((__malloc__))
  #define arma_inline      inline __attribute__((__always_inline__))
  #define arma_noinline           __attribute__((__noinline__))
  
  #undef  ARMA_HAVE_ALIGNED_ATTRIBUTE
  #define ARMA_HAVE_ALIGNED_ATTRIBUTE
  
  #if defined(ARMA_USE_CXX11)
    #if (ARMA_GCC_VERSION < 40800)
      #pragma message ("WARNING: compiler is in C++11 mode, but it has incomplete support for C++11 features;")
      #pragma message ("WARNING: if something breaks, you get to keep all the pieces.")
      #pragma message ("WARNING: to forcefully prevent Armadillo from using C++11 features,")
      #pragma message ("WARNING: #define ARMA_DONT_USE_CXX11 before #include <armadillo>")
      #define ARMA_DONT_USE_CXX11_CHRONO
    #endif
  #endif
  
  #if !defined(ARMA_USE_CXX11)
    #if defined(_GLIBCXX_USE_C99_MATH_TR1) && defined(_GLIBCXX_USE_C99_COMPLEX_TR1)
      #define ARMA_HAVE_TR1
    #endif
  #endif
  
  #if (ARMA_GCC_VERSION >= 40300)
    #undef  arma_hot
    #undef  arma_cold
    
    #define arma_hot  __attribute__((__hot__))
    #define arma_cold __attribute__((__cold__))
  #endif
  
  #if (ARMA_GCC_VERSION >= 40700)
    #define ARMA_HAVE_GCC_ASSUME_ALIGNED
  #endif
  
  // gcc's vectoriser can handle elaborate loops
  #undef ARMA_SIMPLE_LOOPS
  
  #if defined(__OPTIMIZE_SIZE__)
    #define ARMA_SIMPLE_LOOPS
  #endif
  
  #if (defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 200112L))
    #define ARMA_HAVE_SNPRINTF
    #define ARMA_HAVE_ISFINITE
    #define ARMA_HAVE_LOG1P
    #define ARMA_HAVE_ISINF
    #define ARMA_HAVE_ISNAN
  #endif
  
  #undef ARMA_GCC_VERSION
  
#endif


#if defined(__clang__) && (defined(__INTEL_COMPILER) || defined(__NVCC__) || defined(__CUDACC__) || defined(__PGI) || defined(__PATHSCALE__))
  #undef  ARMA_FAKE_CLANG
  #define ARMA_FAKE_CLANG
#endif


#if defined(__clang__) && !defined(ARMA_FAKE_CLANG)
  
  #define ARMA_GOOD_COMPILER
  
  #if !defined(__has_attribute)
    #define __has_attribute(x) 0
  #endif
  
  #if __has_attribute(__pure__)
    #undef  arma_pure
    #define arma_pure __attribute__((__pure__))
  #endif
  
  #if __has_attribute(__const__)
    #undef  arma_const
    #define arma_const __attribute__((__const__))
  #endif
  
  #if __has_attribute(__aligned__)
    #undef  arma_aligned
    #undef  arma_align_mem
    
    #define arma_aligned   __attribute__((__aligned__))
    #define arma_align_mem __attribute__((__aligned__(16)))
    
    #undef  ARMA_HAVE_ALIGNED_ATTRIBUTE
    #define ARMA_HAVE_ALIGNED_ATTRIBUTE
  #endif
  
  #if __has_attribute(__warn_unused_result__)
    #undef  arma_warn_unused
    #define arma_warn_unused __attribute__((__warn_unused_result__))
  #endif
  
  #if __has_attribute(__deprecated__)
    #undef  arma_deprecated
    #define arma_deprecated __attribute__((__deprecated__))
  #endif
  
  #if __has_attribute(__malloc__)
    #undef  arma_malloc
    #define arma_malloc __attribute__((__malloc__))
  #endif
  
  #if __has_attribute(__always_inline__)
    #undef  arma_inline
    #define arma_inline inline __attribute__((__always_inline__))
  #endif
  
  #if __has_attribute(__noinline__)
    #undef  arma_noinline
    #define arma_noinline __attribute__((__noinline__))
  #endif
  
  #if __has_attribute(__hot__)
    #undef  arma_hot
    #define arma_hot __attribute__((__hot__))
  #endif
  
  #if __has_attribute(__cold__)
    #undef  arma_cold
    #define arma_cold __attribute__((__cold__))
  #endif
  
  #if defined(__has_builtin) && __has_builtin(__builtin_assume_aligned)
    #undef  ARMA_HAVE_GCC_ASSUME_ALIGNED
    #define ARMA_HAVE_GCC_ASSUME_ALIGNED
  #endif
  
  #if defined(__apple_build_version__)
    #undef ARMA_USE_EXTERN_CXX11_RNG
    // because Apple engineers are too lazy to implement thread_local
  #endif
  
  #if (defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 200112L))
    #define ARMA_HAVE_SNPRINTF
    #define ARMA_HAVE_ISFINITE
    #define ARMA_HAVE_LOG1P
    #define ARMA_HAVE_ISINF
    #define ARMA_HAVE_ISNAN
  #endif
  
#endif


#if defined(__INTEL_COMPILER)
  
  #if (__INTEL_COMPILER_BUILD_DATE < 20090623)
    #error "*** Need a newer compiler ***"
  #endif
  
  #undef  ARMA_HAVE_GCC_ASSUME_ALIGNED
  #undef  ARMA_HAVE_ICC_ASSUME_ALIGNED
  #define ARMA_HAVE_ICC_ASSUME_ALIGNED
  
#endif


#if defined(_MSC_VER)
  
  #if (_MSC_VER < 1600)
    #error "*** Need a newer compiler ***"
  #endif
  
  #if (_MSC_VER < 1700)
    #pragma message ("WARNING: this compiler is outdated and has incomplete support for the C++ standard;")
    #pragma message ("WARNING: if something breaks, you get to keep all the pieces")
    #define ARMA_BAD_COMPILER
  #endif
  
  #if defined(ARMA_USE_CXX11)
    #if (_MSC_VER < 1800)
      #pragma message ("WARNING: compiler is in C++11 mode, but it has incomplete support for C++11 features;")
      #pragma message ("WARNING: if something breaks, you get to keep all the pieces")
    #endif
  #endif
  
  // #undef  arma_inline
  // #define arma_inline inline __forceinline
  
  #pragma warning(push)
  
  #pragma warning(disable: 4127)  // conditional expression is constant
  #pragma warning(disable: 4510)  // default constructor could not be generated
  #pragma warning(disable: 4511)  // copy constructor can't be generated
  #pragma warning(disable: 4512)  // assignment operator can't be generated
  #pragma warning(disable: 4513)  // destructor can't be generated
  #pragma warning(disable: 4514)  // unreferenced inline function has been removed
  #pragma warning(disable: 4522)  // multiple assignment operators specified
  #pragma warning(disable: 4623)  // default constructor can't be generated
  #pragma warning(disable: 4624)  // destructor can't be generated
  #pragma warning(disable: 4625)  // copy constructor can't be generated
  #pragma warning(disable: 4626)  // assignment operator can't be generated
  #pragma warning(disable: 4710)  // function not inlined
  #pragma warning(disable: 4711)  // call was inlined
  #pragma warning(disable: 4714)  // __forceinline can't be inlined
  
  // #if (_MANAGED == 1) || (_M_CEE == 1)
  //   
  //   // don't do any alignment when compiling in "managed code" mode 
  //   
  //   #undef  arma_aligned
  //   #define arma_aligned
  //   
  //   #undef  arma_align_mem
  //   #define arma_align_mem
  //  
  // #elif (_MSC_VER >= 1700)
  //   
  //   #undef  arma_align_mem
  //   #define arma_align_mem __declspec(align(16))
  //   
  //   #define ARMA_HAVE_ALIGNED_ATTRIBUTE
  //   
  //   // disable warnings: "structure was padded due to __declspec(align(16))"
  //   #pragma warning(disable: 4324)
  //   
  // #endif
  
#endif


#if defined(__SUNPRO_CC)
  
  // http://www.oracle.com/technetwork/server-storage/solarisstudio/training/index-jsp-141991.html
  // http://www.oracle.com/technetwork/server-storage/solarisstudio/documentation/cplusplus-faq-355066.html
  
  #if (__SUNPRO_CC < 0x5100)
    #error "*** Need a newer compiler ***"
  #endif
    
#endif


#if defined(log2)
  #undef log2
  #pragma message ("WARNING: detected 'log2' macro and undefined it")
#endif



// 
// whoever defined macros with the names "min" and "max" should be permanently removed from the gene pool

#if defined(min) || defined(max)
  #undef min
  #undef max
  #pragma message ("WARNING: detected 'min' and/or 'max' macros and undefined them;")
  #pragma message ("WARNING: you may wish to define NOMINMAX before including any windows header")
#endif
