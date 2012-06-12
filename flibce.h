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
int countByteOfRingBuffer(int ring);
void clearRingBuffer(int ring);
int writeBytesToRingBuffer(int ring, unsigned char *data, int len);
int readBytesFromRingBuffer(int ring, unsigned char *data, int len);
unsigned char watchByteFromRingBuffer(int ring, int offset);

int countBitOfRingBuffer(int ring);
int writeBitsToRingBuffer(int ring, unsigned char *data, int len);
int readBitsFromRingBuffer(int ring, unsigned char *data, int len);
unsigned char watchBitsFromRingBuffer(int ring, int offset);

#ifdef __cplusplus
}
#endif

#endif
