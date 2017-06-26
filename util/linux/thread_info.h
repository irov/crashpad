// Copyright 2017 The Crashpad Authors. All rights reserved.
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

#ifndef CRASHPAD_UTIL_LINUX_THREAD_INFO_H_
#define CRASHPAD_UTIL_LINUX_THREAD_INFO_H_

#include <stdint.h>
#include <sys/types.h>
#include <sys/user.h>

#include <type_traits>

#include "build/build_config.h"
#include "util/linux/address_types.h"
#include "util/linux/scoped_ptrace_attach.h"
#include "util/misc/initialization_state_dcheck.h"
#include "util/numeric/int128.h"

#if defined(OS_ANDROID)
#include <android/api-level.h>
#endif

namespace crashpad {

//! \brief The set of general purpose registers for an architecture family.
union ThreadContext {
  ThreadContext();
  ~ThreadContext();

  //! \brief The general purpose registers used by the 32-bit variant of the
  //!     architecture.
  struct t32 {
#if defined(ARCH_CPU_X86_FAMILY)
    // Reflects user_regs_struct in sys/user.h.
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t eax;
    uint32_t xds;
    uint32_t xes;
    uint32_t xfs;
    uint32_t xgs;
    uint32_t orig_eax;
    uint32_t eip;
    uint32_t xcs;
    uint32_t eflags;
    uint32_t esp;
    uint32_t xss;
#elif defined(ARCH_CPU_ARM_FAMILY)
    // Reflects user_regs in sys/user.h.
    uint32_t regs[11];
    uint32_t fp;
    uint32_t ip;
    uint32_t sp;
    uint32_t lr;
    uint32_t pc;
    uint32_t cpsr;
    uint32_t orig_r0;
#else
#error Port.
#endif  // ARCH_CPU_X86_FAMILY
  } t32;

  //! \brief The general purpose registers used by the 64-bit variant of the
  //!     architecture.
  struct t64 {
#if defined(ARCH_CPU_X86_FAMILY)
    // Reflects user_regs_struct in sys/user.h.
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t rbp;
    uint64_t rbx;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rax;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t orig_rax;
    uint64_t rip;
    uint64_t cs;
    uint64_t eflags;
    uint64_t rsp;
    uint64_t ss;
    uint64_t fs_base;
    uint64_t gs_base;
    uint64_t ds;
    uint64_t es;
    uint64_t fs;
    uint64_t gs;
#elif defined(ARCH_CPU_ARM_FAMILY)
    // Reflects user_regs_struct in sys/user.h.
    uint64_t regs[31];
    uint64_t sp;
    uint64_t pc;
    uint64_t pstate;
#else
#error Port.
#endif  // ARCH_CPU_X86_FAMILY
  } t64;

#if defined(ARCH_CPU_X86_FAMILY) || defined(ARCH_CPU_ARM64)
  using NativeThreadContext = user_regs_struct;
#elif defined(ARCH_CPU_ARMEL)
  using NativeThreadContext = user_regs;
#else
#error Port.
#endif  // ARCH_CPU_X86_FAMILY || ARCH_CPU_ARM64

#if defined(ARCH_CPU_32_BITS)
  static_assert(sizeof(t32) == sizeof(NativeThreadContext), "Size mismatch");
#else  // ARCH_CPU_64_BITS
  static_assert(sizeof(t64) == sizeof(NativeThreadContext), "Size mismatch");
#endif  // ARCH_CPU_32_BITS
};
static_assert(std::is_standard_layout<ThreadContext>::value,
              "Not standard layout");

//! \brief The floating point registers used for an architecture family.
union FloatContext {
  FloatContext();
  ~FloatContext();

  //! \brief The floating point registers used by the 32-bit variant of the
  //!     architecture.
  struct f32 {
#if defined(ARCH_CPU_X86_FAMILY)
    // Reflects user_fpxregs_struct in sys/user.h
    struct fxsave {
      uint16_t cwd;
      uint16_t swd;
      uint16_t twd;
      uint16_t fop;
      uint32_t fip;
      uint32_t fcs;
      uint32_t foo;
      uint32_t fos;
      uint32_t mxcsr;
      uint32_t reserved;
      uint32_t st_space[32];
      uint32_t xmm_space[32];
      uint32_t padding[56];
    } fxsave;
#elif defined(ARCH_CPU_ARM_FAMILY)
    // Reflects user_fpregs in sys/user.h.
    struct fpregs {
      struct fp_reg {
        uint32_t sign1 : 1;
        uint32_t unused : 15;
        uint32_t sign2 : 1;
        uint32_t exponent : 14;
        uint32_t j : 1;
        uint32_t mantissa1 : 31;
        uint32_t mantisss0 : 32;
      } fpregs[8];
      uint32_t fpsr : 32;
      uint32_t fpcr : 32;
      uint8_t type[8];
      uint32_t init_flag;
    } fpregs;

