#include "ThreadPool.h"

#include <iostream>
#include <string>

ThreadPool::ThreadPool(int workerCount)
{
	workerCount = workerCount;

	for (int i = 0; i < workerCount; i++) {
		InactiveThreads.push(new WorkerThread(i, this));
	}
}

void ThreadPool::StartScheduling()
{
	//std::string str = "Starting Thread Pool with " + std::to_string(workerCount) + " threads.\n";
	//std::cerr << str;
	this->isRunning = true;
	this->start();
}

void ThreadPool::StopScheduling()
{
	this->isRunning = false;
}

void ThreadPool::ScheduleTask(IWorkerAction* task)
{
	this->PendingTasks.push(task);
	//std::string str = "Scheduling Task: " + std::to_string(PendingTasks.size()) + " tasks.\n";
	//std::cerr << str;
}

void ThreadPool::run()
{
	while (this->isRunning) {
		// Has task to do
		if (!this->PendingTasks.empty()) {
			// Has thread available
			if (!this->InactiveThreads.empty()) {
				// Take the queued inactive thread
				auto workerThread = this->InactiveThreads.front();
				this->InactiveThreads.pop();

				// Assign id in unordered map
				this->ActiveThreads[workerThread->GetID()] = workerThread;

				// Grab and assign queued task
				auto task = this->PendingTasks.front();
				workerThread->AssignTask(task);
				this->PendingTasks.pop();

				workerThread->start();
			}
		}
	}
}

void ThreadPool::OnFinishedTask(int id)
{
	if (this->ActiveThreads[id] != nullptr) {
		delete this->ActiveThreads[id];
		this->ActiveThreads.erase(id);

		InactiveThreads.push(new WorkerThread(id, this));
	}
}
