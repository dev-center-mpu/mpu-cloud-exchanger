#pragma once

#include "draco/mesh/mesh.h"
#include "Mesh.h"

std::unique_ptr<draco::Mesh> BuildDracoMesh(MeshData meshData);

