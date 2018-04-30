#include "cTardis.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>

template <class T>
T getRandInRange(T min, T max, T randValueFrom0to1)
{
	double value = min + randValueFrom0to1 * static_cast<double>(max - min);
	return static_cast<T>(value);
}

cTardis::cTardis(glm::vec3 startingPos, glm::vec3 goal, cMazeMaker* theMaze, cRandThreaded* pRandThread)
{
	InitializeCriticalSection(&(this->CS_DataLock));
	this->position = startingPos;
	this->theMM = theMaze;
	this->CS_isLocked = false;
	this->isCaptured = false;
	this->endPosition = goal;

	if (!pRandThread)
	{
		pRandThread = new cRandThreaded();
	}

	this->pRand = pRandThread;

	handle = 0;
	address = 0;
}

cTardis::~cTardis()
{
	DeleteCriticalSection(&(this->CS_DataLock));
	return;
}

bool cTardis::getTardisPosition(glm::vec3 &position)
{
	this->lockTardisData();
	position = this->position;
	this->unlockTardisData();
	return true;
}

bool cTardis::setTardisPosition(glm::vec3 position)
{
	this->lockTardisData();
	this->position = position;
	this->unlockTardisData();
	return true;
}

bool cTardis::isDataLocked()
{
	return CS_isLocked;
}

void cTardis::lockTardisData()
{
	EnterCriticalSection(&(this->CS_DataLock));
	this->CS_isLocked = true;
	return;
}

void cTardis::unlockTardisData()
{
	LeaveCriticalSection(&(this->CS_DataLock));
	this->CS_isLocked = false;
	return;
}

void cTardis::beginThread()
{
	this->handle = CreateThread(
		NULL,
		0,
		TardisThread,
		(void*)this,
		0,
		&(this->address)
	);

	return;
}

void cTardis::Update()
{
	std::vector<glm::vec3> possiblePositions;
	glm::vec3 myPos(0.0f);
	this->getTardisPosition(myPos);
	float distToGoal = glm::distance(myPos, this->endPosition);

	for (int index = 0; index < 100; index++)
	{
		//Tardis will try to move a distance of 1 to 15 units away
		float distToMove = getRandInRange<float>(1.0f, 15.0f, (float)this->pRand->getNextRandDouble());
		float angleToMove = getRandInRange<float>(0.0f, (float)(2.0f * M_PI), (float)this->pRand->getNextRandDouble());

		float angleX = sin(angleToMove);
		float angleZ = cos(angleToMove);
		glm::vec3 direction = glm::normalize(glm::vec3(angleX, 0.0f, angleZ));
		direction = direction * distToMove;
		
		glm::vec3 newPosition = myPos + direction;

		//A: Check if it's out of the range of the maze
		if (newPosition.x < 0.0f || newPosition.x > theMM->maze.size())
			continue;
		if (newPosition.z < 0.0f || newPosition.z > theMM->maze.size())
			continue;

		//B: Check to make sure it's not inside a wall
		int xPoint = (int)newPosition.x;
		int zPoint = (int)newPosition.z;

		if (theMM->maze[zPoint][xPoint][0] == true)
			continue;

		//C: If we've made it this far, check to make sure we're closing the distance to the exit
		float newDistToGoal = glm::distance(newPosition, this->endPosition);

		if (newDistToGoal < distToGoal)
		{
			possiblePositions.push_back(newPosition);
		}
	}

	if (possiblePositions.size() == 0)
		return;

	else
	{
		glm::vec3 myNewPos(0.0f);
		float maxVal = (float)possiblePositions.size() - 0.001f;
		float floatChoice = getRandInRange<float>(0.0f, maxVal, (float)this->pRand->getNextRandDouble());
		int intChoice = (int)floatChoice;
		myNewPos = possiblePositions[intChoice];
		this->setTardisPosition(myNewPos);
	}
	this->getTardisPosition(myPos);
	std::cout << "Tardis Position: X = " << myPos.x << ", Z = " << myPos.z << std::endl;
	return;
}

DWORD WINAPI TardisThread(void* TardisData)
{
	cTardis* theTardis = (cTardis*)(TardisData);

	while (!theTardis->isCaptured)
	{
		theTardis->Update();

		Sleep(2000);
	}

	return 0;
}