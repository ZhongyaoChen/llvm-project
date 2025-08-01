//===-- Floating point environment manipulation functions -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIBC_SRC___SUPPORT_FPUTIL_FENVIMPL_H
#define LLVM_LIBC_SRC___SUPPORT_FPUTIL_FENVIMPL_H

#include "hdr/fenv_macros.h"
#include "hdr/math_macros.h"
#include "hdr/types/fenv_t.h"
#include "src/__support/CPP/type_traits.h"
#include "src/__support/libc_errno.h"
#include "src/__support/macros/attributes.h" // LIBC_INLINE
#include "src/__support/macros/config.h"
#include "src/__support/macros/optimization.h"
#include "src/__support/macros/properties/architectures.h"

#if defined(LIBC_TARGET_ARCH_IS_AARCH64) && defined(__ARM_FP)
#if defined(__APPLE__)
#include "aarch64/fenv_darwin_impl.h"
#else
#include "aarch64/FEnvImpl.h"
#endif

// The extra !defined(APPLE) condition is to cause x86_64 MacOS builds to use
// the dummy implementations below. Once a proper x86_64 darwin fenv is set up,
// the apple condition here should be removed.
#elif defined(LIBC_TARGET_ARCH_IS_X86) && !defined(__APPLE__)
#include "x86_64/FEnvImpl.h"
#elif defined(LIBC_TARGET_ARCH_IS_ARM) && defined(__ARM_FP)
#include "arm/FEnvImpl.h"
#elif defined(LIBC_TARGET_ARCH_IS_ANY_RISCV) && defined(__riscv_flen)
#include "riscv/FEnvImpl.h"
#else

namespace LIBC_NAMESPACE_DECL {
namespace fputil {

// All dummy functions silently succeed.

LIBC_INLINE int clear_except(int) { return 0; }

LIBC_INLINE int test_except(int) { return 0; }

LIBC_INLINE int get_except() { return 0; }

LIBC_INLINE int set_except(int) { return 0; }

LIBC_INLINE int raise_except(int) { return 0; }

LIBC_INLINE int enable_except(int) { return 0; }

LIBC_INLINE int disable_except(int) { return 0; }

LIBC_INLINE int get_round() { return FE_TONEAREST; }

LIBC_INLINE int set_round(int rounding_mode) {
  return (rounding_mode == FE_TONEAREST) ? 0 : 1;
}

LIBC_INLINE int get_env(fenv_t *) { return 0; }

LIBC_INLINE int set_env(const fenv_t *) { return 0; }

} // namespace fputil
} // namespace LIBC_NAMESPACE_DECL
#endif

namespace LIBC_NAMESPACE_DECL {
namespace fputil {

LIBC_INLINE static constexpr int
clear_except_if_required([[maybe_unused]] int excepts) {
  if (cpp::is_constant_evaluated()) {
    return 0;
  } else {
#ifndef LIBC_MATH_HAS_NO_EXCEPT
    if (math_errhandling & MATH_ERREXCEPT)
      return clear_except(excepts);
#endif // LIBC_MATH_HAS_NO_EXCEPT
    return 0;
  }
}

LIBC_INLINE static constexpr int
set_except_if_required([[maybe_unused]] int excepts) {
  if (cpp::is_constant_evaluated()) {
    return 0;
  } else {
#ifndef LIBC_MATH_HAS_NO_EXCEPT
    if (math_errhandling & MATH_ERREXCEPT)
      return set_except(excepts);
#endif // LIBC_MATH_HAS_NO_EXCEPT
    return 0;
  }
}

LIBC_INLINE static constexpr int
raise_except_if_required([[maybe_unused]] int excepts) {
  if (cpp::is_constant_evaluated()) {
    return 0;
  } else {
#ifndef LIBC_MATH_HAS_NO_EXCEPT
    if (math_errhandling & MATH_ERREXCEPT)
#ifdef LIBC_TARGET_ARCH_IS_X86_64
      return raise_except</*SKIP_X87_FPU*/ true>(excepts);
#else  // !LIBC_TARGET_ARCH_IS_X86
      return raise_except(excepts);
#endif // LIBC_TARGET_ARCH_IS_X86

#endif // LIBC_MATH_HAS_NO_EXCEPT
    return 0;
  }
}

LIBC_INLINE static constexpr void
set_errno_if_required([[maybe_unused]] int err) {
  if (!cpp::is_constant_evaluated()) {
#ifndef LIBC_MATH_HAS_NO_ERRNO
    if (math_errhandling & MATH_ERRNO)
      libc_errno = err;
#endif // LIBC_MATH_HAS_NO_ERRNO
  }
}

} // namespace fputil
} // namespace LIBC_NAMESPACE_DECL

#endif // LLVM_LIBC_SRC___SUPPORT_FPUTIL_FENVIMPL_H
