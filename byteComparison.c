#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "fileManagement.h"

int isInByteArray();
void printAsHex();
int checkFfrWarning();
int checkPaWarning();
int checkSvcWarning();
int checkBsWarning();

int checkBsWarning(unsigned char *data) {
	const unsigned char warnings[][2] = {
	/*(i)*/		{0x41, 0x4E},
	/*(ii)*/	{0x44, 0x52},
	/*(iii)*/	{0x4C, 0x41},
	/*(iv)*/	{0x42, 0x4F},
	/*(v)*/		{0x52, 0x41},
	/*(vi)*/	{0x54, 0x4F},
	/*(vii)*/	{0x83, 0x59},
	/*(viii)*/	{0xA0, 0x1F},
	/*(ix)*/	{0x52, 0x49},
	/*(x)*/		{0x51, 0x00}
	};
	for(int i = 0; i < 10; i++) {
		if(isInByteArray(warnings[i], data, 8) == 1) {
			return 1;
		}
	}
	return 0;
}

int checkSvcWarning(unsigned char *data) {
	const unsigned char warnings[][2] = {
		{0x17, 0x2A},
		{0xD6, 0x93},
		{0xBD, 0xD8},
		{0xFA, 0xEE},
		{0x43, 0x00}
	};
	for(int i = 0; i < 5; i++) {
		if(isInByteArray(warnings[i], data, 16) == 1) {
			return 1;
		}
	}
	return 0;
}

int checkPaWarning(unsigned char *data) {
	const unsigned char warnings[][2] = {
		{0x17, 0x2A},
		{0xD6, 0x93},
		{0xBD, 0xD8},
		{0xFA, 0xEE},
		{0x43, 0x00}
	};
	for(int i = 0; i < 5; i++) {
		if(isInByteArray(warnings[i], data, 4) == 1) {
			return 1;
		}
	}
	return 0;
}

int checkFfrWarning(unsigned char *data) {
	const unsigned char warnings[][2] = {
		{0xA0, 0x0F},
		{0xB0, 0x72},
		{0x2F, 0xA8},
		{0x83, 0x59},
		{0xCE, 0x23}
	};
	for(int i = 0; i < 5; i++) {
		if(isInByteArray(warnings[i], data, 24) == 1) {
			return 1;
		}
	}
	return 0;
}

int isInByteArray(unsigned char *len2toFind, unsigned char *toSearch, int 		toSearchLen) {
	int toFindLen = 2;
	//printf("Lunghezza: sizeof(toSearch) = %ld\n", sizeof(toSearch));
	//printf("Lunghezza: sizeof(toSearch[0]) = %ld\n", sizeof(toSearch[0]));
	//printf("Lunghezza: %d\n", toSearchLen);
	for (int i = 0; i + toFindLen <= toSearchLen; i++) {
		if (len2toFind[0] == toSearch[i] && len2toFind[1] == toSearch[i+1]) {
			return 1;
		}
	}
	return 0;
}
