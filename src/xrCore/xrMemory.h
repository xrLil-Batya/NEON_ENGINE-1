#ifndef xrMemoryH
#define xrMemoryH
#pragma once

#include "memory_monitor.h" 

#include "xrMemory_pso.h"
#include "xrMemory_POOL.h"

class XRCORE_API		xrMemory
{
public:
	struct				mdbg {
		void*			_p;
		size_t 			_size;
		const char*		_name;
		u32				_dummy;
	};
public:
	xrMemory			();
	void				_initialize		(BOOL _debug_mode=FALSE);
	void				_destroy		();
 
	u32					stat_calls;
	s32					stat_counter;
public:
	 
	u32					mem_usage		(u32* pBlocksUsed=NULL, u32* pBlocksFree=NULL);
	void				mem_compact		();
	void				mem_counter_set	(u32 _val)	{ stat_counter = _val;	}
	u32					mem_counter_get	()			{ return stat_counter;	}

 
	void				InitializeMemoryPools();
	void*				mem_alloc		(size_t	size				);
	void*				mem_realloc		(void*	p, size_t size		);
	void				mem_free		(void*	p					);

	pso_MemCopy*		mem_copy;
	pso_MemFill*		mem_fill;
	pso_MemFill32*		mem_fill32;
};

extern XRCORE_API	xrMemory	Memory;

#undef	ZeroMemory
#undef	CopyMemory
#undef	FillMemory
#define ZeroMemory(a,b)		Memory.mem_fill(a,0,b)
#define CopyMemory(a,b,c)	memcpy(a,b,c)			//. CopyMemory(a,b,c)
#define FillMemory(a,b,c)	Memory.mem_fill(a,c,b)
 
#include "xrMemory_subst_msvc.h"
 
  
template <class T>
IC T*		xr_alloc	(u32 count)				{	return  (T*)Memory.mem_alloc(count*sizeof(T));	}
template <class T>
IC void		xr_free		(T* &P)					{	if (P) { Memory.mem_free((void*)P); P=NULL;	};	}
IC void*	xr_malloc	(size_t size)			{	return	Memory.mem_alloc(size);					}
IC void*	xr_realloc	(void* P, size_t size)	{	return Memory.mem_realloc(P,size);				}
 

XRCORE_API	char* 	xr_strdup	(const char* string);
 
IC void*	operator new		(size_t size)		{	return Memory.mem_alloc(size?size:1);				}
IC void		operator delete		(void *p)			{	xr_free(p);											}
IC void*	operator new[]		(size_t size)		{	return Memory.mem_alloc(size?size:1);				}
IC void		operator delete[]	(void* p)			{	xr_free(p);											}
 

// POOL-ing
 
extern		BOOL		mem_initialized;

XRCORE_API void vminfo			(size_t *_free, size_t *reserved, size_t *committed);
XRCORE_API void log_vminfo		();
XRCORE_API u32	mem_usage_impl	(HANDLE heap_handle, u32* pBlocksUsed, u32* pBlocksFree);

#endif // xrMemoryH