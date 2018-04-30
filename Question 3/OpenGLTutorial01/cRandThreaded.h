#ifndef _HG_cRandThreaded_
#define _HG_cRandThreaded_

#include <Windows.h>

template <class T>
T getRandInRange(T min, T max);

//template <class T>
//T getRandInRange(T min, T max, T randValueFrom0to1);

class cRandThreaded
{
public:
	cRandThreaded();
	~cRandThreaded();
	double getNextRandDouble(void);

	static const int SIZEOFBUFFER = 100;
	double randDoubles[SIZEOFBUFFER];
	int lastReadIndex;	// = 0;
	CRITICAL_SECTION CS_bufferIndexLock;
private:
	void m_LoadBufferWithRandoms(void);
};

#endif