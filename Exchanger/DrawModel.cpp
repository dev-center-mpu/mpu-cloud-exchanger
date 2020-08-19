#include "DrawModel.h"

// SFML
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// FreeImage
#include <FreeImage.h>

// C3D
#include <attr_color.h>

MbCube CalculateBoundBox(const NodeVector& nodes, const MeshVector& meshes) {
	float min_x, min_y, min_z, max_x, max_y, max_z, center_x = 0, center_y = 0, center_z = 0;
	MbCube modelBoundBox;

	min_x = meshes[0]._verticies[0], max_x = meshes[0]._verticies[0],
		min_y = meshes[0]._verticies[1], max_y = meshes[0]._verticies[1],
		min_z = meshes[0]._verticies[2], max_z = meshes[0]._verticies[2];

	for (const Node& n : nodes) {
		if (n._type != Part) continue;
		const Node& shellNode = nodes[n._children[0]];

		if (shellNode._type != Mesh) continue;
		const MeshData& m = meshes[shellNode._mesh];
		if (m._mode != Mode::TRIANGLES) continue;
		glm::mat4 transform = glm::mat4(1.0f);
		if (!n._worldMatrix.empty()) transform = glm::make_mat4(n._worldMatrix.data());

		for (int j = 0; j < m._verticies.size(); j += 3) {
			glm::vec4 vertex = glm::vec4(m._verticies[j], m._verticies[j + 1], m._verticies[j + 2], 1);
			glm::vec4 trans = transform * vertex;
			glm::vec3 worldSpace = trans;

			min_x = std::min(min_x, worldSpace.x); max_x = std::max(max_x, worldSpace.x);
			min_y = std::min(min_y, worldSpace.y); max_y = std::max(max_y, worldSpace.y);
			min_z = std::min(min_z, worldSpace.z); max_z = std::max(max_z, worldSpace.z);
		}
	}

	modelBoundBox.pmin = MbCartPoint3D(min_x, min_y, min_z);
	modelBoundBox.pmax = MbCartPoint3D(max_x, max_y, max_z);
	return modelBoundBox;
}

MbRect Calculate2DBoundBox(MbCube cuboid, glm::mat4 viewMat) {
	MbRect bb;
	MbMatrix3D mat;

	double dArray[16] = { 0.0 };

	const float* pSource = (const float*)glm::value_ptr(viewMat);
	for (int i = 0; i < 16; ++i)
		dArray[i] = pSource[i];

	/*for (size_t row = 0; row < 4; row++) {
		//mat.SetRow(row, MbHomogeneous3D(viewMat[row][0], viewMat[row][1], viewMat[row][2], viewMat[row][3]));
		mat.SetRow(row, MbHomogeneous3D(viewMat[0][row], viewMat[1][row], viewMat[2][row], viewMat[3][row]));
	}*/

	mat.SetEl(dArray);

	MbPlacement3D place(mat);

	cuboid.Transform(mat);
	//cuboid.ProjectionRect(place, bb);
	cuboid.ProjectionRect(MbPlacement3D::global, bb);

	return bb;
}

glm::mat4 CalculateViewMatrix(const MbCube& modelBoundBox, float& maxSide) {
	float side_x = modelBoundBox.GetLengthX();
	float side_y = modelBoundBox.GetLengthY();
	float side_z = modelBoundBox.GetLengthZ();

	float max_side = side_x;
	max_side = std::max(max_side, side_y);
	max_side = std::max(max_side, side_z);
	maxSide = max_side;

	float eye = maxSide * sqrt(3.f) / 2.f;
	//eye = maxSide;

	MbCartPoint3D centerPnt;
	modelBoundBox.GetCenter(centerPnt);
	glm::mat4 view = glm::lookAt(glm::vec3(eye, eye, eye),
		glm::vec3(centerPnt.x, centerPnt.y, centerPnt.z),
		glm::vec3(0.0, 1.0, 0.0));

	return view;
}

std::vector<float> TransformRGBtoBGR(std::vector<float> colors) {
	for (int i = 0; i < colors.size(); i += 3) {
		if (i < colors.size() && (i + 2) < colors.size()) {
			std::swap(colors[i], colors[i + 2]);
		}
		//float r = colors[i], b = colors[i + 2];
		//colors[i] = b; colors[i + 2] = r;
	}
	return colors;
}

