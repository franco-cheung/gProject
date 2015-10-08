#include "skybox.hpp"
#include "AppWindow.hpp"

skybox::skybox(){}

void skybox::load_skybox()
{
	glActiveTexture(GL_TEXTURE1);

	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

	const std::string suffixes[] = { "right", "back", "down", "up", "front", "left" };

	GLuint targets[] = {
		GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	};

	for( int i = 0; i < 6; i++ ) {

	 std::string texName = suffixes[i] + ".jpg";
	 QImage img = QGLWidget::convertToGLFormat(QImage(texName.c_str(),"JPG"));
	 glTexImage2D(targets[i], 0, GL_RGBA,img.width(), img.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());

	}

	// Typical cube map settings

	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// Set the CubeMapTex uniform to texture unit 0
	GLuint programHandle = 0;
	int uniloc = AppWindow::m_viewer->temp->glGetUniformLocation(programHandle, "CubeMapTex");

	if( uniloc >= 0 )
	{
		AppWindow::m_viewer->temp->glUniform1i(uniloc, 0);
	}

	setup_Skybox();
}

void skybox::setup_Skybox()
{
	for(int i = 0; i <= 40; i++)
    {
        double ring0 = M_PI * 2 * (-0.5 + (double) (i - 1) / 40);
        double z_axis0  = sin(ring0);
        double xy_axis0 =  cos(ring0);
        double ring1 = M_PI * 2 * (-0.5 + (double) i / 40);
        double z_axis1 = sin(ring1);
        double xy_axis1 = cos(ring1);

        for(int j = 0; j <= 40; j++)
        {
            double section = 2 * M_PI * (double) (j - 1) / 40;
            double x = cos(section);
            double y = sin(section);

            sphereVerts.push_back(x * xy_axis0);
            sphereVerts.push_back(y * xy_axis0);
            sphereVerts.push_back(z_axis0);     
            sphereVerts.push_back(x * xy_axis1);
            sphereVerts.push_back(y * xy_axis1);
            sphereVerts.push_back(z_axis1);
        }
    } 
    load_skyboxBuffer();
}

void skybox::load_skyboxBuffer()
{
	s_VAO.create();
	s_VAO.bind();

	mPosBuffer.create();
	mPosBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
	mPosBuffer.bind();
	mPosBuffer.allocate(&sphereVerts[0], sizeof(sphereVerts[0]) * sphereVerts.size());
	AppWindow::m_viewer->mProgram.setAttributeBuffer("mesh_pos", GL_FLOAT, 0, 3);
	AppWindow::m_viewer->mProgram.enableAttributeArray("mesh_pos");

	glActiveTexture(GL_TEXTURE1);
	AppWindow::m_viewer->mProgram.setUniformValue("CubeMapTex", 1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texID);
	s_VAO.release();
}

void skybox::renderSky()
{
	s_VAO.bind();

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 3200);
	s_VAO.release();
}