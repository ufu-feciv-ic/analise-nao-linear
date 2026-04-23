#pragma once

#include <cstdint>

#include "model/DistributedLoad.h"
#include "model/Node.h"

struct LoadToolState
{
    NodalLoad pendingNodalLoad;
    DistributedLoadValue pendingDistributedLoad;
    bool pendingDistributedLoadVariable = false;
    std::uint64_t distributedLoadPanelSelectionToken = 0;
    bool distributedLoadPanelSelectionTokenInitialized = false;
};