void RenderShell(Node& n, MeshData& mesh, glm::mat4 viewMat) {
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	std::vector<float> bgrColors = TransformRGBtoBGR(mesh._colors);
	glm::mat4 modelMat = n._worldMatrix.empty() ? glm::mat4(1.0f) : glm::make_mat4(n._worldMatrix.data());
	glm::mat4 modelViewMat = viewMat * modelMat;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(glm::value_ptr(modelViewMat));

	for (int i = 0; i < mesh._verticies.size(); i += 9) {
		glBegin(GL_TRIANGLES);

		glColor3f(
			bgrColors[i], 
			bgrColors[i + 1],
			bgrColors[i + 2]);
		glNormal3f(
			mesh._normals[i],
			mesh._normals[i + 1],
			mesh._normals[i + 2]
		);
		glVertex3f(
			mesh._verticies[i],
			mesh._verticies[i + 1],
			mesh._verticies[i + 2]
		);

		glColor3f(
			bgrColors[i + 3],
			bgrColors[i + 4],
			bgrColors[i + 5]);
		glNormal3f(
			mesh._normals[i + 3],
			mesh._normals[i + 4],
			mesh._normals[i + 5]
		);
		glVertex3f(
			mesh._verticies[i + 3],
			mesh._verticies[i + 4],
			mesh._verticies[i + 5]
		);

		glColor3f(
			bgrColors[i + 6],
			bgrColors[i + 7],
			bgrColors[i + 8]);
		glNormal3f(
			mesh._normals[i + 6],
			mesh._normals[i + 7],
			mesh._normals[i + 8]
		);
		glVertex3f(
			mesh._verticies[i + 6],
			mesh._verticies[i + 7],
			mesh._verticies[i + 8]
		);

		glEnd();
	}

	glPopMatrix();
	glPopAttrib();

	/*glEnable(GL_LIGHTING);
	//glDisable(GL_LIGHTING);
	//glDisable(GL_BLEND);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0.5f, 1.0f);
	glEnable(GL_POLYGON_SMOOTH);

	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

 	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	glVertexPointer(3, GL_FLOAT, 0, mesh._verticies.data());
	glNormalPointer(GL_FLOAT, 0, mesh._normals.data());
	std::vector<float> bgrColors = TransformRGBtoBGR(mesh._colors);
	glColorPointer(3, GL_FLOAT, 0, bgrColors.data());

	GLfloat R, G, B;
	uint322RGB(mesh._color, B, G, R);
	GLfloat colorAmbient[] = { R * mesh.ambient,     G * mesh.ambient,     B * mesh.ambient,     mesh.opacity };
	GLfloat colorDiffuse[] = { R * mesh.diffuse,     G * mesh.diffuse,     B * mesh.diffuse,     mesh.opacity };
	GLfloat colorSpecular[] = { R * mesh.specularity, G * mesh.specularity, B * mesh.specularity, mesh.opacity };
	GLfloat colorEmission[] = { R * mesh.emission,    G * mesh.emission,    B * mesh.emission,    mesh.opacity };
	GLfloat colorIndexes[] = { mesh.ambient, mesh.diffuse, mesh.opacity };

	//glMaterialfv(GL_FRONT, GL_AMBIENT, colorAmbient);
	//glMaterialfv(GL_FRONT, GL_DIFFUSE, colorDiffuse);
	//glMaterialfv(GL_FRONT, GL_SPECULAR, colorSpecular);
	//glMaterialfv(GL_FRONT, GL_EMISSION, colorEmission);
	//glMaterialf(GL_FRONT, GL_SHININESS, mesh.shininess);
	//glMaterialfv(GL_FRONT, GL_COLOR_INDEXES, colorIndexes);

	glMatrixMode(GL_MODELVIEW);
	glm::mat4 modelView;
	glm::mat4 transform = glm::mat4(1.0f);
	if (!n._worldMatrix.empty()) transform = glm::make_mat4(n._worldMatrix.data());
	modelView = viewMat * transform;
	glLoadMatrixf(glm::value_ptr(modelView));
	glDrawArrays(GL_TRIANGLES, 0, mesh._verticies.size() / 3);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);*/
}

