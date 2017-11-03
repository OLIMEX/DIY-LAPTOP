#ifndef __LINUX_COMPILER_H
#error "Please don't include <linux/compiler-gcc5.h> directly, include <linux/compiler.h> instead."
#endif

/*
 * Force always-inline if the user requests it so via the .config,
 * or if gcc is too old:
 */
#if !defined(CONFIG_ARCH_SUPPORTS_OPTIMIZED_INLINING) || \
    !defined(CONFIG_OPTIMIZE_INLINING) || (__GNUC__ < 4)
// Force gnu89-inline.
#define inline		inline	__attribute__((always_inline)) __attribute__((__gnu_inline__))
#define __inline__	__inline__	__attribute__((always_inline)) __attribute__((__gnu_inline__))
#define __inline	__inline	__attribute__((always_inline)) __attribute__((__gnu_inline__))
#endif
