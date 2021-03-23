#ifndef _FILENAME_H_
#define _FILENMAE_H_




#ifndef MAX_PATH
#define MAX_PATH 260 
#endif 

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

#ifndef SECTOR_SIZE
#define SECTOR_SIZE 512
#endif

#ifndef DBG
#define DBG 1
#endif

#define FILEFLAG_POOL_TAG 'FASV'
#define FILE_FLAG_LENGTH  sizeof(FILE_FLAG)
#define FILE_GUID_LENGTH  16
#define HASH_SIZE 20

#pragma pack(1)

/**
 * File Flag Structure
 * is written into end of file every time when file is closed. 
 */
typedef struct _FILE_FLAG{

	/**
	 * This field holds GUID, GUID is used to distinguish encrypted 
	 * files and un-encrypted files. All encrypted files share 
	 * the same GUID that described in file.c
	 */
	UCHAR    FileFlagHeader[FILE_GUID_LENGTH] ;  
	
	/**
	 * This field holds sha1 digest of key for specified file,
	 * Each file has its own encrypt/decrypt key.
	 */
	UCHAR    FileKeyHash[HASH_SIZE] ;

	/**
	 * This field holds real size of file, so engine 
	 * can return/set right file size for user's query/set
	 * request.
	 */
	LONGLONG FileValidLength ;

	/**
	 * For further usage and sector size alignment.
	 */
	UCHAR    Reserved[SECTOR_SIZE-HASH_SIZE-FILE_GUID_LENGTH-8] ;

}FILE_FLAG,*PFILE_FLAG;

/**
 * declared global file flag infomation,
 * it is used to compare with file flag of open file
 * to judge whether specified file are encrypted or
 * not.
 */
extern PFILE_FLAG g_psFileFlag ;

#pragma pack()


#endif