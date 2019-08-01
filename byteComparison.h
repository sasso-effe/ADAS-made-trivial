#ifndef _BYTE_COMPARISON_
	#define _BYTE_COMPARISON_
	extern int isInByteArray(unsigned char *len2toFind, unsigned char *toSearch, int toSearchLen);
	extern int checkFfrWarning(unsigned char *data);
	extern int checkPaWarning(unsigned char *data);
	extern int checkSvcWarning(unsigned char *data);
	extern int checkBsWarning(unsigned char *data);
#endif
