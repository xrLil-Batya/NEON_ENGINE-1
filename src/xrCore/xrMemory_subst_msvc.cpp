#include "stdafx.h"
#pragma hdrstop

#include "xrMemory_align.h"
#include "xrMemory_pure.h"

const		u32			mem_pools_count = 64;
const		u32			mem_pools_ebase = 32;
const		u32			mem_generic = mem_pools_count + 1;
const		u32			mem_pools_resize = 4096;

#define STD_ALLOCK

void xrMemory::InitializeMemoryPools()
{
#ifndef STD_ALLOCK
	// initialize POOLs
	u32	element = mem_pools_ebase;
	u32 sector = mem_pools_ebase * 4096;
	for (u32 pid = 0; pid < mem_pools_count; pid++)
	{
		mem_pools[pid]._initialize(element, sector, 0x1);
		element += mem_pools_ebase;
	}
#endif

	mem_initialized = TRUE;
}

#	define	debug_mode 0

MEMPOOL		mem_pools			[mem_pools_count];



// MSVC
ICF	u8*		acc_header			(void* P)	{	u8*		_P		= (u8*)P;	return	_P-1;	}
ICF	u32		get_header			(void* P)	{	return	(u32)*acc_header(P);				}
ICF	u32		get_pool			(size_t size)
{
	u32		pid					= u32(size/mem_pools_ebase);
	if (pid>=mem_pools_count)	return mem_generic;
	else						return pid;
}

#define PURE_MEMORY_ALIGNMENT 1 << 4

void*	xrMemory::mem_alloc		(size_t size)
{
	stat_calls++;

#ifdef STD_ALLOCK
	void* result = _aligned_malloc(size, PURE_MEMORY_ALIGNMENT);
	if (result)
		memset(result, 0, size);     // FILL ZERO
	return (result);
#else 


	u32		_footer				=	debug_mode?4:0;
	void*	_ptr				=	0;

	//
	if (!mem_initialized)		
	{
		void*	_real			=	xr_aligned_offset_malloc	(1 + size + _footer, 16, 0x1);
		_ptr					=	(void*)(((u8*)_real)+1);
		*acc_header(_ptr)		=	mem_generic;
	} 
	else
	{
		u32	pool				=	get_pool	(1+size+_footer);
		if (mem_generic==pool)	
		{
			void*	_real		=	xr_aligned_offset_malloc	(1 + size + _footer,16,0x1);
			_ptr				=	(void*)(((u8*)_real)+1);
			*acc_header(_ptr)	=	mem_generic;
		} 
		else
		{
			void*	_real		=	mem_pools[pool].create();
			_ptr				=	(void*)(((u8*)_real)+1);
			*acc_header(_ptr)	=	(u8)pool;
		}
	}

	return	_ptr;
#endif

}

void	xrMemory::mem_free		(void* P)
{
#ifdef STD_ALLOCK
	_aligned_free(P);
#else
	stat_calls++;
 	u32	pool					= get_header	(P);
	void* _real					= (void*)(((u8*)P)-1);
	
	if (mem_generic==pool)		
  		xr_aligned_free			(_real);
 	else
   		mem_pools[pool].destroy	(_real);
#endif

}

extern BOOL	g_bDbgFillMemory	;

void*	xrMemory::mem_realloc	(void* P, size_t size )
{
	stat_calls++;
	if (0 == P)
		return mem_alloc(size);

#ifdef STD_ALLOCK
	size_t old_size = P ? _aligned_msize(P, PURE_MEMORY_ALIGNMENT, 0) : 0;
	void* result = _aligned_realloc(P, size, PURE_MEMORY_ALIGNMENT);
	if (result && size > old_size)
		memset((u8*)result + old_size, 0, size - old_size);  // FILL ZERO
	return (result);

#else



	u32		p_current			= get_header(P);
	u32		p_new				= get_pool	(1+size+(debug_mode?4:0));
	u32		p_mode				;

	if (mem_generic==p_current)	
	{
		if (p_new<p_current)	
			p_mode	= 2	;
		else					
			p_mode	= 0	;
	} 
	else 						
		p_mode	= 1	;

	void*	_real				= (void*)(((u8*)P)-1);
	void*	_ptr				= NULL;
	if		(0==p_mode)
	{
		u32		_footer			=	debug_mode?4:0;
	
		void*	_real2			=	xr_aligned_offset_realloc	(_real,1+size+_footer,16,0x1);
		_ptr					= (void*)(((u8*)_real2)+1);
		*acc_header(_ptr)		= mem_generic;
	}
	else if (1==p_mode)	
	{
		// pooled realloc
		R_ASSERT2				(p_current<mem_pools_count,"Memory corruption");
		u32		s_current		= mem_pools[p_current].get_element();
		u32		s_dest			= (u32)size;
		void*	p_old			= P;
		// new 
		void* p_new = mem_alloc(size);
		mem_copy				(p_new,p_old,_min(s_current-1,s_dest));
 		mem_free				(p_old);
		_ptr					= p_new;
	} 
	else if (2==p_mode)	
	{
		// relocate into another mmgr(pooled) from real
		void*	p_old			= P;
		void*	p_new			= mem_alloc(size);
		mem_copy				(p_new,p_old,(u32)size);
		mem_free				(p_old);
		_ptr					= p_new;
	}
  
	return	_ptr;
#endif
}
 