#pragma once

#include "IWorkerAction.h"
#include "IThread.h"

class IFinishedTask
{
public:
	virtual void OnFinishedTask(int id) = 0;
};

class WorkerThread : public IThread
{
public:
	WorkerThread(int id, IFinishedTask* task);
	~WorkerThread() {}

	void AssignTask(IWorkerAction* _task);
	int GetID() const { return _id; }

private:
	void run() override;

	int _id;
	IFinishedTask* _onFinished = nullptr;
	IWorkerAction* _task = nullptr;
};

