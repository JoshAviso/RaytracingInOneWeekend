#include "RayTracingWorkerAction.h"

void RayTracingWorkerAction::OnStartTask()
{
	// What raytracing logic I want for this

	onFinish->OnFinishedExecution();

	delete this;
}
