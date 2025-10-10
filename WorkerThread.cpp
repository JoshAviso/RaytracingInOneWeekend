#include "WorkerThread.h"

#include <iostream>

WorkerThread::WorkerThread(int id, IFinishedTask* task)	: _id(id), _onFinished(task), _task(nullptr) {}

void WorkerThread::AssignTask(IWorkerAction* task)
{
	if(task != nullptr) this->_task = task;
	//std::cerr << "Assigned a task to a thread: " << _task << std::endl;
}

void WorkerThread::run()
{
	//std::cerr << "Worker Thread Started: Task == " << _task << std::endl;
	if (this->_task != nullptr) this->_task->OnStartTask();

	if (_onFinished != nullptr) _onFinished->OnFinishedTask(_id);
	//std::cerr << "Worker Thread Finished: Finish == " << _onFinished << std::endl;
}
