#include "cDalek.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

template <class T>
T getRandInRange(T min, T max, T randValueFrom0to1)
{
	double value = min + randValueFrom0to1 * static_cast<double>(max - min);
	return static_cast<T>(value);
}

cDalek::cDalek(glm::vec3 startingPos, cTardis* theDoctor, cMazeMaker* theMaze, cRandThreaded* pRandThread)
{
	InitializeCriticalSection(&(this->CS_DataLock));
	this->position = startingPos;
	this->scannerPosition = startingPos;
	this->scannerOutTheWall = false;
	this->theMM = theMaze;
	this->CS_isLocked = false;
	this->foundDoctor = false;
	this->theTardis = theDoctor;

	if (!pRandThread)
	{
		pRandThread = new cRandThreaded();
	}

	this->pRand = pRandThread;

	handle = 0;
	address = 0;
}

cDalek::~cDalek()
{
	DeleteCriticalSection(&(this->CS_DataLock));
	return;
}

bool cDalek::getDalekPosition(glm::vec3 &position)
{
	this->lockDalekData();
	position = this->position;
	this->unlockDalekData();
	return true;
}

bool cDalek::setDalekPosition(glm::vec3 position)
{
	this->lockDalekData();
	this->position = position;
	this->unlockDalekData();
	return true;
}

bool cDalek::getFoundDoctor(bool &found)
{
	this->lockDalekData();
	found = this->foundDoctor;
	this->unlockDalekData();
	return true;
}

bool cDalek::setFoundDoctor(bool found)
{
	this->lockDalekData();
	this->foundDoctor = found;
	this->unlockDalekData();
	return true;
}

bool cDalek::isDataLocked()
{
	return CS_isLocked;
}

void cDalek::beginThread()
{
	this->handle = CreateThread(
		NULL,
		0,
		DalekThread,
		(void*)this,
		0,
		&(this->address)
	);

	return;
}

void cDalek::Update()
{
	if (!scannerOutTheWall)
	{	//Our first order of business is to get the scanner into a corridor. We won't be doing this past the first update.
		scannerOutTheWall = true;
		int scannerPosX = (int)this->position.x;
		int scannerPosZ = (int)this->position.z;

		if (scannerPosZ != 0)
		{
			if (theMM->maze[scannerPosZ - 1][scannerPosX][0] == false)
			{
				setDalekPosition(glm::vec3(position.x, 0.0f, position.z - 1.0f));
				return;
			}
		}
		if (scannerPosZ != theMM->maze.size() - 1)
		{
			if (theMM->maze[scannerPosZ + 1][scannerPosX][0] == false)
			{
				setDalekPosition(glm::vec3(position.x, 0.0f, position.z + 1.0f));
				return;
			}
		}
		if (scannerPosX != 0)
		{
			if (theMM->maze[scannerPosZ][scannerPosX - 1][0] == false)
			{
				setDalekPosition(glm::vec3(position.x - 1.0f, 0.0f, position.z));
				return;
			}
		}
		if (scannerPosX != theMM->maze.size() - 1)
		{
			if (theMM->maze[scannerPosZ][scannerPosX + 1][0] == false)
			{
				setDalekPosition(glm::vec3(position.x + 1.0f, 0.0f, position.z));
				return;
			}
		}
	}
	else
	{
		glm::vec3 doctorPos(0.0f);
		glm::vec3 myPos(0.0f);
		this->theTardis->getTardisPosition(doctorPos);
		this->getDalekPosition(myPos);
		float currentDist = glm::distance(myPos, doctorPos);
		if (currentDist < 1.5f)
		{
			this->setFoundDoctor(true);
			this->theTardis->isCaptured = true;
			return;
		}
		else
		{
			this->setFoundDoctor(false);
			int scannerPosX = (int)this->position.x;
			int scannerPosZ = (int)this->position.z;
			if (scannerPosZ != 0)
			{
				if (theMM->maze[scannerPosZ - 1][scannerPosX][0] == false)
				{
					glm::vec3 potentialNewPos = glm::vec3(position.x, 0.0f, position.z - 1.0f);
					float newDist = glm::distance(potentialNewPos, doctorPos);
					if (newDist <= currentDist)
					{
						setDalekPosition(potentialNewPos);
						return;
					}
				}
			}

			if (scannerPosZ != theMM->maze.size() - 1)
			{
				if (theMM->maze[scannerPosZ + 1][scannerPosX][0] == false)
				{
					glm::vec3 potentialNewPos = glm::vec3(position.x, 0.0f, position.z + 1.0f);
					float newDist = glm::distance(potentialNewPos, doctorPos);
					if (newDist <= currentDist)
					{
						setDalekPosition(potentialNewPos);
						return;
					}
				}
			}

			if (scannerPosX != 0)
			{
				if (theMM->maze[scannerPosZ][scannerPosX - 1][0] == false)
				{
					glm::vec3 potentialNewPos = glm::vec3(position.x - 1.0f, 0.0f, position.z);
					float newDist = glm::distance(potentialNewPos, doctorPos);
					if (newDist <= currentDist)
					{
						setDalekPosition(potentialNewPos);
						return;
					}
				}
			}

			if (scannerPosX != theMM->maze.size() - 1)
			{
				if (theMM->maze[scannerPosZ][scannerPosX + 1][0] == false)
				{
					glm::vec3 potentialNewPos = glm::vec3(position.x + 1.0f, 0.0f, position.z);
					float newDist = glm::distance(potentialNewPos, doctorPos);
					if (newDist <= currentDist)
					{
						setDalekPosition(potentialNewPos);
						return;
					}
				}
			}
		}
	}
}

void cDalek::lockDalekData()
{
	EnterCriticalSection(&(this->CS_DataLock));
	this->CS_isLocked = true;
	return;
}

void cDalek::unlockDalekData()
{
	LeaveCriticalSection(&(this->CS_DataLock));
	this->CS_isLocked = false;
	return;
}

DWORD WINAPI DalekThread(void* DalekData)
{
	cDalek* thisDalek = (cDalek*)(DalekData);
	while (true)
	{
		thisDalek->Update();

		if (thisDalek->theTardis->isCaptured || thisDalek->theTardis->isEscaped)
			break;
		Sleep(100);
	}

	return 0;
}