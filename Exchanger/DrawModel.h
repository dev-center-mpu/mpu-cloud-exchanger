#pragma once

#include "Mesh.h" // MeshData
#include "Node.h"; // Node

// Draco3D

#include "draco/core/data_buffer.h" 

draco::DataBuffer DrawModel(MeshVector meshes, NodeVector nodes, const int width, const int height);