    // Reflects user_vfp in sys/user.h.
    struct vfp {
      uint64_t fpregs[32];
      uint32_t fpscr;
    } vfp;

    bool have_fpregs;
    bool have_vfp;
#else
#error Port.
#endif  // ARCH_CPU_X86_FAMILY
  } f32;

  //! \brief The floating point registers used by the 64-bit variant of the
  //!     architecture.
  struct f64 {
#if defined(ARCH_CPU_X86_FAMILY)
    // Refelects user_fpregs_struct in sys/user.h
    struct fxsave {
      uint16_t cwd;
      uint16_t swd;
      uint16_t ftw;
      uint16_t fop;
      uint64_t rip;
      uint64_t rdp;
      uint32_t mxcsr;
      uint32_t mxcr_mask;
      uint32_t st_space[32];
      uint32_t xmm_space[64];
      uint32_t padding[24];
    } fxsave;
#elif defined(ARCH_CPU_ARM_FAMILY)
    uint128_struct vregs[32];
    uint32_t fpsr;
    uint32_t fpcr;
    uint8_t padding[8];
#else
#error Port.
#endif  // ARCH_CPU_X86_FAMILY
  } f64;

#if defined(ARCH_CPU_X86)
// __ANDROID_API_N__ is a proxy for determining whether unified headers are in
// use. It’s only defined by unified headers. Unified headers call this
// structure user_fpxregs_struct regardless of API level.
#if defined(OS_ANDROID) && __ANDROID_API__ <= 19 && !defined(__ANDROID_API_N__)
  using NativeFpxregs = user_fxsr_struct;
#else
  using NativeFpxregs = user_fpxregs_struct;
#endif  // OS_ANDROID
  static_assert(sizeof(f32::fxsave) == sizeof(NativeFpxregs), "Size mismatch");
#elif defined(ARCH_CPU_X86_64)
  static_assert(sizeof(f64::fxsave) == sizeof(user_fpregs_struct),
                "Size mismatch");
#elif defined(ARCH_CPU_ARMEL)
  static_assert(sizeof(f32::fpregs) == sizeof(user_fpregs), "Size mismatch");
  static_assert(sizeof(f32::vfp) == sizeof(user_vfp), "Size mismatch");
#elif defined(ARCH_CPU_ARM64)
  static_assert(sizeof(f64) == sizeof(user_fpsimd_struct), "Size mismatch");
#else
#error Port.
#endif  // ARCH_CPU_X86
};
static_assert(std::is_standard_layout<FloatContext>::value,
              "Not standard layout");

class ThreadInfo {
 public:
  ThreadInfo();
  ~ThreadInfo();

  //! \brief Initializes this object with information about the thread whose ID
  //!     is \a tid.
  //!
  //! This method must be called successfully prior to calling any other method
  //! in this class. This method may only be called once.
  //!
  //! It is unspecified whether the information that an object of this class
  //! returns is loaded at the time Initialize() is called or subsequently, and
  //! whether this information is cached in the object or not.
  //!
  //! \param[in] tid The thread ID to obtain information for.
  //!
  //! \return `true` on success, `false` on failure with a message logged.
  bool Initialize(pid_t tid);

  //! \brief Determines the target thread’s bitness.
  //!
  //! \return `true` if the target is 64-bit.
  bool Is64Bit();

  //! \brief Uses `ptrace` to collect general purpose registers from the target
  //!     thread and places the result in \a context.
  //!
  //! \param[out] context The registers read from the target thread.
  void GetGeneralPurposeRegisters(ThreadContext* context);

  //! \brief Uses `ptrace` to collect floating point registers from the target
  //!     thread and places the result in \a context.
  //!
  //! \param[out] context The registers read from the target thread.
  //!
  //! \return `true` on success, with \a context set. Otherwise, `false` with a
  //!     message logged.
  bool GetFloatingPointRegisters(FloatContext* context);

  //! \brief Uses `ptrace` to determine the thread-local storage address for the
  //!     target thread and places the result in \a address.
  //!
  //! \param[out] address The address of the TLS area.
  //!
  //! \return `true` on success. `false` on failure with a message logged.
  bool GetThreadArea(LinuxVMAddress* address);

 private:
  size_t GetGeneralPurposeRegistersAndLength(ThreadContext* context);

  ThreadContext context_;
  ScopedPtraceAttach attachment_;
  pid_t tid_;
  InitializationStateDcheck initialized_;
  bool is_64_bit_;
};

}  // namespace crashpad

#endif  // CRASHPAD_UTIL_LINUX_THREAD_INFO_H_
