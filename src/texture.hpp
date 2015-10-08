#ifndef CS488_TEXTURE_HPP
#define CS488_TEXTURE_HPP

#include <GL/glu.h>
#include <iostream>
#include "./ImageMagick/include/ImageMagick-6/Magick++.h"
#include <QOpenGLFunctions_3_3_Core>

class Texture
{
public:
   Texture(GLenum TextureTarget, const std::string& FileName);
   bool Load();
   void Bind(GLenum TextureUnit);

private:

	std::string m_fileName;
	GLenum m_textureTarget;
	GLuint m_textureObj;
	Magick::Image m_image;
	Magick::Blob m_blob;
}; 

#endif