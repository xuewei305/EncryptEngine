#include "security.h"
#include "log.h"

#define MASK (0xE7)
#ifndef NULL
#define NULL 0
#endif


#define ENC_BUFF_KEY_BIT 0x77
extern INT FlagEncryptForPPT;
char fdata_encrypt(unsigned char * data, unsigned int length)
{
	PCHAR buf;
	UINT64 i; 
	buf = (PCHAR)data;

	for(i=0;i<length;i++)
	{
		/*
		if (FlagEncryptForPPT == 2)
		{
			buf[i] ^= ENC_BUFF_KEY_BIT;
		}*/

		buf[i] ^= ENC_BUFF_KEY_BIT;
	}
	
	return 1;
}

char fdata_decrypt(unsigned char * data, unsigned int length)
{
	
	PCHAR buf;
	unsigned int i;

	buf = (PCHAR)data;

	for(i=0;i<length;i++)
	{
		buf[i] ^= ENC_BUFF_KEY_BIT;
	}
	
	return 1;	
	
}