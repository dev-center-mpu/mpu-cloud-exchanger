#include "BuildModelTree.h"
#include <attr_product.h>
#include <solid.h>
#include <mesh.h>
#include <attr_color.h>
#include <instance.h>
#include <assembly.h>
#include <algorithm>
#include "Tools.h"

MeshData CalculateBaseMesh(const MbMesh& c3dMesh, uint32 baseColor) {
	MeshData mesh;
	mesh._mode = Mode::TRIANGLES;

	float a, d, sp, sh, t, e;
	c3dMesh.GetVisual(a, d, sp, sh, t, e);
	mesh.metallic = sh / 100.f;
	mesh.roughness = sp;
	mesh.opacity = t;

	mesh.ambient = a;
	mesh.diffuse = d;
	mesh.specularity = sp;
	mesh.shininess = sh;
	mesh.opacity = t;
	mesh.emission = e;

	auto color = RGB2uint32(1, 1, 1);
	const MbColor* colorAttr = static_cast<const MbColor*>(c3dMesh.GetSimpleAttribute(at_Color));

	if (c3dMesh.IsColored()) {
		color = c3dMesh.GetColor();
		mesh.isColored = true;
	}
	if (colorAttr != NULL) {
		color = colorAttr->Color();
		mesh.isColored = true;
	}
	mesh._color = color;

	if (c3dMesh.GetGrid(0)) {
		MbFloatVector3D n; MbFloatPoint3D p;

		if (c3dMesh.GetGrid(0)->NormalsCount() != 0) {
			c3dMesh.GetGrid(0)->GetNormal(0, n);
			mesh._normalMax = n;
			mesh._normalMin = n;
		}

		if (c3dMesh.GetGrid(0)->PointsCount() != 0) {
			c3dMesh.GetGrid(0)->GetPoint(0, p);
			mesh._vertexMax = p;
			mesh._vertexMin = p;
		}
	}

	for (size_t gridIndex = 0; gridIndex < c3dMesh.GridsCount(); gridIndex++) {
		const MbGrid* grid = c3dMesh.GetGrid(gridIndex);
		uint gridColor = grid->GetColor();
		if (!grid->IsColored()) gridColor = baseColor;

		if (grid == NULL) continue;
		for (size_t trigIndex = 0; trigIndex < grid->TrianglesCount(); trigIndex++) {
			MbFloatPoint3D p1, p2, p3;

			MbFloatVector3D n1, n2, n3;

			if (!grid->GetTrianglePoints(trigIndex, p1, p2, p3)) continue;
			if (!grid->GetTriangleNormals(trigIndex, n1, n2, n3)) continue;
			float r, g, b;
			uint322RGB(gridColor, r, g, b);
			mesh._colors.insert(mesh._colors.cend(), { r, g, b });
			mesh._colors.insert(mesh._colors.cend(), { r, g, b });
			mesh._colors.insert(mesh._colors.cend(), { r, g, b });

			std::vector<float> canditaes = { mesh._normalMax.x, n1.x, n2.x, n3.x };
			mesh._normalMax.x = *std::max_element(canditaes.cbegin(), canditaes.cend());
			canditaes[0] = mesh._normalMin.x;
			mesh._normalMin.x = *std::min_element(canditaes.cbegin(), canditaes.cend());

			canditaes = { mesh._normalMax.y, n1.y, n2.y, n3.y };
			mesh._normalMax.y = *std::max_element(canditaes.cbegin(), canditaes.cend());
			canditaes[0] = mesh._normalMin.y;
			mesh._normalMin.y = *std::min_element(canditaes.cbegin(), canditaes.cend());

			canditaes = { mesh._normalMax.z, n1.z, n2.z, n3.z };
			mesh._normalMax.z = *std::max_element(canditaes.cbegin(), canditaes.cend());
			canditaes[0] = mesh._normalMin.z;
			mesh._normalMin.z = *std::min_element(canditaes.cbegin(), canditaes.cend());

			canditaes = { mesh._vertexMax.x, p1.x, p2.x, p3.x };
			mesh._vertexMax.x = *std::max_element(canditaes.cbegin(), canditaes.cend());
			canditaes[0] = mesh._vertexMin.x;
			mesh._vertexMin.x = *std::min_element(canditaes.cbegin(), canditaes.cend());

			canditaes = { mesh._vertexMax.y, p1.y, p2.y, p3.y };
			mesh._vertexMax.y = *std::max_element(canditaes.cbegin(), canditaes.cend());
			canditaes[0] = mesh._vertexMin.y;
			mesh._vertexMin.y = *std::min_element(canditaes.cbegin(), canditaes.cend());

			canditaes = { mesh._vertexMax.z, p1.z, p2.z, p3.z };
			mesh._vertexMax.z = *std::max_element(canditaes.cbegin(), canditaes.cend());
			canditaes[0] = mesh._vertexMin.z;
			mesh._vertexMin.z = *std::min_element(canditaes.cbegin(), canditaes.cend());

			mesh._verticies.insert(mesh._verticies.cend(), { p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, p3.x, p3.y, p3.z });
			mesh._normals.insert(mesh._normals.cend(), { n1.x, n1.y, n1.z, n2.x, n2.y, n2.z, n3.x, n3.y, n3.z });
		}
	}

	return mesh;
}

