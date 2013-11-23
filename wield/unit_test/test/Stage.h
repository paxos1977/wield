#pragma once
#include <wield/StageBase.h>
#include <wield/DispatcherBase.h>

#include "ProcessingFunctor.h"
#include "Queue.h"
#include "Stages.h"

using Stage = wield::StageBase<Stages, ProcessingFunctorInterface, Queue, wield::DispatcherBase>;