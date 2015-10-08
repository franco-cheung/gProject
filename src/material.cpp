#include "material.hpp"
#include "AppWindow.hpp"
Material::~Material()
{
}

PhongMaterial::PhongMaterial(const Colour& kd, const Colour& ks, double shininess)
  : m_kd(kd), m_ks(ks), m_shininess(shininess)
{
}

PhongMaterial::~PhongMaterial()
{
}

void PhongMaterial::apply_gl() const
{
  // Perform OpenGL calls necessary to set up this material.
	QVector3D vecKD = QVector3D(m_kd.R(), m_kd.G(), m_kd.B());
	QVector3D vecKS = QVector3D(m_ks.R(), m_ks.G(), m_ks.B());
	AppWindow::m_viewer->mProgram.setUniformValue(AppWindow::m_viewer->mkd, vecKD);
	//AppWindow::m_viewer->mProgram.setUniformValue(AppWindow::m_viewer->mselect, vecKD);
	AppWindow::m_viewer->mProgram.setUniformValue(AppWindow::m_viewer->mks, vecKS);
	AppWindow::m_viewer->mProgram.setUniformValue(AppWindow::m_viewer->mshiny,(float)m_shininess);
}