int checkForExistingMesh(std::multimap<MbItem*, Node>& map, MbItem* item, Type type) {
	if (!item) return -1;
	auto iter = map.find(item);
	MbSpaceItem* sItem = item;
	for (iter; iter != map.cend(); iter++) {
		if (iter->second._item == 0) continue;
		if (iter->second._item->IsA() != st_Solid) continue;
		if (iter->second._item != sItem) continue;
		if (iter->second._type == type) return iter->second._mesh;
	}

	return -1;

	/*MbSpaceItem* sItem = item;
	if (!sItem) return -1;
	for (Node n : nodes) {
		if (n._item == 0) continue;
		if (n._item->IsA() != st_Solid) continue;
		if (n._type != type) continue;
		if (n._item == sItem) return n._mesh;
		//if (n._item->IsSame(*sItem)) return n._mesh;
	}
	*/
}

MeshData CalculateWireMesh(const MbMesh& c3dMesh, uint32 baseColor) {
	MeshData wire;

	wire._mode = Mode::LINE;
	float r, g, b;
	uint322RGB(baseColor, r, g, b);
	float average = (r + g + b) / 3.f;
	wire._color = average < 0.1 ? RGB2uint32(0.1, 0.1, 0.1) : RGB2uint32(0, 0, 0);
	wire.opacity = 1;

	int lastIndex = 0;
	for (int k = 0; k < c3dMesh.PolygonsCount(); k++) {
		const MbPolygon3D* polygon = c3dMesh.GetPolygon(k);

		if (polygon == NULL) continue;
		const MbTopItem* item = polygon->TopItem();
		if ((item == NULL) || (item->IsA() != tt_CurveEdge)) continue;

		for (int i = 0; i < polygon->Count(); i++) {
			MbFloatPoint3D pnt;
			polygon->GetPoint(i, pnt);

			if (i < 2) {
				wire._verticies.insert(wire._verticies.cend(), { pnt.x, pnt.y, pnt.z });
				continue;
			}
			else if (i == polygon->Count() - 1 && i % 2 == 0) {
				MbFloatPoint3D prevPnt;
				polygon->GetPoint(i - 1, prevPnt);
				wire._verticies.insert(wire._verticies.cend(), { pnt.x, pnt.y, pnt.z, prevPnt.x, prevPnt.y, prevPnt.z });
			}
			else {
				MbFloatPoint3D prevPnt;
				polygon->GetPoint(i - 1, prevPnt);

				wire._verticies.insert(wire._verticies.cend(), { prevPnt.x, prevPnt.y, prevPnt.z, pnt.x, pnt.y, pnt.z });
			}

		}
	}

	int totalPnts = (int)wire._verticies.size() / 3.0;
	if (totalPnts % 2 != 0) {
		wire._verticies.push_back(wire._verticies[wire._verticies.size() - 3]);
		wire._verticies.push_back(wire._verticies[wire._verticies.size() - 2]);
		wire._verticies.push_back(wire._verticies[wire._verticies.size() - 1]);
	}

	if (wire._verticies.size() > 3) {
		wire._vertexMin.Init(wire._verticies[0], wire._verticies[1], wire._verticies[2]);
		wire._vertexMax.Init(wire._verticies[0], wire._verticies[1], wire._verticies[2]);
	}

	for (int i = 0; i < wire._verticies.size(); i += 3) {
		MbFloatPoint3D p(wire._verticies[i], wire._verticies[i + 1], wire._verticies[i + 2]);

		wire._vertexMin.x = std::min(wire._vertexMin.x, p.x);
		wire._vertexMin.y = std::min(wire._vertexMin.y, p.y);
		wire._vertexMin.z = std::min(wire._vertexMin.z, p.z);

		wire._vertexMax.x = std::max(wire._vertexMax.x, p.x);
		wire._vertexMax.y = std::max(wire._vertexMax.y, p.y);
		wire._vertexMax.z = std::max(wire._vertexMax.z, p.z);
	}

	return wire;
}

