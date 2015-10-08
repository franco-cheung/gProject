#ifndef CS488_SKYBOX_HPP
#define CS488_SKYBOX_HPP

#include <QVector3D>
#include <vector>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <GL/glu.h>
//
class skybox{

public:
	skybox();
	~skybox();
	std::vector<GLfloat> sphereVerts;

	void load_skybox();
	void setup_Skybox();
	void load_skyboxBuffer();
	void renderSky();
private:
	GLuint texID;
	QOpenGLVertexArrayObject s_VAO;
    QOpenGLBuffer mPosBuffer;
};
#endif