void RenderWireFrame(Node& n, MeshData& mesh, glm::mat4 viewMat) {
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glLineWidth(.5f);

	//std::vector<float> colors(mesh._verticies.size()); 
	//std::fill(colors.begin(), colors.end(), 0.f);

	//glEnableClientState(GL_VERTEX_ARRAY);
	//glEnableClientState(GL_COLOR_ARRAY);

	//glVertexPointer(3, GL_FLOAT, 0, mesh._verticies.data());
	//glColorPointer(3, GL_FLOAT, 0, colors.data());

	glm::mat4 modelMat = n._worldMatrix.empty() ? glm::mat4(1.0f) : glm::make_mat4(n._worldMatrix.data());
	glm::mat4 modelViewMat = viewMat * modelMat;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(glm::value_ptr(modelViewMat));

	for (int i = 0; i < mesh._verticies.size(); i += 6) {
		glBegin(GL_LINES);
		glColor3f(0.f, 0.f, 0.f);
		glVertex3f(
			mesh._verticies[i], 
			mesh._verticies[i + 1], 
			mesh._verticies[i + 2]
		);
		glVertex3f(
			mesh._verticies[i + 3],
			mesh._verticies[i + 4],
			mesh._verticies[i + 5]
		);
		glEnd();
	}

	glPopMatrix();
	glPopAttrib();

	//GLfloat R, G, B;
	//uint322RGB(mesh._color, B, G, R);
	//GLfloat colorAmbient2[] = { 0,     0, 0, 1 };
	//GLfloat colorDiffuse2[] = { 0,     0, 0, 1 };
	//GLfloat colorEmission2[] = { 0,    0,   0,    0 };

	//glMaterialfv(GL_FRONT, GL_AMBIENT, colorAmbient2);
	//glMaterialfv(GL_FRONT, GL_DIFFUSE, colorDiffuse2);

	//float colour[4] = { 0,0,0,0 };
	//glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, colour);

	//glColor3f(0, 0, 0);

	/*glm::mat4 modelMat = n._worldMatrix.empty() ? glm::mat4(1.0f) : glm::make_mat4(n._worldMatrix.data());
	glm::mat4 modelViewMat = viewMat * modelMat;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(glm::value_ptr(modelViewMat));
	glDrawArrays(GL_LINES, 0, mesh._verticies.size() / 3);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glPopMatrix();
	glPopAttrib();*/
}

void RenderNode(Node& n, NodeVector& nodes, MeshVector& meshes, glm::mat4 viewMat) {
	if (n._type != Part) return;
	if (n._children.empty()) return;

	Node& shellNode = nodes[n._children[0]];
	if (shellNode._type == Mesh) {
		MeshData& mesh = meshes[shellNode._mesh];
		if ((mesh._verticies.size() >= 3 
			&& mesh._normals.size() >= 3 
			&& mesh._colors.size() >= 3) 
			&& (mesh._verticies.size() == mesh._normals.size()
			&& mesh._normals.size() == mesh._colors.size()))
				RenderShell(n, mesh, viewMat);
	}
	
	Node& wireNode = nodes[n._children[1]];
	if (wireNode._type == Wireframe) {
		MeshData& mesh = meshes[wireNode._mesh];
		if (mesh._verticies.size() >= 3)
			RenderWireFrame(n, mesh, viewMat);
	}
}

