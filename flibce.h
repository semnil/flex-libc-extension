#ifndef _FLIBCE_H_
#define	_FLIBCE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define	fle_free(a)			if ((a)!=NULL) {free((a));(a)=NULL;}

#ifdef __cplusplus
extern "C" {
#endif

int createRingBuffer(int size);
void removeRingBuffer(int ring);
void removeAllRingBuffer();
int countOfRingBuffer(int ring);
void clearRingBuffer(int ring);
int writeToRingBuffer(int ring, char *data, int len);
int readFromRingBuffer(int ring, char *data, int len);
char watchFromRingBuffer(int ring, int offset);

#ifdef __cplusplus
}
#endif

#endif
