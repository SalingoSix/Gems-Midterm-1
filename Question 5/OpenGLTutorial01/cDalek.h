#ifndef _HG_cDalek_
#define _HG_cDalek_

#include <glm/vec3.hpp>
#include <process.h>
#include <Windows.h>	
#include <iostream>
#include "cMazeMaker.h"
#include "cRandThreaded.h"
#include "cTardis.h"

DWORD WINAPI DalekThread(void* DalekData);

class cDalek
{
public:
	cDalek(glm::vec3 startingPos, cTardis* theDoctor, cMazeMaker* theMaze, cRandThreaded* pRandThread);
	~cDalek();

	bool getDalekPosition(glm::vec3 &position);
	bool setDalekPosition(glm::vec3 position);
	bool getFoundDoctor(bool &found);
	bool isDataLocked();
	void beginThread();
	void Update();

	cMazeMaker* theMM;

	cTardis* theTardis;

	bool foundDoctor;

private:
	void lockDalekData();
	void unlockDalekData();

	bool setFoundDoctor(bool found);

	CRITICAL_SECTION CS_DataLock;
	bool CS_isLocked;

	//Info for the Tardis' thread
	HANDLE handle;
	DWORD address;

	glm::vec3 position;
	glm::vec3 scannerPosition;
	bool scannerOutTheWall;
	cRandThreaded* pRand;
};

#endif