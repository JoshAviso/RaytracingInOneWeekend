#pragma once

#include <queue>
#include <unordered_map>

#include "WorkerThread.h"
#include "IWorkerAction.h"
#include "IThread.h"

class ThreadPool : public IThread, public IFinishedTask
{
public:
	ThreadPool(int workerCount);
	~ThreadPool() {}

	void StartScheduling();
	void StopScheduling();
	void ScheduleTask(IWorkerAction* task);

private:
	bool isRunning = false;
	int workerCount = 1;

	std::queue<IWorkerAction*> PendingTasks;
	std::queue<WorkerThread*> InactiveThreads;
	std::unordered_map<int, WorkerThread*> ActiveThreads;

private:
	void run() override;
	void OnFinishedTask(int id);
};

