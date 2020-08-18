#include "BuildDracoMesh.h"

#include "draco/compression/encode.h"
#include "draco/mesh/triangle_soup_mesh_builder.h"
#include "draco/compression/decode.h";

std::unique_ptr<draco::Mesh> BuildDracoMesh(MeshData meshData)
{
	const int totalFaces = meshData._verticies.size() / 9;
	const int totalVertices = meshData._verticies.size() / 3;
	const int totalNormals = meshData._normals.size() / 3;

	draco::TriangleSoupMeshBuilder mb;
	mb.Start(totalFaces);

	const int pos_att_id = mb.AddAttribute(draco::GeometryAttribute::POSITION, 3, draco::DT_FLOAT32);

	for (draco::FaceIndex i(0); i < totalFaces; ++i) {
		mb.SetAttributeValuesForFace(pos_att_id, i,
			draco::Vector3f(meshData._verticies[i.value() * 9], meshData._verticies[i.value() * 9 + 1], meshData._verticies[i.value() * 9 + 2]).data(),
			draco::Vector3f(meshData._verticies[i.value() * 9 + 3], meshData._verticies[i.value() * 9 + 4], meshData._verticies[i.value() * 9 + 5]).data(),
			draco::Vector3f(meshData._verticies[i.value() * 9 + 6], meshData._verticies[i.value() * 9 + 7], meshData._verticies[i.value() * 9 + 8]).data());
	}

	const int nor_att_id = mb.AddAttribute(draco::GeometryAttribute::NORMAL, 3, draco::DT_FLOAT32);

	for (draco::FaceIndex i(0); i < totalFaces; ++i) {
		mb.SetAttributeValuesForFace(nor_att_id, i,
			draco::Vector3f(meshData._normals[i.value() * 9], meshData._normals[i.value() * 9 + 1], meshData._normals[i.value() * 9 + 2]).data(),
			draco::Vector3f(meshData._normals[i.value() * 9 + 3], meshData._normals[i.value() * 9 + 4], meshData._normals[i.value() * 9 + 5]).data(),
			draco::Vector3f(meshData._normals[i.value() * 9 + 6], meshData._normals[i.value() * 9 + 7], meshData._normals[i.value() * 9 + 8]).data());
	}

	const int col_att_id = mb.AddAttribute(draco::GeometryAttribute::COLOR, 3, draco::DT_FLOAT32);

	for (draco::FaceIndex i(0); i < totalFaces; ++i) {
		mb.SetAttributeValuesForFace(col_att_id, i,
			draco::Vector3f(meshData._colors[i.value() * 9], meshData._colors[i.value() * 9 + 1], meshData._colors[i.value() * 9 + 2]).data(),
			draco::Vector3f(meshData._colors[i.value() * 9 + 3], meshData._colors[i.value() * 9 + 4], meshData._colors[i.value() * 9 + 5]).data(),
			draco::Vector3f(meshData._colors[i.value() * 9 + 6], meshData._colors[i.value() * 9 + 7], meshData._colors[i.value() * 9 + 8]).data());
	}

	return mb.Finalize();
}
