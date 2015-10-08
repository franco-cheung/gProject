#include "CAssimpModel.hpp"
#include <vector>
#include <QVector3D>
#include <QVector2D>
#include <iostream>
#include "texture.hpp"
#include "AppWindow.hpp"
#include <QOpenGLFunctions_3_3_Core>

CAssimpModel::CAssimpModel(){

}

bool CAssimpModel::LoadModelFromFile(std::string sFilePath)
{
   iMeshIndices.clear();
   iMeshNormal.clear();
   max_x = -0xfffffff;
   max_y = -0xfffffff;
   max_z = -0xfffffff;

   min_x = 0xfffffff;
   min_y = 0xfffffff;
   min_z  = 0xfffffff;

   Assimp::Importer importer;
   const aiScene* scene = importer.ReadFile( sFilePath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);
      // aiProcess_CalcTangentSpace       | 
      // aiProcess_Triangulate            |
      // aiProcess_JoinIdenticalVertices  |
      // aiProcess_SortByPType);

   if(!scene)
   {
      // printf("Error parsing '%s': '%s'\n", sFilePath, importer.GetErrorString());
      std::cout << "Error parsing " << sFilePath << " : " << importer.GetErrorString() << std::endl;
      return false;
   }

   m_Entries.resize(scene->mNumMeshes);
   m_Textures.resize(scene->mNumMaterials);
   //std::cout << "m_Entries size: " << m_Entries.size() << " scene num: " << scene->mNumMeshes << std::endl;

   unsigned int NumVertices = 0;
   unsigned int NumIndices = 0;
    
    // Count the number of vertices and indices
   for (unsigned int i = 0 ; i < m_Entries.size() ; i++) {
      m_Entries[i].MaterialIndex = scene->mMeshes[i]->mMaterialIndex;        
      m_Entries[i].NumIndices = scene->mMeshes[i]->mNumFaces * 3;
      m_Entries[i].BaseVertex = NumVertices;
      m_Entries[i].BaseIndex = NumIndices;
        
      NumVertices += scene->mMeshes[i]->mNumVertices;
      NumIndices  += m_Entries[i].NumIndices;
   }
   //std::cerr << "num vert: " << NumVertices << std::endl;
   // Reserve space in the vectors for the vertex attributes and indices
   Positions.reserve(NumVertices);
   Normals.reserve(NumVertices);
   TexCoords.reserve(NumVertices);
   Indices.reserve(NumIndices);


   for(unsigned int i = 0; i < m_Entries.size(); i++)
   {
      aiMesh* mesh = scene->mMeshes[i];
      //iMaterialIndices.push_back(mesh->mMaterialIndex);
      //std::vector<Vertex> Vertices;
      //std::vector<unsigned int> Indices;
      aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
      for(unsigned int q = 0; q < mesh->mNumVertices; q++)
      {
         aiVector3D* pPos = &(mesh->mVertices[q]);
         aiVector3D* uv;

         if(pPos->x > max_x)
         {
            max_x = pPos->x;
         }
         if(pPos->y > max_y)
         {
            max_y = pPos->y;
         }
         if(pPos->z > max_z)
         {
            max_z = pPos->z;
         }

         if(pPos->x < min_x)
         {
            min_x = pPos->x;
         }
         if(pPos->y < min_y)
         {
            min_y = pPos->y;
         }
         if(pPos->z < min_z)
         {
            min_z = pPos->z;
         }

         if(mesh->HasTextureCoords(0))
         {
           uv = &(mesh->mTextureCoords[0][q]);
         }
         else
         {
            uv = &Zero3D;
         }
         aiVector3D* pNormal = &(mesh->mNormals[q]);

         Positions.push_back(QVector3D(pPos->x, pPos->y, pPos->z));
         Normals.push_back(QVector3D(pNormal->x, pNormal->y, pNormal->z));
         TexCoords.push_back(QVector2D(uv->x, uv->y));

         // Vertex v(QVector3D(pPos->x, pPos->y, pPos->z),
         //        QVector2D(uv->x, uv->y),
         //        QVector3D(pNormal->x, pNormal->y, pNormal->z));
         // Vertices.push_back(v);
      }

      for(unsigned int j = 0; j < mesh->mNumFaces; j++)
      {
         const aiFace& face = mesh->mFaces[j];
         for(int k = 0; k < 3; k++)
         {
            // iMeshIndices.push_back(Vertices[face.mIndices[k]].m_pos);
            // iMeshNormal.push_back(Vertices[face.mIndices[k]].m_normal);
            //std::cerr << "hi" << std::endl;
            Indices.push_back(face.mIndices[k]);
         }
      }
      //m_Entries[i].Init(Vertices, Indices);
   }

   if (!InitMaterials(scene, sFilePath)) {
        return false;
   }

   maxBox = QVector3D(max_x, max_y, max_z);
   minBox = QVector3D(min_x, min_y, min_z);
   LoadBuffers();

   return true;
}

