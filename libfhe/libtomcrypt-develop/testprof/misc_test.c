#include  <tomcrypt_test.h>

int misc_test(void)
{
#ifdef LTC_HKDF
   DO(hkdf_test());
#endif
#ifdef LTC_PKCS_5
   DO(pkcs_5_test());
#endif
#ifdef LTC_BASE64
   DO(base64_test());
#endif
#ifdef LTC_ADLER32
   DO(adler32_test());
#endif
#ifdef LTC_CRC32
   DO(crc32_test());
#endif
   return 0;
}

/* $Source$ */
/* $Revision$ */
/* $Date$ */
