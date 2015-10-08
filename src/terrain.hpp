#ifndef CS488_TERRAIN_HPP
#define CS488_TERRAIN_HPP

#include <QVector3D>
#include <vector>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <GL/glu.h>
// #include "AppWindow.hpp"
#include <QOpenGLFunctions_3_3_Core>
#include "texture.hpp"
class terrain{

public:
	terrain(int tWidth, int tLength);
	~terrain();
	int getWidth();
	int getLength();
	void setHeight(std::vector<float> location);
	float getHeight(int x, int z);
	void computeNormal();
	QVector3D getNormal(int x, int z);
	// float heightAt(terrain* terrain, float x, float z);
	void field_setup();
	void LoadTerrainBuffer();
	void Render();
	std::vector<std::vector<float>> h;
	std::vector<QVector3D> iFieldVertices;
    std::vector<QVector3D> iFieldNormal;

    std::vector<Texture*> m_Textures;
    std::vector<QVector3D> iFieldTextCoord;
private:

	QOpenGLVertexArrayObject t_VAO;
  //uint m_VAO;

    QOpenGLBuffer mPosBuffer;
    QOpenGLBuffer mTextBuffer;
    QOpenGLBuffer mNormBuffer;

	int w;
	int l;
	std::vector<std::vector<QVector3D>> normal;
	bool updateNormal;
};
#endif