#pragma once

#include "IWorkerAction.h"
#include "IExecutionEvent.h"

class RayTracingWorkerAction : public IWorkerAction
{
public:
	RayTracingWorkerAction(int id, IExecutionEvent* onFinish);

private:
	int id = 0;
	IExecutionEvent* onFinish = nullptr;

	void OnStartTask() override;
};

