#pragma once
#include <model_item.h>

enum Type {
	None, Assembly, Part, Mesh, Wireframe
};

struct Node
{
	MbItem* _item = 0;
	unsigned int _index = -1;
	std::string _name;
	std::vector<double> _transformMatrix;
	std::vector<double> _worldMatrix;
	std::vector<unsigned int> _children;
	int _mesh;
	int _type = None;
};

typedef std::vector<Node> NodeVector;
typedef std::multimap<MbItem*, Node> InstanceMap;