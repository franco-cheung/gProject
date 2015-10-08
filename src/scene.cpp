#include "scene.hpp"
#include "AppWindow.hpp"
#include <iostream>

SceneNode::SceneNode(const std::string& name)
  : m_name(name)
{
}

SceneNode::~SceneNode()
{
}

void SceneNode::walk_gl(bool picking) const
{
  // Fill me in

  typename ChildList::const_iterator iter;

  for(iter = m_children.begin(); iter != m_children.end(); iter++)
  {
    (*iter)->set_transform( m_trans * (*iter)->get_transform() );

    (*iter)->walk_gl(picking);

    (*iter)->set_transform( this->m_invtrans * (*iter)->get_transform() );
  }
}

void SceneNode::rotate(char axis, double angle)
{
  //std::cerr << "Stub: Rotate " << m_name << " around " << axis << " by " << angle << std::endl;
  // Fill me in
  switch(axis)
  {
    case 'x':
      m_trans.rotate(angle, 1, 0, 0);
      break;
    case 'y':
      m_trans.rotate(angle, 0, 1, 0);
      break;
    case 'z':
      m_trans.rotate(angle, 0, 0, 1);
      break;
    default:
      break;
  }
}

void SceneNode::scale(const QVector3D& amount)
{
  //std::cerr << "Stub: Scale" << m_name << " by x:" << amount.x() << " y:" << amount.y() << " z:" << amount.z() << std:: endl;
  // Fill me in
  m_trans.scale(amount);
}

void SceneNode::translate(const QVector3D& amount)
{
  //std::cerr << "Stub: Translate " << m_name << " by " << amount << std::endl;
  //std::cerr << "Stub: Translate" << m_name << " by x:" << amount.x() << " y:" << amount.y() << " z:" << amount.z() << std:: endl;
  // Fill me in
  m_trans.translate(amount);
}

bool SceneNode::is_joint() const
{
  return false;
}

JointNode::JointNode(const std::string& name)
  : SceneNode(name)
{
}

JointNode::~JointNode()
{
}

void JointNode::walk_gl(bool picking) const
{

  typename ChildList::const_iterator iter;

  for(iter = m_children.begin(); iter != m_children.end(); iter++)
  {
    (*iter)->set_transform( m_trans * (*iter)->get_transform() );

    (*iter)->walk_gl(picking);

    (*iter)->set_transform( this->m_invtrans * (*iter)->get_transform() );
  }
}

bool JointNode::is_joint() const
{
  return true;
}

void JointNode::set_joint_x(double min, double init, double max)
{
  m_joint_x.min = min;
  m_joint_x.init = init;
  m_joint_x.max = max;
}

void JointNode::set_joint_y(double min, double init, double max)
{
  m_joint_y.min = min;
  m_joint_y.init = init;
  m_joint_y.max = max;
}

GeometryNode::GeometryNode(const std::string& name, Primitive* primitive)
  : SceneNode(name),
    m_primitive(primitive)
{
}

GeometryNode::~GeometryNode()
{
}

void GeometryNode::walk_gl(bool picking) const
{
  //std::cerr << m_name << std::endl;
  // Fill me in
  if(!picking)
  {
    m_material->apply_gl();
  }
  //m_material->apply_gl();
  m_primitive->walk_gl(picking, m_trans, m_name);

}
 
