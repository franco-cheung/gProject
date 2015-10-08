#include <iostream>
#include "texture.hpp"
#include <GL/glu.h>
#include "AppWindow.hpp"
#include <QOpenGLFunctions_3_3_Core>

Texture::Texture(GLenum TextureTarget, const std::string& FileName)
{
    m_textureTarget = TextureTarget;
    m_fileName      = FileName;
}

//loads the texture file
bool Texture::Load()
{
    try {
        m_image.read(m_fileName);
        m_image.write(&m_blob, "RGBA");
    }
    catch (Magick::Error& Error) {
        std::cout << "Error loading texture '" << m_fileName << "': " << Error.what() << std::endl;
        return false;
    }

    glGenTextures(1, &m_textureObj);
    glBindTexture(m_textureTarget, m_textureObj);
    glTexImage2D(m_textureTarget, 0, GL_RGBA, m_image.columns(), m_image.rows(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_blob.data());
    glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    
    glBindTexture(m_textureTarget, 0);
    
    return true;
}

//Bind the texture to the object using the textbuffer at 0
void Texture::Bind(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(m_textureTarget, m_textureObj);
    AppWindow::m_viewer->temp->glUniform1i(AppWindow::m_viewer->mTexture, 0);
    //AppWindow::m_viewer->mProgram.uniformLocation("text_colour");
}
