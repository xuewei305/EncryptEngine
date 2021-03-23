#ifndef _SECURITY_H
#define _SECURITY_H
char fdata_encrypt(unsigned char * data, unsigned int length);
char fdata_decrypt(unsigned char * data, unsigned int length);
#define data_encrypt(a,b)  DbgPrint ("Line:%4d,%s\n", __LINE__, __FILE__),fdata_encrypt(a,b)

#define data_decrypt(a,b)  DbgPrint ("Line:%4d,%s\n", __LINE__, __FILE__), fdata_decrypt(a,b)

#endif