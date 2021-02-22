#include "GLTFWrite.h"

#include "BuildDracoMesh.h"
#include "BuildModelTree.h"
#include "DrawModel.h"
#include <model.h>
#include <attr_color.h>

#include "json.hpp"
using json = nlohmann::json;
#include "../cpp-base64/base64.h"
#include "draco/compression/encode.h"
#include "draco/compression/decode.h";
#include <basetsd.h>

struct BufferView { // glTF BufferView
	unsigned short buffer = 0;
	unsigned byteOffset = 0;
	unsigned byteLength = 0;
	unsigned short byteStride = 0;
	unsigned target = 0;
};

enum class ComponentType { // gltf Component Type
	BYTE = 5120,
	UNSIGNED_BYTE = 5121,
	SHORT = 5122,
	UNSIGNED_SHORT = 5123,
	UNSIGNED_INT = 5125,
	FLOAT = 5126
};

struct Accessor { // glTF Accessor
	int bufferView = -1;
	ComponentType componentType = ComponentType::BYTE;
	int count = 0;
	std::vector<float> max;
	std::vector<float> min;
	std::string type = "SCALAR";
};

MbeConvResType GLTFWrite(MbModel& model, char* &outBuffer, size_t& outLength, char*& thumbnailBuffer, size_t& thumbnailLen, const bool &dracoCompress, const bool &binaryOutput)
{
	draco::DataBuffer externalBuffer, previewBuffer;
	json jsonOutput;
	const MbItem* root = model.GetItem(0);

	if (!root) {
		std::cerr << "Root pointer is NULL!\n";
		return cnv_NoObjects;
	}

	MbStepData stepData(ist_SpaceStep, Math::visualSag);
	MbFormNote formNote(false, true);
	//formNote.SetExact(true);

	NodeVector vNodes; MeshVector vMeshes; std::multimap<MbItem*, Node> instanceMap;
	BuildModelTree(root, MbPlacement3D::global, MbPlacement3D::global.GetMatrixFrom(), vNodes, vMeshes, stepData, formNote, instanceMap, true);

	previewBuffer = DrawModel(vMeshes, vNodes);

	jsonOutput["asset"] = { {"version", "2.0"}, {"generator", "MPU-Cloud Converter 2020"} };
	if (dracoCompress) {
		jsonOutput["extensionsRequired"] = { "KHR_draco_mesh_compression" };
		jsonOutput["extensionsUsed"] = { "KHR_draco_mesh_compression" };
	}

	// Scenes

	jsonOutput["scenes"] = { { {"nodes", { 0 } } } };
	jsonOutput["scene"] = 0;

	// Nodes

	std::list<json> jNodeList;
	for (const Node& n : vNodes) {
		json jNode = { {"name", n._name} };
		if (!n._transformMatrix.empty()) jNode["matrix"] = n._transformMatrix;
		if (!n._children.empty()) jNode["children"] = n._children;
		if (n._mesh >= 0) jNode["mesh"] = n._mesh;
		if (n._type > 0 && n._type < 5) {
			std::string type;
			switch (n._type)
			{
			case Assembly: type = u8"Сборка"; break;
			case Part: type = u8"Деталь"; break;
			case Mesh: type = u8"Оболочка";  break;
			case Wireframe: type = u8"Обводка";  break;
			}
			jNode["extras"] = { {"type", type} };
		}

		jNodeList.push_back(jNode);
	}
	jsonOutput["nodes"] = jNodeList;


	// Buffer & BufferViews

	draco::EncoderBuffer dracoEncBuf;
	draco::Encoder dracoEncoder;

	dracoEncoder.SetEncodingMethod(draco::MESH_EDGEBREAKER_ENCODING);
	dracoEncoder.SetSpeedOptions(3, 3);

	std::list<BufferView> bufferViews;
	std::list<Accessor> accessors;
	int bufferViewIndex = 0;

	for (int i = 0; i < vMeshes.size(); i++) {
		MeshData& m = vMeshes[i];
		if (m._mode == Mode::TRIANGLES) {
			BufferView bV;

			if (dracoCompress) {
				//bV.byteOffset = dracoBuffer.data_size();
				bV.byteOffset = externalBuffer.data_size();

				dracoEncBuf.Clear();
				auto mesh = BuildDracoMesh(m);

				dracoEncoder.EncodeMeshToBuffer(*mesh, &dracoEncBuf);
				m._points = m._verticies.size() / 3;
				//m._points = mesh->attribute(0)->buffer()->data_size() / mesh->attribute(0)->byte_stride();
				//m._points = mesh->num_points();
				m._faces = m._verticies.size() / 9;
				//m._faces = mesh->num_faces();

				for (int j = 0; j < m._faces; j++) {
					m._indexMax = std::max(m._indexMax, (unsigned)mesh->face(draco::FaceIndex(j))[0].value());
					m._indexMax = std::max(m._indexMax, (unsigned)mesh->face(draco::FaceIndex(j))[1].value());
					m._indexMax = std::max(m._indexMax, (unsigned)mesh->face(draco::FaceIndex(j))[2].value());
				}

				// Data Aligment
				int remain = dracoEncBuf.size() % 4;
				if (remain > 0) dracoEncBuf.Resize(dracoEncBuf.size() + 4 - remain);

				/*
				*	For performance and compatibility reasons,
				*	each element of a vertex attribute must be
				* 	aligned to 4-byte boundaries inside bufferView
				*/

				externalBuffer.Update(dracoEncBuf.data(), dracoEncBuf.size(), externalBuffer.data_size());

				bV.buffer = 0;
				bV.byteLength = dracoEncBuf.size();
				bufferViews.push_back(bV);
				bufferViewIndex++;
			}
			else {
				dracoEncoder.SetEncodingMethod(draco::MESH_SEQUENTIAL_ENCODING);

				auto mesh = BuildDracoMesh(m);
				dracoEncBuf.Clear();
				dracoEncoder.EncodeMeshToBuffer(*mesh, &dracoEncBuf);

				draco::Decoder decoder;
				draco::DecoderBuffer dracoDecBuf;
				dracoDecBuf.Init(dracoEncBuf.data(), dracoEncBuf.size());
				auto maybe_mesh = decoder.DecodeMeshFromBuffer(&dracoDecBuf);
				auto decoded_mesh = std::move(maybe_mesh).value();

				BufferView bV;
				bV.byteOffset = externalBuffer.data_size();

				externalBuffer.Update(decoded_mesh->attribute(0)->buffer()->data(), decoded_mesh->attribute(0)->buffer()->data_size(), externalBuffer.data_size());

				m._points = decoded_mesh->attribute(0)->buffer()->data_size() / decoded_mesh->attribute(0)->byte_stride();
				bV.buffer = 0;
				bV.byteLength = externalBuffer.data_size() - bV.byteOffset;
				bufferViews.push_back(bV);

				bV.byteOffset = externalBuffer.data_size();

				externalBuffer.Update(decoded_mesh->attribute(1)->buffer()->data(), decoded_mesh->attribute(1)->buffer()->data_size(), externalBuffer.data_size());
				bV.byteLength = externalBuffer.data_size() - bV.byteOffset;
				bufferViews.push_back(bV);

				bV.byteOffset = externalBuffer.data_size();

				externalBuffer.Update(decoded_mesh->attribute(2)->buffer()->data(), decoded_mesh->attribute(2)->buffer()->data_size(), externalBuffer.data_size());
				bV.byteLength = externalBuffer.data_size() - bV.byteOffset;
				bufferViews.push_back(bV);

				bV.byteOffset = externalBuffer.data_size();

				m._faces = decoded_mesh->num_faces();
				m._indexMax = decoded_mesh->num_points() - 1;

				for (int j = 0; j < m._faces; j++) {
					m._indicies.push_back((uint)decoded_mesh->face(draco::FaceIndex(j))[0].value());
					m._indicies.push_back((uint)decoded_mesh->face(draco::FaceIndex(j))[1].value());
					m._indicies.push_back((uint)decoded_mesh->face(draco::FaceIndex(j))[2].value());
				}
				externalBuffer.Update(m._indicies.data(), m._indicies.size() * sizeof(uint), externalBuffer.data_size());

				bV.byteLength = externalBuffer.data_size() - bV.byteOffset;
				bufferViews.push_back(bV);
			}

			// Accessors

			Accessor accessorVerticies;

			accessorVerticies.count = m._points;
			accessorVerticies.componentType = ComponentType::FLOAT;

			accessorVerticies.max.insert(accessorVerticies.max.cend(), { m._vertexMax.x, m._vertexMax.y, m._vertexMax.z });
			accessorVerticies.min.insert(accessorVerticies.min.cend(), { m._vertexMin.x, m._vertexMin.y, m._vertexMin.z });

			accessorVerticies.type = "VEC3";
			if (!dracoCompress) accessorVerticies.bufferView = bufferViewIndex++;

			accessors.push_back(accessorVerticies);

			Accessor accessorNormals;

			accessorNormals.count = m._points;
			accessorNormals.componentType = ComponentType::FLOAT;

			accessorNormals.max.insert(accessorNormals.max.cend(), { m._normalMax.x, m._normalMax.y, m._normalMax.z });
			accessorNormals.min.insert(accessorNormals.min.cend(), { m._normalMin.x, m._normalMin.y, m._normalMin.z });

			accessorNormals.type = "VEC3";
			if (!dracoCompress) accessorNormals.bufferView = bufferViewIndex++;

			accessors.push_back(accessorNormals);

			Accessor accessorColors;

			accessorColors.count = m._points;
			accessorColors.componentType = ComponentType::FLOAT;

			accessorColors.type = "VEC3";
			if (!dracoCompress) accessorColors.bufferView = bufferViewIndex++;

			accessors.push_back(accessorColors);

			Accessor accessorIndices;
			accessorIndices.count = m._faces * 3;

			accessorIndices.componentType = ComponentType::UNSIGNED_INT;
			accessorIndices.max.push_back((float)m._indexMax);
			accessorIndices.min.push_back(0);

			accessorIndices.type = "SCALAR";
			if (!dracoCompress) accessorIndices.bufferView = bufferViewIndex++;

			accessors.push_back(accessorIndices);

		}
		else if (m._mode == Mode::LINE) {
			BufferView bV;
			bV.buffer = 0;
			bV.byteOffset = externalBuffer.data_size();
			bV.byteLength = m._verticies.size() * sizeof(float);

			externalBuffer.Update(m._verticies.data(), m._verticies.size() * sizeof(float), externalBuffer.data_size());

			bufferViews.push_back(bV);

			Accessor accessorWire;

			accessorWire.componentType = ComponentType::FLOAT;
			accessorWire.count = m._verticies.size() / 3;

			accessorWire.max.insert(accessorWire.max.cend(), { m._vertexMax.x, m._vertexMax.y, m._vertexMax.z });
			accessorWire.min.insert(accessorWire.min.cend(), { m._vertexMin.x, m._vertexMin.y, m._vertexMin.z });

			accessorWire.type = "VEC3";
			accessorWire.bufferView = bufferViewIndex++;
			accessors.push_back(accessorWire);
		}
	}

	if (!binaryOutput) {
		std::string b64Buffer = base64_encode(externalBuffer.data(), externalBuffer.data_size());
		jsonOutput["buffers"] = {
			{ {"byteLength", externalBuffer.data_size()}, {"uri", "data:application/octet-stream;base64," + b64Buffer} }
		};
	}
	else {
		jsonOutput["buffers"] = { { {"byteLength",  externalBuffer.data_size()} } };
	}

	std::list<json> jBufferViewList;
	for (auto& bufferView : bufferViews) {
		json jBufferView = { { "buffer", bufferView.buffer }, {"byteOffset", bufferView.byteOffset}, {"byteLength", bufferView.byteLength} };
		jBufferViewList.push_back(jBufferView);
	}
	jsonOutput["bufferViews"] = jBufferViewList;

	std::list<json> jsonAccessorList;

	for (const Accessor& accessor : accessors) {
		json jsonAccessor;
		if (accessor.bufferView == -1) {
			jsonAccessor = { {"componentType", (int)accessor.componentType}, {"count", accessor.count}, {"type", accessor.type} };
		}
		else {
			jsonAccessor = { {"componentType", (int)accessor.componentType}, {"count", accessor.count}, {"type", accessor.type}, {"bufferView", accessor.bufferView } };
		}
		if (!accessor.max.empty()) jsonAccessor.push_back({ "max", accessor.max });
		if (!accessor.min.empty()) jsonAccessor.push_back({ "min", accessor.min });
		jsonAccessorList.push_back(jsonAccessor);
	}

	jsonOutput["accessors"] = jsonAccessorList;

	// Meshes

	std::list<json> jMeshList;

	for (int i = 0, attrIndex = 0; i < vMeshes.size(); i++) {
		const MeshData& m = vMeshes[i];
		json jMesh;

		json jMeshAttributes;
		if (m._mode == Mode::TRIANGLES) {
			jMeshAttributes = { {  "POSITION", attrIndex++ }, { "NORMAL", attrIndex++ }, { "COLOR_0", attrIndex++ } };
			if (dracoCompress) {
				json jExtentions = { "extensions", { {"KHR_draco_mesh_compression", { { "bufferView", i }, {"attributes", { { "POSITION", 0 }, { "NORMAL", 1 }, {"COLOR_0", 2} } } } } } };
				jMesh = { {"primitives", { { { "mode", (int)m._mode }, {"attributes", jMeshAttributes }, { "indices", attrIndex++ }, {"material", i }, jExtentions } } } };
			}
			else {
				jMesh = { {"primitives", { { { "mode", (int)m._mode }, {"attributes", jMeshAttributes }, { "indices", attrIndex++ }, {"material", i } } } } };
			}
		}
		else if (m._mode == Mode::LINE) {
			jMeshAttributes = { {  "POSITION", attrIndex++ } };
			jMesh = { {"primitives", { { { "mode", (int)m._mode }, {"attributes", jMeshAttributes }, {"material", i } } } } };
		}

		jMeshList.push_back(jMesh);
	}

	jsonOutput["meshes"] = jMeshList;

	std::list<json> jMaterialList;

	for (const MeshData& m : vMeshes) {
		json jMaterial;
		float r, g, b;
		uint322RGB(m._color, r, g, b);
		if (m._mode != Mode::LINE) {
			r = 1; g = 1; b = 1;
		}
		jMaterial["pbrMetallicRoughness"] = { {"baseColorFactor", { r, g, b, m.opacity } }, { "metallicFactor", m.metallic }, {"roughnessFactor", m.roughness } };
		jMaterialList.push_back(jMaterial);
	}

	jsonOutput["materials"] = jMaterialList;

	if (binaryOutput) {
		std::string jsonStr = jsonOutput.dump();
		draco::DataBuffer jsonChunck, dataBuffer;
		jsonChunck.Update(jsonStr.c_str(), strlen(jsonStr.c_str()));
		// Data Aligment
		int remain = jsonChunck.data_size() % 4;
		if (remain > 0) {
			std::vector<char> pads;
			for (int i = 0; i < 4 - remain; i++) pads.push_back(0x20);
			jsonChunck.Update(pads.data(), pads.size() * sizeof(char), jsonChunck.data_size());
		}
		remain = externalBuffer.data_size() % 4;
		if (remain > 0)  externalBuffer.Resize(externalBuffer.data_size() + 4 - remain);

		/*
		*	The start and the end of each chunk must be aligned to 4-byte boundary.
		*/

		UINT32 magic = 0x46546C67;
		UINT32 version = 2;
		UINT32 length = sizeof(UINT32) * 7 + jsonChunck.data_size() + externalBuffer.data_size();

		UINT32 jsonChunkLength = jsonChunck.data_size();
		UINT32 jsonChunkType = 0x4E4F534A;

		UINT32 bufferChunkLength = externalBuffer.data_size();
		UINT32 bufferChunkType = 0x004E4942;

		draco::DataBuffer glb;
		glb.Update(&magic, sizeof(magic));
		glb.Update(&version, sizeof(version), glb.data_size());
		glb.Update(&length, sizeof(length), glb.data_size());

		glb.Update(&jsonChunkLength, sizeof(jsonChunkLength), glb.data_size());
		glb.Update(&jsonChunkType, sizeof(jsonChunkType), glb.data_size());
		glb.Update(jsonChunck.data(), jsonChunck.data_size(), glb.data_size());

		glb.Update(&bufferChunkLength, sizeof(bufferChunkLength), glb.data_size());
		glb.Update(&bufferChunkType, sizeof(bufferChunkType), glb.data_size());
		glb.Update(externalBuffer.data(), externalBuffer.data_size(), glb.data_size());

		outLength = glb.data_size();
		outBuffer = (char*)malloc(outLength);
		memcpy(outBuffer, glb.data(), outLength);

		//FILE* pFile;
		//fopen_s(&pFile, "myfile.bin", "wb");
		//fwrite(dataBuffer.data(), sizeof(uint8_t), length, pFile);
		//fclose(pFile);
	}
	else {
		draco::DataBuffer dataBuffer;
		std::string jsonStr = jsonOutput.dump();
		dataBuffer.Update(jsonStr.c_str(), strlen(jsonStr.c_str()));

		outLength = dataBuffer.data_size();
		outBuffer = (char*)malloc(outLength);
		memcpy(outBuffer, dataBuffer.data(), outLength);
	}

	thumbnailLen = previewBuffer.data_size();
	thumbnailBuffer = (char*)malloc(thumbnailLen);
	memcpy(thumbnailBuffer, previewBuffer.data(), thumbnailLen);

    return cnv_Success;
}
