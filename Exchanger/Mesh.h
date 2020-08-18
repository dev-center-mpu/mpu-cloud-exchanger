#pragma once
#include <vector>
#include <mesh_float_point3d.h>

enum class Mode {
	POINTS = 0,
	LINE = 1,
	LINE_LOOP = 2,
	LINE_STRIP = 3,
	TRIANGLES = 4
};

struct MeshData
{
	unsigned int _index;
	Mode _mode = Mode::TRIANGLES;

	std::vector<float> _verticies;
	MbFloatPoint3D _vertexMax = MbFloatPoint3D(0, 0, 0);
	MbFloatPoint3D _vertexMin = MbFloatPoint3D(0, 0, 0);

	std::vector<float> _normals;
	MbFloatVector3D _normalMin = MbFloatVector3D(0, 0, 0);
	MbFloatVector3D _normalMax = MbFloatVector3D(0, 0, 0);

	std::vector<float> _colors;

	std::vector<unsigned> _indicies;
	unsigned _indexMax = 0;
	unsigned _indexMin = 0;

	int _points = 0;
	int _faces = 0;

	uint32 _color;
	bool isColored = false;

	float metallic = 0.1f;
	float roughness = 1.0f;
	float opacity = 1.0f;

	float ambient = 0.4f;
	float diffuse = 0.6f;
	float specularity = 0.7f;
	float shininess = 50.f;
	float emission = 0.0f;
};

typedef std::vector<MeshData> MeshVector;