draco::DataBuffer DrawModel(MeshVector meshes, NodeVector nodes)
{
	draco::DataBuffer thumbnailBuffer;
	if (meshes.size() == 0) return thumbnailBuffer;
	if (nodes.size() == 0) return thumbnailBuffer;

	// Инициализация окошка

	sf::RenderWindow renderWindow(sf::VideoMode(600, 600), "Drawing", sf::Style::Default);
	if (!renderWindow.isOpen()) {
		std::cerr << "ОШИБКА: Не удалось запустить окно предварительной визуализации моделей" << std::endl;
		return thumbnailBuffer;
	}
	renderWindow.setActive(true);
	renderWindow.setVisible(true);
	glViewport(0, 0, renderWindow.getSize().x, renderWindow.getSize().y);
	glClearColor(1.f, 1.f, 1.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Установка глобального освещения

	glEnable(GL_LIGHTING);
	if (!glIsEnabled(GL_LIGHTING))
		std::cerr << "ПРЕДУПРЕЖДЕНИЕ: Не удалось активировать освещение для предварительной визуализации" << std::endl;
	glEnable(GL_LIGHT0);
	if (!glIsEnabled(GL_LIGHT0))
		std::cerr << "ПРЕДУПРЕЖДЕНИЕ: Не удалось активировать источник света для предварительной визуализации" << std::endl;

	float lightAmbient = .6f, lightDiffuse = .5f, lightSpecular = .6f;
	GLfloat glfLightAmbient[] = { lightAmbient, lightAmbient, lightAmbient, 1.f };
	GLfloat glfLightDiffuse[] = { lightDiffuse, lightDiffuse, lightDiffuse, 1.f };
	GLfloat glfLightSpecular[] = { lightSpecular, lightSpecular, lightSpecular, 1.f };
	glm::vec4 lightPos = glm::vec4(-1.f, -1.f, 1.f, 0.f);

	glLightfv(GL_LIGHT0, GL_AMBIENT, glfLightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, glfLightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, glfLightSpecular);
	glLightfv(GL_LIGHT0, GL_POSITION, glm::value_ptr(lightPos));

	glEnable(GL_DEPTH_TEST);
	if (!glIsEnabled(GL_DEPTH_TEST))
		std::cerr << "ПРЕДУПРЕЖДЕНИЕ: Не удалось активировать тест глубины для предварительной визуализации" << std::endl;
	glPolygonOffset(0.5f, 1.0f);
	glEnable(GL_POLYGON_OFFSET_FILL);
	if (!glIsEnabled(GL_POLYGON_OFFSET_FILL))
		std::cerr << "ПРЕДУПРЕЖДЕНИЕ: Не удалось активировать смещение полигонов для предварительной визуализации" << std::endl;
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	if (!glIsEnabled(GL_COLOR_MATERIAL))
		std::cerr << "ПРЕДУПРЕЖДЕНИЕ: Не удалось активировать заливку моделей для предварительной визуализации" << std::endl;

	// Расчет матрицы вида и области видимости модели

	float max_side = 0; // Максимальный габарит модели
	MbCube modelBoundBox = CalculateBoundBox(nodes, meshes);
	glm::mat4 viewMat = CalculateViewMatrix(modelBoundBox, max_side);
	MbRect screenBoundBox = Calculate2DBoundBox(modelBoundBox, viewMat);

	// Установка ортогонального вида

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if (screenBoundBox.GetLengthX() > screenBoundBox.GetLengthY()) {
		double scale = screenBoundBox.GetLengthX() / screenBoundBox.GetLengthY();
		screenBoundBox.bottom *= scale; screenBoundBox.top *= scale;
	}
	else if (screenBoundBox.GetLengthX() < screenBoundBox.GetLengthY()) {
		double scale = screenBoundBox.GetLengthY() / screenBoundBox.GetLengthX();
			screenBoundBox.left *= scale; screenBoundBox.right *= scale;
	}

	screenBoundBox.Enlarge(0.1f * max_side);
	glOrtho(screenBoundBox.left, screenBoundBox.right, screenBoundBox.bottom, screenBoundBox.top, -1.0, max_side * 10);

	// Отрисовка деталей сборки через OpenGL

	for (Node& n : nodes) {
		RenderNode(n, nodes, meshes, viewMat);
	}

	// Создание снимка модели

	BYTE* pixels = new BYTE[3 * renderWindow.getSize().x * renderWindow.getSize().y];
	glReadPixels(0, 0, renderWindow.getSize().x, renderWindow.getSize().y, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	FIBITMAP* image = FreeImage_ConvertFromRawBits(pixels, renderWindow.getSize().x, renderWindow.getSize().y, 3 * renderWindow.getSize().x, 24, 0xFF0000, 0x00FF00, 0x0000FF, false);

	FIMEMORY* imgBuffer = FreeImage_OpenMemory();
	FreeImage_SaveToMemory(FIF_PNG, image, imgBuffer);
	unsigned long imgBufferSize = 0;
	unsigned char* imgData = 0;
	FreeImage_AcquireMemory(imgBuffer, &imgData, &imgBufferSize);
	thumbnailBuffer.Update(imgData, imgBufferSize);

	// Отчистка памяти и содержимого окошка

	delete[] pixels;
	FreeImage_CloseMemory(imgBuffer);
	FreeImage_Unload(image);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderWindow.clear();
	renderWindow.setActive(false);
	renderWindow.setVisible(false);
	renderWindow.close();

	return thumbnailBuffer;
}