#include "flibce.h"

int g_nRingBuffer = 0;
typedef struct {
	unsigned char	*ptr;
	int				nSize;
	int				nWrite;
	int				nWriteBit;
	int				nRead;
	int				nReadBit;
	int				nCount;
} RING_BUFFER;
RING_BUFFER	*g_pRingBuffers = NULL;

int createRingBuffer(int size)
{
	if (!g_pRingBuffers) {
		// new
		g_pRingBuffers = (RING_BUFFER *)calloc(sizeof(RING_BUFFER), sizeof(char));
		g_pRingBuffers[0].ptr = (unsigned char *)calloc(size, sizeof(unsigned char));
		g_pRingBuffers[0].nSize = size;
		g_pRingBuffers[0].nWrite = 0;
		g_pRingBuffers[0].nWriteBit = 0;
		g_pRingBuffers[0].nRead = 0;
		g_pRingBuffers[0].nReadBit = 0;
		g_pRingBuffers[0].nCount = 0;
		g_nRingBuffer = 0;
	} else {
		// add
		g_pRingBuffers = (RING_BUFFER *)realloc(g_pRingBuffers, sizeof(RING_BUFFER) * (g_nRingBuffer + 1));
		g_pRingBuffers[g_nRingBuffer].ptr = (unsigned char *)calloc(size, sizeof(unsigned char));
		g_pRingBuffers[g_nRingBuffer].nSize = size;
		g_pRingBuffers[g_nRingBuffer].nWrite = 0;
		g_pRingBuffers[g_nRingBuffer].nWriteBit = 0;
		g_pRingBuffers[g_nRingBuffer].nRead = 0;
		g_pRingBuffers[g_nRingBuffer].nReadBit = 0;
		g_pRingBuffers[g_nRingBuffer].nCount = 0;
	}
	return g_nRingBuffer++;
}

void removeRingBuffer(int ring)
{
	int i;
	if (ring < 0 || ring >= g_nRingBuffer)
		return;
	if (!g_pRingBuffers || !g_nRingBuffer)
		return;
	fle_free(g_pRingBuffers[ring].ptr);
	for (i = 0;i < g_nRingBuffer;i++) {
		if (g_pRingBuffers[i].ptr)
			break;
	}
	if (i == g_nRingBuffer) {
		fle_free(g_pRingBuffers);
		g_nRingBuffer = 0;
	}
}

void removeAllRingBuffer()
{
	int i;
	for (i = 0;i < g_nRingBuffer;i++) {
		fle_free(g_pRingBuffers[i].ptr);
	}
	fle_free(g_pRingBuffers);
	g_nRingBuffer = 0;
}

int indexOfRingBuffer(int base, int offset, int size)
{
	if (size <= 0)
		return 0;
	else
		return ((base + offset < size)?(base + offset):(indexOfRingBuffer(base - size, offset, size)));
}

int countByteOfRingBuffer(int ring)
{
	if (ring < 0 || ring >= g_nRingBuffer)
		return -1;
	else
		return g_pRingBuffers[ring].nCount + ((g_pRingBuffers[ring].nWriteBit > 0)?1:0);
}

void clearRingBuffer(int ring)
{
	if (ring >= 0 && ring < g_nRingBuffer) {
		g_pRingBuffers[ring].nWrite = 0;
		g_pRingBuffers[ring].nWriteBit = 0;
		g_pRingBuffers[ring].nRead = 0;
		g_pRingBuffers[ring].nReadBit = 0;
		g_pRingBuffers[ring].nCount = 0;
	}
}

int writeBytesToRingBuffer(int ring, unsigned char *data, int len)
{
	unsigned char *ptr;
	int nWrite;
	int nSize;
	int rtc;

	if (ring < 0 || ring >= g_nRingBuffer)
		return -1;

	ptr = g_pRingBuffers[ring].ptr;
	nWrite = g_pRingBuffers[ring].nWrite;
	nSize = g_pRingBuffers[ring].nSize;
	rtc = indexOfRingBuffer(nWrite, len, nSize);

	if (nWrite <= rtc) {
		memcpy(&ptr[nWrite], data, len);
	} else {
		memcpy(&ptr[nWrite], data, nSize - nWrite);
		memcpy(ptr, &data[nSize - nWrite], len - (nSize - nWrite));
	}
	g_pRingBuffers[ring].nWrite = rtc;
	g_pRingBuffers[ring].nCount += len;

	if (g_pRingBuffers[ring].nCount <= nSize)
		return 0;
	else {
		clearRingBuffer(ring);
		return -1;
	}
}

int readBytesFromRingBuffer(int ring, unsigned char *data, int len)
{
	unsigned char *ptr;
	int nRead;
	int nSize;
	int rtc;

	if (ring < 0 || ring >= g_nRingBuffer)
		return -1;

	ptr = g_pRingBuffers[ring].ptr;
	nRead = g_pRingBuffers[ring].nRead;
	nSize = g_pRingBuffers[ring].nSize;
	rtc = indexOfRingBuffer(nRead, len, nSize);

	if (g_pRingBuffers[ring].nCount < len)
		len = g_pRingBuffers[ring].nCount;
	if (nRead < rtc) {
		memcpy(data, &ptr[nRead], len);
	} else {
		memcpy(data, &ptr[nRead], nSize - nRead);
		memcpy(&data[nSize - nRead], ptr, len - (nSize - nRead));
	}
	g_pRingBuffers[ring].nRead = rtc;
	g_pRingBuffers[ring].nCount -= len;

	return len;
}