bool CAssimpModel::InitMaterials(const aiScene* scene, std::string& sFilePath)
{
   std::string::size_type SlashIndex = sFilePath.find_last_of("/");
   std::string Dir;

   if (SlashIndex == std::string::npos) {
      Dir = ".";
   }
   else if (SlashIndex == 0) {
      Dir = "/";
   }
   else {
      Dir = sFilePath.substr(0, SlashIndex);
   }

   bool Ret = true;

   for(unsigned int i = 0; i < scene->mNumMaterials; i++)
   {
      const aiMaterial* pMaterial = scene->mMaterials[i];
      m_Textures[i] = NULL;
      if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) 
      {
         aiString Path;
         if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            std::string FullPath = Dir + "/" + Path.data;
            m_Textures[i] = new Texture(GL_TEXTURE_2D, FullPath.c_str());

             if (!m_Textures[i]->Load()) {
               printf("Error loading texture '%s'\n", FullPath.c_str());
               delete m_Textures[i];
               m_Textures[i] = NULL;
               Ret = false;
             }
         }
      }

      if (!m_Textures[i]) 
      {
         //std::cerr << "this: " << sFilePath << std::endl;
         m_Textures[i] = new Texture(GL_TEXTURE_2D, "./white.png");
         Ret = m_Textures[i]->Load();
      }

      if(sFilePath == "T43.obj")
      {
         m_Textures[i] = new Texture(GL_TEXTURE_2D, "./tankText.png");
         Ret = m_Textures[i]->Load();
      }
   }

   return Ret;
}

void CAssimpModel::LoadBuffers()
{
   m_VAO.create();
   m_VAO.bind();

   mPosBuffer.create();
   mPosBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
   mPosBuffer.bind();
   mPosBuffer.allocate(&Positions[0], sizeof(Positions[0]) * Positions.size());
   AppWindow::m_viewer->mProgram.setAttributeBuffer("mesh_pos", GL_FLOAT, 0, 3);
   AppWindow::m_viewer->mProgram.enableAttributeArray("mesh_pos");

   mTextBuffer.create();
   mTextBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
   mTextBuffer.bind();
   mTextBuffer.allocate(&TexCoords[0], sizeof(TexCoords[0]) * TexCoords.size());
   AppWindow::m_viewer->mProgram.setAttributeBuffer("mesh_text", GL_FLOAT, 0, 2);
   AppWindow::m_viewer->mProgram.enableAttributeArray("mesh_text");

   mNormBuffer.create();
   mNormBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
   mNormBuffer.bind();
   mNormBuffer.allocate(&Normals[0], sizeof(Normals[0]) * Normals.size());
   AppWindow::m_viewer->mProgram.setAttributeBuffer("mesh_norms", GL_FLOAT, 0, 3);
   AppWindow::m_viewer->mProgram.enableAttributeArray("mesh_norms");

   // for(unsigned int i = 0; i > m_Entries.size(); i++)
   // {
   //    unsigned int index = m_Entries[i].MaterialIndex;
   //    if(m_Textures[index])
   //    {
   //       m_Textures[index]->Load();
   //    }
   // }

   m_VAO.release();
}


void CAssimpModel::RenderModel()
{
   m_VAO.bind();
   //std::cerr << "Entries: " << m_Entries.size() << " texture: " << m_Textures.size() << std::endl;
   //std::cerr << "Pos: " << Positions.size() << " normals: " << Normals.size() << " Indices: " << Indices.size() << " texture: " << TexCoords.size()<< std::endl;

   for (unsigned int i = 0 ; i < m_Entries.size() ; i++) {
         const unsigned int MaterialIndex = m_Entries[i].MaterialIndex;

         assert(MaterialIndex < m_Textures.size());
         if (m_Textures[MaterialIndex]) 
         {
            //std::cerr << "binds " << std::endl;
            m_Textures[MaterialIndex]->Bind(GL_TEXTURE0);
         }

         //glDrawElements(GL_TRIANGLES, m_Entries[i].NumIndices, GL_UNSIGNED_INT, 0);
         AppWindow::m_viewer->temp->glDrawElementsBaseVertex(GL_TRIANGLES, m_Entries[i].NumIndices, GL_UNSIGNED_INT, &Indices[m_Entries[i].BaseIndex], m_Entries[i].BaseVertex);
   }

   // Make sure the VAO is not changed from the outside
   m_VAO.release();
}