#include "terrain.hpp"
#include <QVector3D>
#include <vector>
#include "algebra.hpp"
#include "AppWindow.hpp"

terrain::terrain(int tWidth, int tLength)
{
	w = tWidth;
	l = tLength;

	updateNormal = false;
}

terrain::~terrain()
{
	h.clear();
	normal.clear();
}

//get the width of the terrain
int terrain::getWidth()
{
	return w;
}

//get the length of the terrain
int terrain::getLength()
{
	return l;
}

//sets the height of the terrain with the given location position
void terrain::setHeight(std::vector<float> location)
{
	h.push_back(location);
	updateNormal = false;
}

//gets the height of the terrain at position x z
float terrain::getHeight(int x, int z)
{
  //this if statements checks if the x or z values are out of the terrain bounds
	if(x > (w-1) || x < 0 || z > (l-1) || z < 0)
	{
		return -11111;
	}
	return h[z][x];
}

//computes the normal of the terrain
void terrain::computeNormal()
{
	if(updateNormal)
	{
		return;
	}

	std::vector<QVector3D> norm;
	const float fallout = 0.5f;
 // 	normal.resize(l);
 //    norm.resize(l);
	// for(int i = 0; i < l; i++) {
 //                normal[i].resize(w);
 //                norm[i].resize(w);
 //    }


	for(int z = 0; z < l; z++) {
	    for(int x = 0; x < w; x++) {
	        QVector3D sum = QVector3D(0.0f, 0.0f, 0.0f);

	        QVector3D out;
	        if (z > 0) {
	            out = QVector3D(0.0f, h[z - 1][x] - h[z][x], -1.0f);
	        }
	        QVector3D in;
	        if (z < l - 1) {
	            in = QVector3D(0.0f, h[z + 1][x] - h[z][x], 1.0f);
	        }
	        QVector3D left;
	        if (x > 0) {
	            left = QVector3D(-1.0f, h[z][x - 1] - h[z][x], 0.0f);
	        }
	        QVector3D right;
	        if (x < w - 1) {
	            right = QVector3D(1.0f, h[z][x + 1] - h[z][x], 0.0f);
	        }
	        
	        if (x > 0 && z > 0) {
	            sum =  sum + QVector3D::crossProduct(out, left).normalized();
	        }
	        if (x > 0 && z < l - 1) {
	           sum = sum + QVector3D::crossProduct(left, in).normalized();
	        }
	        if (x < w - 1 && z < l - 1) {
	           sum = sum + QVector3D::crossProduct(in, right).normalized();
	        }
	        if (x < w - 1 && z > 0) {
	           sum = sum + QVector3D::crossProduct(right, out).normalized();
	        }
	        if (sum.length() == 0) {
                sum = QVector3D(0.0f, 1.0f, 0.0f);
            }
	        //norm[z][x] = sum;
	        norm.push_back(fallout * sum);
	    }
	    normal.push_back(norm);
	    norm.clear();
    }
    updateNormal = true;
}

//this grabs the normal vector at point x z
QVector3D terrain::getNormal(int x, int z)
{
	if(!updateNormal)
	{
		computeNormal();
	}
	return normal[z][x];
}

// float terrain::heightAt(terrain* terrain, float x, float z) {
// 	if (x < 0) 
// 	{
//         x = 0;
//     }
//     else if (x > terrain->getWidth() - 1) {
//         x = terrain->getWidth() - 1;
//     }
//     if (z < 0) {
//         z = 0;
//     }
//     else if (z > terrain->getLength() - 1) {
//         z = terrain->getLength() - 1;
//     }

//     int leftX = (int)x;
//     if (leftX == terrain->getWidth() - 1) {
//         leftX--;
//     }
//     float fracX = x - leftX;
    
//     int outZ = (int)z;
//     if (outZ == terrain->getWidth() - 1) {
//         outZ--;
//     }
//     float fracZ = z - outZ;

//     //Compute the four heights for the grid cell
//     float h11 = terrain->getHeight(leftX, outZ);
//     float h12 = terrain->getHeight(leftX, outZ + 1);
//     float h21 = terrain->getHeight(leftX + 1, outZ);
//     float h22 = terrain->getHeight(leftX + 1, outZ + 1);
    
//     //Take a weighted average of the four heights
//     return (1 - fracX) * ((1 - fracZ) * h11 + fracZ * h12) +
//         fracX * ((1 - fracZ) * h21 + fracZ * h22);
// }

//This function is used to initialize the 
void terrain::field_setup()
{
  for(int z = 0; z < this->getLength() - 1; z++)
  {
    for(int x = 0; x < this->getWidth(); x++)
    {
      QVector3D norms = this->getNormal(x, z);
      iFieldNormal.push_back(norms);
      iFieldVertices.push_back(QVector3D(x, this->getHeight(x, z), z));
      iFieldTextCoord.push_back(QVector3D(x, this->getHeight(x, z), z));

      norms = this->getNormal(x, z + 1);
      iFieldNormal.push_back(norms);
      iFieldVertices.push_back(QVector3D(x, this->getHeight(x, z + 1), z + 1));
      iFieldTextCoord.push_back(QVector3D(x, this->getHeight(x, z + 1), z + 1));
      
    }
  }
  m_Textures.resize(1);
  m_Textures[0] = new Texture(GL_TEXTURE_2D, "./grassText.png");
  m_Textures[0]->Load();

  LoadTerrainBuffer();
}

//creates three buffers for rendering purposes
void terrain::LoadTerrainBuffer()
{
   t_VAO.create();
   t_VAO.bind();

   mPosBuffer.create();
   mPosBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
   mPosBuffer.bind();
   mPosBuffer.allocate(&iFieldVertices[0], sizeof(iFieldVertices[0]) * iFieldVertices.size());
   AppWindow::m_viewer->mProgram.setAttributeBuffer("mesh_pos", GL_FLOAT, 0, 3);
   AppWindow::m_viewer->mProgram.enableAttributeArray("mesh_pos");

   mTextBuffer.create();
   mTextBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
   mTextBuffer.bind();
   mTextBuffer.allocate(&iFieldTextCoord[0], sizeof(iFieldTextCoord[0]) * iFieldTextCoord.size());
   AppWindow::m_viewer->mProgram.setAttributeBuffer("mesh_text", GL_FLOAT, 0, 2);
   AppWindow::m_viewer->mProgram.enableAttributeArray("mesh_text");

   mNormBuffer.create();
   mNormBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
   mNormBuffer.bind();
   mNormBuffer.allocate(&iFieldNormal[0], sizeof(iFieldNormal[0]) * iFieldNormal.size());
   AppWindow::m_viewer->mProgram.setAttributeBuffer("mesh_norms", GL_FLOAT, 0, 3);
   AppWindow::m_viewer->mProgram.enableAttributeArray("mesh_norms");

   t_VAO.release();
}

//draws the terrain
void terrain::Render()
{
   t_VAO.bind();
   m_Textures[0]->Bind(GL_TEXTURE0);
   glDrawArrays(GL_TRIANGLE_STRIP, 0, iFieldVertices.size());   
   t_VAO.release();
}