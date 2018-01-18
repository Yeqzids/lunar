/* Code to read integers of various sizes and 64-bit double-precision
floats from binary data buffers.  See uses in 'vsopson.cpp' and 'lunar2.cpp'
where data is extracted from binary files.

   It's assumed that the input data is in Intel (little-Endian) order.

   On Intel machines,  you can extract such data by type-punning,
even if the data isn't aligned on a word boundary.  On some other
machines,  the data must be aligned on a word boundary and/or you
may have reverse the byte order.

   OpenWATCOM,  Microsoft C,  Borland,  TurboC,  and Digital Mars are all
Intel-only.  If we're using one of those compilers,  we can type-pun.
gcc and clang #define __x86_64__ or __i386__ for Intel hardware.

   There are probably other cases where we could type-pun.  But the
non-punning code will work in all cases;  all we lose by not punning
is a very small amount of speed.  If I learn of other cases where
punning is possible,  the conditions under which USE_TYPE_PUNNING
is defined can be modified.      */

#if defined(__x86_64__) || defined(__i386__) || defined(__WATCOMC__) \
                      || defined(_MSC_VER) || defined (__BORLANDC__) \
                      || defined (__TURBOC__) || defined( __DMC__)
   #define USE_TYPE_PUNNING
#endif

#if defined( __WORD_ORDER__) && (__WORD_ORDER__ == __ORDER_BIG_ENDIAN__)
   #define USING_BIG_ENDIAN
#endif

#undef USE_TYPE_PUNNING

#ifdef USE_TYPE_PUNNING
   #define get16bits(d)  (*((const uint16_t *) (d)))
   #define get32bits(d)  (*((const uint32_t *) (d)))
   #define get64bits(d)  (*((const uint64_t *) (d)))
   #define get_double(d) (*((const double *) (d)))
#else             /* Can't directly read binary data */
   #define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
   #define get32bits(d) ((((uint32_t)(((const uint8_t *)(d))[3])) << 24)\
                        +(((uint32_t)(((const uint8_t *)(d))[2])) << 16)\
                        +(((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )

static inline double get_double( const void *iptr)
{
   const char *idata = (const char *)iptr;
   int i;
   double rval;
   char *buff = (char *)&rval;

#if defined( USING_BIG_ENDIAN)
   for( i = 7; i >= 0; i--)
#else
   for( i = 0; i < 8; i++)
#endif
      buff[i] = *idata++;
   return( rval);
}
#endif            /* End of code to indirectly read binary data */