std::vector<double> c3dMatrix2Vec(const MbMatrix3D mat) {
	std::vector<double> matrix;

	if (mat.IsSingle()) return matrix;

	for (size_t row = 0; row < 4; row++) {
		matrix.push_back(mat.GetFullRow(row).x);
		matrix.push_back(mat.GetFullRow(row).y);
		matrix.push_back(mat.GetFullRow(row).z);
		matrix.push_back(mat.GetFullRow(row).w);
	}

	return matrix;
}

int BuildModelTree(const MbItem* root, MbPlacement3D fromLCS, MbMatrix3D worldMat, NodeVector& nodes, MeshVector& meshes, const MbStepData& sd, const MbFormNote& fn, std::multimap<MbItem*, Node>& instanceMap, bool calculateEdges)
{
	MbeSpaceType type = root->IsA();
	c3d::AttrVector productAttribs; std::string sName, sId, sDescr;
	c3d::string_t name, id, descr;
	root->GetAttributes(productAttribs, at_ProductAttribute, at_ProductInfo);
	if (!productAttribs.empty()) {
		MbProductInfo* prod = (MbProductInfo*)productAttribs[0];
		prod->GetDataStd(sId, sName, sDescr);
		//prod->GetData(id, name, descr);
	}

	switch (type)
	{
	case st_Solid:
	{
		const MbSolid* solid = static_cast<const MbSolid*>(root);

		MbPlacement3D lcs;
		solid->GetPlacement(lcs);
		MbMatrix3D trs;
		fromLCS.GetMatrixToPlace(lcs, trs);
		MbMatrix3D globalTRS(worldMat, trs);

		Node solidNode;
		unsigned int nodeIndex = nodes.size();
		solidNode._index = nodeIndex;
		solidNode._transformMatrix = c3dMatrix2Vec(trs);
		solidNode._worldMatrix = c3dMatrix2Vec(globalTRS);

		if (sName.empty()) sName = u8"Деталь";
		auto ansiName = ansi2unicode(sName);
		solidNode._name = utf8_encode(ansiName);
		//solidNode._name = utf8_encode(ToWstring(name));
		solidNode._type = Part;
		//solidNode._item = (MbItem*)root;

		solidNode._mesh = -1;
		solidNode._children = { nodeIndex + 1 };
		if (calculateEdges) solidNode._children.push_back(nodeIndex + 2);
		nodes.push_back(solidNode);

		MbMesh c3dMesh;
		MbFaceShell* shell = solid->GetShell();
		shell->CalculateMesh(sd, fn, c3dMesh);
		c3dMesh.ConvertAllToTriangles();

		Node meshNode;
		unsigned int meshNodeIndex = nodes.size();
		meshNode._index = meshNodeIndex;
		meshNode._name = u8"Оболочка";
		meshNode._type = Type::Mesh;
		meshNode._item = (MbItem*)root;

		auto color = RGB2uint32(1, 1, 1);
		const MbColor* colorAttr = static_cast<const MbColor*>(solid->GetSimpleAttribute(at_Color));

		if (solid->IsColored())
			color = solid->GetColor();
		if (colorAttr != NULL)
			color = colorAttr->Color();

		int existedMeshIndex = checkForExistingMesh(instanceMap, (MbItem*)root, Type::Mesh);
		if (existedMeshIndex >= 0) {
			meshNode._mesh = existedMeshIndex;
			//std::cout << "Found existed mesh\n";
		}
		else {
			MeshData mesh = CalculateBaseMesh(c3dMesh, color);
			if (mesh._verticies.empty()) return -1;
			mesh._color = color;

			unsigned int meshIndex = meshes.size();
			meshNode._mesh = meshIndex;
			meshes.push_back(mesh);

			instanceMap.insert({ (MbItem*)root, meshNode });
		}

		nodes.push_back(meshNode);

		if (calculateEdges) {
			Node wireNode;
			unsigned int wireNodeIndex = nodes.size();
			wireNode._index = wireNodeIndex;
			wireNode._name = u8"Обводка";
			wireNode._type = Wireframe;
			wireNode._item = (MbItem*)root;

			int existedWireMeshIndex = checkForExistingMesh(instanceMap, (MbItem*)root, Wireframe);
			if (existedWireMeshIndex >= 0) {
				wireNode._mesh = existedWireMeshIndex;
			}
			else {
				MeshData wire = CalculateWireMesh(c3dMesh, color);
				unsigned int wireIndex = meshes.size();
				wireNode._mesh = wireIndex;
				meshes.push_back(wire);

				instanceMap.insert({ (MbItem*)root, wireNode });

			}

			nodes.push_back(wireNode);

		}

		return nodeIndex;
	}
	break;
	case st_Instance:
	{
		const MbInstance* inst = static_cast<const MbInstance*>(root);

		if (inst) {
			MbPlacement3D lcs = inst->GetPlacement();
			MbMatrix3D trs;
			fromLCS.GetMatrixToPlace(lcs, trs);
			MbMatrix3D worldMatrix(worldMat, trs);

			const MbItem* item = static_cast<const MbItem*>(inst->GetItem());
			if (item)
				return BuildModelTree(item, lcs, worldMat, nodes, meshes, sd, fn, instanceMap, calculateEdges);
			else return -1;
		}
		else return -1;

	}
	break;
	case st_Assembly:
	{
		const MbAssembly* assm = static_cast<const MbAssembly*>(root);
		auto lcs = assm->GetPlacement();
		MbMatrix3D trs;
		fromLCS.GetMatrixToPlace(lcs, trs);

		if (assm) {
			Node node;
			if (sName.empty()) sName = u8"Сборка";
			auto ansiName = ansi2unicode(sName);
			node._name = utf8_encode(ansiName);
			unsigned int nodeIndex = nodes.size();
			node._index = nodeIndex;
			node._type = Assembly;
			node._mesh = -1;
			node._transformMatrix = c3dMatrix2Vec(trs);
			nodes.push_back(node);
			//std::cout << tab << "Assembly " << nodeIndex << " " << unicode2ansi(ToWstring(name)) << "\n";

			MbMatrix3D worldMatrix(worldMat, trs);

			std::vector<unsigned int> children;
			for (size_t i = 0; i < assm->ItemsCount(); i++) {
				const MbItem* assmItem = assm->GetItem(i);
				if (assmItem) {
					int index = BuildModelTree(assmItem, lcs, worldMatrix, nodes, meshes, sd, fn, instanceMap, calculateEdges);
					if (index >= 0) children.push_back(index);
				}
			}

			nodes[nodeIndex]._children = children;
			return nodeIndex;
		}
		else return -1;
	}
	break;
	default:
		//std::cout << tab << type << " - Skip\n";
		return -1;
		break;
	}
}
