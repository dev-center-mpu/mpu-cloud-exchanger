#pragma once

#include <model_item.h>
#include "Node.h"
#include "Mesh.h"

int BuildModelTree(const MbItem* root, MbPlacement3D fromLCS,
	MbMatrix3D worldMat, NodeVector& nodes, MeshVector& meshes, const MbStepData& sd, const MbFormNote& fn,
	std::multimap<MbItem*, Node>& instanceMap, bool calculateEdges = true);