#include "flibce.h"

int g_nRingBuffer = 0;
typedef struct {
	char	*ptr;
	int		nSize;
	int		nWrite;
	int		nRead;
	int		nCount;
} RING_BUFFER;
RING_BUFFER	*g_pRingBuffers = NULL;

int createRingBuffer(int size)
{
	if (!g_pRingBuffers) {
		// new
		g_pRingBuffers = (RING_BUFFER *)calloc(sizeof(RING_BUFFER), sizeof(char));
		g_pRingBuffers[0].ptr = (char *)calloc(size, sizeof(char));
		g_pRingBuffers[0].nSize = size;
		g_pRingBuffers[0].nWrite = 0;
		g_pRingBuffers[0].nRead = 0;
		g_pRingBuffers[0].nCount = 0;
		g_nRingBuffer = 0;
	} else {
		// add
		g_pRingBuffers = (RING_BUFFER *)realloc(g_pRingBuffers, sizeof(RING_BUFFER) * g_nRingBuffer);
		g_pRingBuffers[g_nRingBuffer].ptr = (char *)calloc(size, sizeof(char));
		g_pRingBuffers[g_nRingBuffer].nSize = size;
		g_pRingBuffers[g_nRingBuffer].nWrite = 0;
		g_pRingBuffers[g_nRingBuffer].nRead = 0;
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

int countOfRingBuffer(int ring)
{
	if (ring < 0 || ring >= g_nRingBuffer)
		return -1;
	else
		return g_pRingBuffers[ring].nCount;
}

void clearRingBuffer(int ring)
{
	if (ring >= 0 && ring < g_nRingBuffer) {
		g_pRingBuffers[ring].nWrite = 0;
		g_pRingBuffers[ring].nRead = 0;
		g_pRingBuffers[ring].nCount = 0;
	}
}

int writeToRingBuffer(int ring, char *data, int len)
{
	char *ptr = g_pRingBuffers[ring].ptr;
	int nWrite = g_pRingBuffers[ring].nWrite;
	int nSize = g_pRingBuffers[ring].nSize;
	int rtc = indexOfRingBuffer(nWrite, len, nSize);

	if (ring < 0 || ring >= g_nRingBuffer)
		return -1;

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

int readFromRingBuffer(int ring, char *data, int len)
{
	char *ptr = g_pRingBuffers[ring].ptr;
	int nRead = g_pRingBuffers[ring].nRead;
	int nSize = g_pRingBuffers[ring].nSize;
	int rtc = indexOfRingBuffer(nRead, len, nSize);

	if (ring < 0 || ring >= g_nRingBuffer)
		return -1;

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

char watchFromRingBuffer(int ring, int offset)
{
	char *ptr = g_pRingBuffers[ring].ptr;
	int nRead = g_pRingBuffers[ring].nRead;
	int nSize = g_pRingBuffers[ring].nSize;
	int rtc = indexOfRingBuffer(nRead, offset, nSize);

	if (ring < 0 || ring >= g_nRingBuffer)
		return -1;

	return g_pRingBuffers[ring].ptr[rtc];
}
