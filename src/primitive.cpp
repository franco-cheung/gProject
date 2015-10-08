#include "primitive.hpp"
#include "AppWindow.hpp"
Primitive::~Primitive()
{
}

Sphere::~Sphere()
{
}

void Sphere::walk_gl(bool picking, QMatrix4x4 m, std::string name) const
{
  // Fill me in
	// AppWindow::m_viewer->draw_sphere(picking, m, name);
	// QMatrix4x4 champ = AppWindow::m_viewer->modelMatrix;

	// AppWindow::m_viewer->ballMatrix = champ;
}