unsigned char watchByteFromRingBuffer(int ring, int offset)
{
	unsigned char *ptr;
	int nRead;
	int nSize;
	int rtc;

	if (ring < 0 || ring >= g_nRingBuffer)
		return -1;

	ptr = g_pRingBuffers[ring].ptr;
	nRead = g_pRingBuffers[ring].nRead;
	nSize = g_pRingBuffers[ring].nSize;
	rtc = indexOfRingBuffer(nRead, offset, nSize);

	return g_pRingBuffers[ring].ptr[rtc];
}

int countBitOfRingBuffer(int ring)
{
	if (ring < 0 || ring >= g_nRingBuffer)
		return -1;
	else
		return g_pRingBuffers[ring].nCount * 8 + g_pRingBuffers[ring].nWriteBit;
}

int writeBitsToRingBuffer(int ring, unsigned char *data, int len)
{
	unsigned char *ptr;
	int nWrite;
	int nSize;
	int nLen;
	int	nBit;
	int index0, index1;
	int i;
	unsigned char mask = 0xff;

	if (ring < 0 || ring >= g_nRingBuffer)
		return -1;

	ptr = g_pRingBuffers[ring].ptr;
	index1 = nWrite = g_pRingBuffers[ring].nWrite;
	nSize = g_pRingBuffers[ring].nSize;
	nLen = len / 8;
	nBit = len % 8;
	if (g_pRingBuffers[ring].nWriteBit + nBit >= 8) {
		nLen++;
		nBit = g_pRingBuffers[ring].nWriteBit + nBit - 8;
	} else
		nBit = g_pRingBuffers[ring].nWriteBit + nBit;

	for (i = 0;i < nLen;i++) {
		index0 = indexOfRingBuffer(nWrite, i, nSize);
		index1 = indexOfRingBuffer(nWrite, i + 1, nSize);
		ptr[index0] = (ptr[index0] & (mask << g_pRingBuffers[ring].nWriteBit)) | (data[i] >> g_pRingBuffers[ring].nWriteBit);
		ptr[index1] = data[i] << (8 - g_pRingBuffers[ring].nWriteBit);
	}

	g_pRingBuffers[ring].nWrite = index1;
	g_pRingBuffers[ring].nWriteBit = nBit;
	g_pRingBuffers[ring].nCount += nLen;

	if (g_pRingBuffers[ring].nCount < nSize || g_pRingBuffers[ring].nCount == nSize && g_pRingBuffers[ring].nWriteBit == 0)
		return 0;
	else {
		clearRingBuffer(ring);
		return -1;
	}
}

int readBitsFromRingBuffer(int ring, unsigned char *data, int len)
{
	unsigned char *ptr;
	int nRead;
	int nSize;
	int nLen;
	int	nBit;
	int index0, index1;
	int i;

	if (ring < 0 || ring >= g_nRingBuffer)
		return -1;

	ptr = g_pRingBuffers[ring].ptr;
	index1 = nRead = g_pRingBuffers[ring].nRead;
	nSize = g_pRingBuffers[ring].nSize;
	nLen = len / 8;
	nBit = len % 8;
	if (g_pRingBuffers[ring].nReadBit + nBit >= 8) {
		nLen++;
		nBit = g_pRingBuffers[ring].nReadBit + nBit - 8;
	} else
		nBit = g_pRingBuffers[ring].nReadBit + nBit;

	if (g_pRingBuffers[ring].nCount < nLen) {
		nLen = g_pRingBuffers[ring].nCount;
		nBit = g_pRingBuffers[ring].nReadBit;
	} else if (g_pRingBuffers[ring].nCount == nLen && g_pRingBuffers[ring].nReadBit < nBit)
		nBit = g_pRingBuffers[ring].nReadBit;

	for (i = 0;i < nLen;i++) {
		index0 = indexOfRingBuffer(nRead, i, nSize);
		index1 = indexOfRingBuffer(nRead, i + 1, nSize);
		data[i] = (unsigned char)((ptr[index0] << g_pRingBuffers[ring].nReadBit) | (ptr[index1] >> (8 - g_pRingBuffers[ring].nReadBit)));
	}

	g_pRingBuffers[ring].nRead = index1;
	g_pRingBuffers[ring].nReadBit = nBit;
	g_pRingBuffers[ring].nCount -= nLen;

	return len;
}

unsigned char watchBitsFromRingBuffer(int ring, int offset)
{
	unsigned char *ptr;
	int nRead;
	int nSize;
	int nOffset;
	int	nBit;
	int index0, index1;

	if (ring < 0 || ring >= g_nRingBuffer)
		return -1;

	ptr = g_pRingBuffers[ring].ptr;
	nRead = g_pRingBuffers[ring].nRead;
	nSize = g_pRingBuffers[ring].nSize;
	nOffset = offset / 8;
	nBit = offset % 8;
	if (g_pRingBuffers[ring].nReadBit + nBit >= 8) {
		nOffset++;
		nBit = g_pRingBuffers[ring].nReadBit + nBit - 8;
	} else
		nBit = g_pRingBuffers[ring].nReadBit + nBit;

	index0 = indexOfRingBuffer(nRead, nOffset, nSize);
	if (nBit) {
		index1 = indexOfRingBuffer(nRead, nOffset + 1, nSize);
		return (unsigned char)((g_pRingBuffers[ring].ptr[index0] << nBit) + (g_pRingBuffers[ring].ptr[index1] >> (8 - nBit)));
	} else
		return g_pRingBuffers[ring].ptr[index0];
}
