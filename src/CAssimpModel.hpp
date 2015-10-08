#ifndef CS488_CASSIMPMODEL_HPP
#define CS488_CASSIMPMODEL_HPP
#include <assimp/Importer.hpp>      
#include <assimp/scene.h>           
#include <assimp/postprocess.h>

#include <vector>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QVector3D>
#include <QVector2D>
#include "texture.hpp"
#include <QOpenGLFunctions_3_3_Core>

struct Vertex
{
    Vertex() {}

    QVector3D m_pos;
    QVector2D m_tex;
    QVector3D m_normal;

    Vertex(const QVector3D& pos, const QVector2D& tex, const QVector3D& normal)
    {
        m_pos    = pos;
        m_tex    = tex;
        m_normal = normal;
    }
};

class CAssimpModel
{
public:
   CAssimpModel();
   ~CAssimpModel();
   bool LoadModelFromFile(std::string sFilePath);
   void LoadBuffers();
   bool InitMaterials(const aiScene* scene, std::string& sFilePath);
   void RenderModel();
   //void RenderSphere(std::vector<GLfloat> sphereVerts);
   std::vector<QVector3D> iMeshIndices;
   std::vector<QVector3D> iMeshNormal;
   std::vector<Texture*> m_Textures;

  QOpenGLBuffer mPosBuffer;
  QOpenGLBuffer mTextBuffer;
  QOpenGLBuffer mNormBuffer;

  QVector3D maxBox;
  QVector3D minBox;
  float max_x;
  float max_y;
  float max_z;

  float min_x;
  float min_y;
  float min_z;
private:
  bool bLoaded;

  

  QOpenGLVertexArrayObject m_VAO;
  //uint m_VAO;
  QOpenGLBuffer m_Buffers[3];
  
   //static std::vector<CTexture> tTextures;
   
  std::vector<int> iMaterialIndices;
  int iNumMaterials; 

  struct MeshEntry {
    MeshEntry()
    {
        NumIndices = 0;
        BaseVertex = 0;
        BaseIndex = 0;
        MaterialIndex = 0xFFFFFFFF;
    }
              
    unsigned int NumIndices;
    unsigned int BaseVertex;
    unsigned int BaseIndex;
    unsigned int MaterialIndex;
  };

  std::vector<MeshEntry> m_Entries;
    
  std::vector<QVector3D> Positions;
  std::vector<QVector3D> Normals;
  std::vector<QVector2D> TexCoords;
  std::vector<unsigned int> Indices;

};
#endif