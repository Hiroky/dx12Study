#pragma once

#include <Windows.h>

//Œ^
typedef unsigned char				u8;
typedef signed char					s8;
typedef unsigned long long int		u64;
typedef unsigned int				u32;
typedef unsigned int				uint;
typedef int							s32;
typedef unsigned short				u16;
typedef short						s16;
typedef float						f32;
typedef double						f64;



#ifndef NDEBUG

#define slAssert(exp)			assert(exp)
#define slAssertMsg(exp, ...)	assert(exp)

#else

#define slAssert(exp)	

#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) if (x != nullptr) { x->Release(); x = nullptr; }
#endif

#ifndef THROW_IF_FAILED
#define THROW_IF_FAILED(hr) if (FAILED(hr)) { throw; }
#endif

