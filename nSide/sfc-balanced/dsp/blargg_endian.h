// CPU Byte Order Utilities

// snes_spc 0.9.0
#ifndef BLARGG_ENDIAN
#define BLARGG_ENDIAN

#include "blargg_common.h"

// BLARGG_CPU_CISC: Defined if CPU has very few general-purpose registers (< 16)
#if defined (_M_IX86) || defined (_M_IA64) || defined (__i486__) || \
		defined (__x86_64__) || defined (__ia64__) || defined (__i386__)
	#define BLARGG_CPU_X86 1
	#define BLARGG_CPU_CISC 1
#endif

#if defined (__powerpc__) || defined (__ppc__) || defined (__POWERPC__) || defined (__powerc)
	#define BLARGG_CPU_POWERPC 1
	#define BLARGG_CPU_RISC 1
#endif

inline void blargg_verify_byte_order()
{
	#ifndef NDEBUG
		#if defined(ENDIAN_MSB)
			volatile int i = 1;
			assert( *(volatile char*) &i == 0 );
		#elif defined(ENDIAN_LSB)
			volatile int i = 1;
			assert( *(volatile char*) &i != 0 );
		#endif
	#endif
}

inline unsigned get_le16( void const* p )
{
	return  (unsigned) ((unsigned char const*) p) [1] << 8 |
			(unsigned) ((unsigned char const*) p) [0];
}

inline unsigned get_be16( void const* p )
{
	return  (unsigned) ((unsigned char const*) p) [0] << 8 |
			(unsigned) ((unsigned char const*) p) [1];
}

inline blargg_ulong get_le32( void const* p )
{
	return  (blargg_ulong) ((unsigned char const*) p) [3] << 24 |
			(blargg_ulong) ((unsigned char const*) p) [2] << 16 |
			(blargg_ulong) ((unsigned char const*) p) [1] <<  8 |
			(blargg_ulong) ((unsigned char const*) p) [0];
}

inline blargg_ulong get_be32( void const* p )
{
	return  (blargg_ulong) ((unsigned char const*) p) [0] << 24 |
			(blargg_ulong) ((unsigned char const*) p) [1] << 16 |
			(blargg_ulong) ((unsigned char const*) p) [2] <<  8 |
			(blargg_ulong) ((unsigned char const*) p) [3];
}

inline void set_le16( void* p, unsigned n )
{
	((unsigned char*) p) [1] = (unsigned char) (n >> 8);
	((unsigned char*) p) [0] = (unsigned char) n;
}

inline void set_be16( void* p, unsigned n )
{
	((unsigned char*) p) [0] = (unsigned char) (n >> 8);
	((unsigned char*) p) [1] = (unsigned char) n;
}

inline void set_le32( void* p, blargg_ulong n )
{
	((unsigned char*) p) [0] = (unsigned char) n;
	((unsigned char*) p) [1] = (unsigned char) (n >> 8);
	((unsigned char*) p) [2] = (unsigned char) (n >> 16);
	((unsigned char*) p) [3] = (unsigned char) (n >> 24);
}

inline void set_be32( void* p, blargg_ulong n )
{
	((unsigned char*) p) [3] = (unsigned char) n;
	((unsigned char*) p) [2] = (unsigned char) (n >> 8);
	((unsigned char*) p) [1] = (unsigned char) (n >> 16);
	((unsigned char*) p) [0] = (unsigned char) (n >> 24);
}

#if BLARGG_NONPORTABLE
	// Optimized implementation if byte order is known
	#if defined(ENDIAN_LSB)
		#define GET_LE16( addr )        (*(BOOST::uint16_t*) (addr))
		#define GET_LE32( addr )        (*(BOOST::uint32_t*) (addr))
		#define SET_LE16( addr, data )  (void) (*(BOOST::uint16_t*) (addr) = (data))
		#define SET_LE32( addr, data )  (void) (*(BOOST::uint32_t*) (addr) = (data))
	#elif defined(ENDIAN_MSB)
		#define GET_BE16( addr )        (*(BOOST::uint16_t*) (addr))
		#define GET_BE32( addr )        (*(BOOST::uint32_t*) (addr))
		#define SET_BE16( addr, data )  (void) (*(BOOST::uint16_t*) (addr) = (data))
		#define SET_BE32( addr, data )  (void) (*(BOOST::uint32_t*) (addr) = (data))
		
		#if BLARGG_CPU_POWERPC
			// PowerPC has special byte-reversed instructions
			#if defined (__MWERKS__)
				#define GET_LE16( addr )        (__lhbrx( addr, 0 ))
				#define GET_LE32( addr )        (__lwbrx( addr, 0 ))
				#define SET_LE16( addr, in )    (__sthbrx( in, addr, 0 ))
				#define SET_LE32( addr, in )    (__stwbrx( in, addr, 0 ))
			#elif defined (__GNUC__)
				#define GET_LE16( addr )        ({unsigned ppc_lhbrx_; asm( "lhbrx %0,0,%1" : "=r" (ppc_lhbrx_) : "r" (addr), "0" (ppc_lhbrx_) ); ppc_lhbrx_;})
				#define GET_LE32( addr )        ({unsigned ppc_lwbrx_; asm( "lwbrx %0,0,%1" : "=r" (ppc_lwbrx_) : "r" (addr), "0" (ppc_lwbrx_) ); ppc_lwbrx_;})
				#define SET_LE16( addr, in )    ({asm( "sthbrx %0,0,%1" : : "r" (in), "r" (addr) );})
				#define SET_LE32( addr, in )    ({asm( "stwbrx %0,0,%1" : : "r" (in), "r" (addr) );})
			#endif
		#endif
	#endif
#endif

#ifndef GET_LE16
	#define GET_LE16( addr )        get_le16( addr )
	#define SET_LE16( addr, data )  set_le16( addr, data )
#endif

#ifndef GET_LE32
	#define GET_LE32( addr )        get_le32( addr )
	#define SET_LE32( addr, data )  set_le32( addr, data )
#endif

#ifndef GET_BE16
	#define GET_BE16( addr )        get_be16( addr )
	#define SET_BE16( addr, data )  set_be16( addr, data )
#endif

#ifndef GET_BE32
	#define GET_BE32( addr )        get_be32( addr )
	#define SET_BE32( addr, data )  set_be32( addr, data )
#endif

// auto-selecting versions

inline void set_le( BOOST::uint16_t* p, unsigned     n ) { SET_LE16( p, n ); }
inline void set_le( BOOST::uint32_t* p, blargg_ulong n ) { SET_LE32( p, n ); }
inline void set_be( BOOST::uint16_t* p, unsigned     n ) { SET_BE16( p, n ); }
inline void set_be( BOOST::uint32_t* p, blargg_ulong n ) { SET_BE32( p, n ); }
inline unsigned     get_le( BOOST::uint16_t* p ) { return GET_LE16( p ); }
inline blargg_ulong get_le( BOOST::uint32_t* p ) { return GET_LE32( p ); }
inline unsigned     get_be( BOOST::uint16_t* p ) { return GET_BE16( p ); }
inline blargg_ulong get_be( BOOST::uint32_t* p ) { return GET_BE32( p ); }

#endif
