#ifndef SF_ENVIRONMENT_H
#define SF_ENVIRONMENT_H

#define SF_ENV_OS_WINDOWS 0
#define SF_ENV_OS_MACOS   0
#define SF_ENV_OS_LINUX   0
#define SF_ENV_OS_UNKNOWN 0

#if defined(_WIN32) || defined(_WIN64)
	#undef  SF_ENV_OS_WINDOWS
	#define SF_ENV_OS_WINDOWS 1
	#define SF_ENV_OS_NAME "Windows"
#elif defined(__APPLE__) && defined(__MACH__)
	#undef  SF_ENV_OS_MACOS
	#define SF_ENV_OS_MACOS 1
	#define SF_ENV_OS_NAME "macOS"
#elif defined(__linux__) || defined(__linux)
	#undef  SF_ENV_OS_LINUX
	#define SF_ENV_OS_LINUX 1
	#define SF_ENV_OS_NAME "Linux"
#else
	#undef  SF_ENV_OS_UNKNOWN
	#define SF_ENV_OS_UNKNOWN 1
	#define SF_ENV_OS_NAME "Unknown OS"
#endif

#define SF_ENV_COMPILER_CLANG   0
#define SF_ENV_COMPILER_GCC     0
#define SF_ENV_COMPILER_MSVC    0
#define SF_ENV_COMPILER_UNKNOWN 0

#if defined(__clang__)
	#undef  SF_ENV_COMPILER_CLANG
	#define SF_ENV_COMPILER_CLANG 1
	#define SF_ENV_COMPILER_NAME "Clang"
#elif defined(__GNUC__)
	#undef  SF_ENV_COMPILER_GCC
	#define SF_ENV_COMPILER_GCC 1
	#define SF_ENV_COMPILER_NAME "GCC"
#elif defined(_MSC_VER)
	#undef  SF_ENV_COMPILER_MSVC
	#define SF_ENV_COMPILER_MSVC 1
	#define SF_ENV_COMPILER_NAME "MSVC"
#else
	#undef  SF_ENV_COMPILER_UNKNOWN
	#define SF_ENV_COMPILER_UNKNOWN 1
	#define SF_ENV_COMPILER_NAME "Unknown compiler"
#endif

#define SF_ENV_ARCH_X86_64  0
#define SF_ENV_ARCH_X86     0
#define SF_ENV_ARCH_ARM64   0
#define SF_ENV_ARCH_ARM     0
#define SF_ENV_ARCH_UNKNOWN 0

#if defined(__x86_64__) || defined(__x86_64) || defined(__amd64__)  || defined(__amd64)  || defined(_M_X64) || defined(_M_AMD64)
	#undef  SF_ENV_ARCH_X86_64
	#define SF_ENV_ARCH_X86_64 1
	#define SF_ENV_ARCH_NAME "x86_64"
	#define SF_ENV_ARCH_BITS 64
#elif defined(__i386__) || defined(__i386) || defined(_M_IX86)  || defined(_X86_)
	#undef  SF_ENV_ARCH_X86
	#define SF_ENV_ARCH_X86 1
	#define SF_ENV_ARCH_NAME "x86"
	#define SF_ENV_ARCH_BITS 32
#elif defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
	#undef  SF_ENV_ARCH_ARM64
	#define SF_ENV_ARCH_ARM64 1
	#define SF_ENV_ARCH_NAME "ARM64"
	#define SF_ENV_ARCH_BITS 64
#elif defined(__arm__) || defined(__arm) || defined(_M_ARM)
	#undef  SF_ENV_ARCH_ARM
	#define SF_ENV_ARCH_ARM 1
	#define SF_ENV_ARCH_NAME "ARM"
	#define SF_ENV_ARCH_BITS 32
#else
	#undef  SF_ENV_ARCH_UNKNOWN
	#define SF_ENV_ARCH_UNKNOWN 1
	#define SF_ENV_ARCH_NAME "Unknown architecture"
	#define SF_ENV_ARCH_BITS 0
#endif


#endif
