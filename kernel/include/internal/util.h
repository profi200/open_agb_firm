#pragma once


#define LIKELY(expr)   __builtin_expect((expr), true)
#define UNLIKELY(expr) __builtin_expect((expr), false)
