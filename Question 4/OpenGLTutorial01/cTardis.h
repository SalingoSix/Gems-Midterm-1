#ifndef _HG_cTardis_
#define _HG_cTardis_

#include <glm/vec3.hpp>
#include <process.h>
#include <Windows.h>	
#include <iostream>
#include "cMazeMaker.h"
#include "cRandThreaded.h"

DWORD WINAPI TardisThread(void* TardisData);

class cTardis
{
public:
	cTardis(glm::vec3 startingPos, glm::vec3 goal, cMazeMaker* theMaze, cRandThreaded* pRandThread);
	~cTardis();
	bool getTardisPosition(glm::vec3 &position);
	bool setTardisPosition(glm::vec3 position);
	bool isDataLocked();
	void beginThread();
	void Update();

	cMazeMaker* theMM;

	bool isCaptured;

private:
	void lockTardisData();
	void unlockTardisData();

	CRITICAL_SECTION CS_DataLock;
	bool CS_isLocked;

	//Info for the Tardis' thread
	HANDLE handle;
	DWORD address;

	glm::vec3 position;
	glm::vec3 endPosition;
	cRandThreaded* pRand;

};


#endif