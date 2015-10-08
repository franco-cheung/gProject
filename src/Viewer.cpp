#include <QtWidgets>
#include <QtOpenGL>
#include "Viewer.hpp"
#include <iostream>
#include <math.h>
#include <GL/glu.h>
#include <assimp/Importer.hpp> // C++ importer interface
#include <assimp/scene.h> // Output data structure
#include <assimp/postprocess.h> // Post processing flags
#include "CAssimpModel.hpp"
#include "skybox.hpp"
#include <vector>


#include <algorithm>
#include "imageloader.hpp"
#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif

Mix_Chunk *beam; 
Mix_Chunk *explode; 

Viewer::Viewer(const QGLFormat& format, SceneNode *n, QWidget *parent) 
    : QGLWidget(format, parent) 
#if (QT_VERSION >= QT_VERSION_CHECK(5, 1, 0))
    , mCircleBufferObject(QOpenGLBuffer::VertexBuffer)
    , mVertexArrayObject(this)
#else 
    , mCircleBufferObject(QGLBuffer::VertexBuffer)
#endif
{
    root = n;
    b1 = false;
    b2 = false;
    b3 = false;

    mTransformMatrix.setToIdentity();
    colourV = 250;
    num_enemy = 0;
    num_particle = 0;
    set_enemy = 1;

    hero = false;
    heroMove = false;
    ball = false;
    initField = false;
    fired = false;
    initSky = false;

    smokeTimer = new QTimer(this);
    connect(smokeTimer, SIGNAL(timeout()), this, SLOT(smokeIgnite()));

    shootTimer = new QTimer(this);
    connect(shootTimer, SIGNAL(timeout()), this, SLOT(shotFired()));
    shootTimer->start(0);

    monsterTimer = new QTimer(this);
    connect(monsterTimer, SIGNAL(timeout()), this, SLOT(enemyMoved()));
    

    monsterCounter = 0;
    west = QVector3D(-1, 0, 0);
    east = QVector3D(1, 0, 0);
    north = QVector3D(0, 0, -1);
    south = QVector3D(0, 0, 1);
}

Viewer::~Viewer() {
    // Nothing to do here right now.
}

QSize Viewer::minimumSizeHint() const {
    return QSize(50, 50);
}

QSize Viewer::sizeHint() const {
    return QSize(300, 600);
}

void Viewer::initializeGL() {
    QGLFormat glFormat = QGLWidget::format();
    if (!glFormat.sampleBuffers()) {
        std::cerr << "Could not enable sample buffers." << std::endl;
        return;
    }
        
    glShadeModel(GL_SMOOTH);
    glClearColor( 0.4, 0.4, 0.4, 0.0 );
    //glEnable(GL_DEPTH_TEST);

    if (!mProgram.addShaderFromSourceFile(QGLShader::Vertex, "shader.vert")) {
        std::cerr << "Cannot load vertex shader." << std::endl;
        return;
    }

    if (!mProgram.addShaderFromSourceFile(QGLShader::Fragment, "shader.frag")) {
        std::cerr << "Cannot load fragment shader." << std::endl;
        return;
    }

    if ( !mProgram.link() ) {
        std::cerr << "Cannot link shaders." << std::endl;
        return;
    }

    

    // double radius = width() < height() ? 
    //     (float)width() * 0.25 : (float)height() * 0.25;
        
    // for(size_t i=0; i<40; ++i) {
    //     circleData[i*3] = radius * cos(i*2*M_PI/40);
    //     circleData[i*3 + 1] = radius * sin(i*2*M_PI/40);
    //     circleData[i*3 + 2] = 0.0;
    // }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 1, 0))
    mVertexArrayObject.create();
    mVertexArrayObject.bind();

    mCircleBufferObject.create();
    mCircleBufferObject.setUsagePattern(QOpenGLBuffer::StaticDraw);

    //std::cerr << "vao1: " << std::endl;
#else 
    /*
     * if qt version is less than 5.1, use the following commented code
     * instead of QOpenGLVertexVufferObject. Also use QGLBuffer instead of 
     * QOpenGLBuffer.
     */
    uint vao;
     
    typedef void (APIENTRY *_glGenVertexArrays) (GLsizei, GLuint*);
    typedef void (APIENTRY *_glBindVertexArray) (GLuint);
     
    _glGenVertexArrays glGenVertexArrays;
    _glBindVertexArray glBindVertexArray;
     
    glGenVertexArrays = (_glGenVertexArrays) QGLWidget::context()->getProcAddress("glGenVertexArrays");
    glBindVertexArray = (_glBindVertexArray) QGLWidget::context()->getProcAddress("glBindVertexArray");
     
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);    

    mCircleBufferObject.create();
    mCircleBufferObject.setUsagePattern(QGLBuffer::StaticDraw);

    
    //std::cerr << "vao2: " << vao << std::endl;
#endif

    if (!mCircleBufferObject.bind()) {
        std::cerr << "could not bind vertex buffer to the context." << std::endl;
        return;
    }

    /*mCircleBufferObject.allocate(circleData, 40 * 3 * sizeof(float));
    mSBO.allocate(&sphereVerts[0], 40 * 40 * 6 * sizeof(float));*/

    mProgram.bind();

    mProgram.enableAttributeArray("vert");
    mProgram.setAttributeBuffer("vert", GL_FLOAT, 0, 3);

    

    mMvpMatrixLocation = mProgram.uniformLocation("mvpMatrix");
    mColorLocation = mProgram.uniformLocation("frag_color");

    shadeToggle = 1;
    shadeLocation = mProgram.uniformLocation("shade");

    //lighting && colour texture
    mkd = mProgram.uniformLocation("kd");
    mselect = mProgram.uniformLocation("select");
    mks = mProgram.uniformLocation("ks");
    mld = mProgram.uniformLocation("ld");
    mlight = mProgram.uniformLocation("light");
    mProgram.setUniformValue(mlight, QVector4D(50, 100, -100, 1));
    mshiny = mProgram.uniformLocation("shininess");
    mProgram.setUniformValue(mshiny, 10);
    mNorm = mProgram.uniformLocation("normal");
    mTexture = mProgram.uniformLocation("text_colour");

    alpha = mProgram.uniformLocation("alpha");
    drawSky = mProgram.uniformLocation("DrawSkyBox");
    modelView = mProgram.uniformLocation("mv");

    mProgram.setUniformValue(shadeLocation, shadeToggle);
    
    temp = new QOpenGLFunctions_3_3_Core();
    temp->initializeOpenGLFunctions();

    //sphere_setup();
    sky = new skybox();
    sky->load_skybox();

    sphereModel = new CAssimpModel();
    sphereModel->CAssimpModel::LoadModelFromFile("sphere.obj");
Mix_PlayChannel(-1, explode, 0);
    smokeModel = new CAssimpModel();
    smokeModel->CAssimpModel::LoadModelFromFile("smoke.obj");

    heroModel = new CAssimpModel();
    heroModel->CAssimpModel::LoadModelFromFile("EM-208.obj");

    enemyModel = new CAssimpModel();
    enemyModel->CAssimpModel::LoadModelFromFile("T43.obj");

    field = loadTerrain("heightmap.bmp", 15);
    field_setup();
Mix_PlayChannel(-1, explode, 0);
    enemy_setup();

    if(!soundInit())
    {
        std::cerr << "failed to make sound" << std::endl;
    }

    if(!load_soundFile())
    {
        std::cerr << "failed to load sound" << std::endl;
    }

    root->walk_gl(false);
}

void Viewer::paintGL() {
    // Clear framebuffer
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Set up lighting
    // Draw stuff
    // root->set_transform(root->get_transform());
    // root->walk_gl(false);
    // root->set_transform(root->get_inverse());
    draw_sky();

    draw_field();

    for(std::vector<bullet>::iterator it = shots.begin(); it != shots.end(); ++it)
    {
        if(it->alive == true)
        {
            //std::cerr << "bout to draw" << std::endl;

            draw_sphere(it->bulletPos, 0);
            //std::cerr << "done drawing" << std::endl;
            QVector4D tempPos = it->bulletPos.column(3);
            //td::cerr << "bullet Pos: " << tempPos.x() << " : " << tempPos.y() << " : " << tempPos.z() << std::endl;
            it->bulletPos.translate(0, -(it->gravity) , 5 - (it->velocity));
            if(it->velocity <= 5)
            {
                it->velocity += 0.05;
            }
            if(it->gravity < 2)
            {
                it->gravity += 0.05;
            }

            if(tempPos.y() < 0 || terrainCollision(tempPos) || enemyCollision(tempPos))
            {

                it->alive = false;
                //shots.erase(it);
            }
        }
    }

    draw_model();

    for(std::vector<particle>::iterator it = smokes.begin(); it != smokes.end(); ++it)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        mProgram.setUniformValue(alpha, it->alphaSmoke);
        draw_sphere(it->smokePos, 1);
        glDisable(GL_BLEND);
    }

    humanToEnemyCollision();
        
    if(num_enemy < set_enemy)
    {
        enemy_setup();
    }

    for(std::vector<monsters>::iterator it = spawns.begin(); it != spawns.end(); it++)
    {
        if(it->alive == true)
        {
            //std::cerr << "Monster Pos: " << it->monsterPos.x() << " : " << it->monsterPos.y() << " : " << it->monsterPos.z() << std::endl;
            draw_enemy(it->monsterMatrix);
        }
    }

    
    // heroMove = false;
    // removeBullet();
}

void Viewer::resizeGL(int width, int height) {
    if (height == 0) {
        height = 1;
    }

    mPerspMatrix.setToIdentity();
    mPerspMatrix.perspective(60.0, (float) width / (float) height, 0.001, 1000);

    glViewport(0, 0, width, height);    
}

void Viewer::keyPressEvent ( QKeyEvent * event) {
    //std::cerr << "Stub: button " << event->key() << " pressed\n";
    //std::cerr << "current Pos: " << heroPos.x() << " : " << heroPos.y() << " : " << heroPos.z() << std::endl;

    if (event->key() == Qt::Key_W || event->key() == Qt::Key_S || event->key() == Qt::Key_A || event->key() == Qt::Key_D)
    {
        heroMove = true;
    }

    if(heroDir == north)
    {
        if (event->key() == Qt::Key_W && (field->getHeight(heroPos.x(), heroPos.z() - 1) != -11111))
        {
            modelMatrix.translate(0, field->getHeight(heroPos.x(), heroPos.z() - 1) - heroPos.y(), 1);
            heroPos.setY(field->getHeight(heroPos.x(), heroPos.z() - 1));
            heroPos.setZ(heroPos.z() - 1);
        }
        else if (event->key() == Qt::Key_S && (field->getHeight(heroPos.x(), heroPos.z() + 1) != -11111)) 
        {
            heroDir = south;
            modelMatrix.rotate(180, 0, 1, 0);
        }
        else if (event->key() == Qt::Key_A && (field->getHeight(heroPos.x() - 1, heroPos.z()) != -11111))
        {
            heroDir = west;
            modelMatrix.rotate(90, 0, 1, 0);
        } 
        else if (event->key() == Qt::Key_D && (field->getHeight(heroPos.x() + 1, heroPos.z()) != -11111)) 
        {
            heroDir = east;
            modelMatrix.rotate(270, 0, 1, 0);
        }    
    }
    else if(heroDir == east)
    {
        if (event->key() == Qt::Key_W && (field->getHeight(heroPos.x(), heroPos.z() - 1) != -11111))
        {
            heroDir = north;
            modelMatrix.rotate(90, 0, 1, 0);
        }
        else if (event->key() == Qt::Key_S && (field->getHeight(heroPos.x(), heroPos.z() + 1) != -11111)) 
        {
            heroDir = south;
            modelMatrix.rotate(270, 0, 1, 0);
        }
        else if (event->key() == Qt::Key_A && (field->getHeight(heroPos.x() - 1, heroPos.z()) != -11111))
        {
            heroDir = west;
            modelMatrix.rotate(180, 0, 1, 0);
        } 
        else if (event->key() == Qt::Key_D && (field->getHeight(heroPos.x() + 1, heroPos.z()) != -11111)) 
        {
            modelMatrix.translate(0, field->getHeight(heroPos.x() + 1, heroPos.z()) - heroPos.y(), 1);
            heroPos.setY(field->getHeight(heroPos.x() + 1, heroPos.z()));
            heroPos.setX(heroPos.x() + 1);
        }  
    }
    else if(heroDir == south)
    {
        if (event->key() == Qt::Key_W && (field->getHeight(heroPos.x(), heroPos.z() - 1) != -11111))
        {
            heroDir = north;
            modelMatrix.rotate(180, 0, 1, 0);
        }
        else if (event->key() == Qt::Key_S && (field->getHeight(heroPos.x(), heroPos.z() + 1) != -11111)) 
        {
            modelMatrix.translate(0, field->getHeight(heroPos.x(), heroPos.z() + 1) - heroPos.y(), 1);
            heroPos.setY(field->getHeight(heroPos.x(), heroPos.z() + 1));
            heroPos.setZ(heroPos.z() + 1);
        }
        else if (event->key() == Qt::Key_A && (field->getHeight(heroPos.x() - 1, heroPos.z()) != -11111))
        {
            heroDir = west;
            modelMatrix.rotate(270, 0, 1, 0);
        } 
        else if (event->key() == Qt::Key_D && (field->getHeight(heroPos.x() + 1, heroPos.z()) != -11111)) 
        {
            heroDir = east;
            modelMatrix.rotate(90, 0, 1, 0);
        } 
    }
    else if(heroDir == west)
    {
        if (event->key() == Qt::Key_W && (field->getHeight(heroPos.x(), heroPos.z() - 1) != -11111))
        {
            heroDir = north;
            modelMatrix.rotate(270, 0, 1, 0);
        }
        else if (event->key() == Qt::Key_S && (field->getHeight(heroPos.x(), heroPos.z() + 1) != -11111)) 
        {
            heroDir = south;
            modelMatrix.rotate(90, 0, 1, 0);
        }
        else if (event->key() == Qt::Key_A && (field->getHeight(heroPos.x() - 1, heroPos.z()) != -11111))
        {
            modelMatrix.translate(0, field->getHeight(heroPos.x() - 1, heroPos.z()) - heroPos.y(), 1);
            heroPos.setY(field->getHeight(heroPos.x() - 1, heroPos.z()));
            heroPos.setX(heroPos.x() - 1);
        } 
        else if (event->key() == Qt::Key_D && (field->getHeight(heroPos.x() + 1, heroPos.z()) != -11111)) 
        {
            heroDir = east;
            modelMatrix.rotate(180, 0, 1, 0);
        } 
    }
    

    if (event->key() == Qt::Key_Space) {
        
        //std::cerr << "fire" << std::endl;
        QMatrix4x4 tempBall;
        tempBall = modelMatrix;
        tempBall.scale(0.1,0.1,0.1);
        tempBall.translate(0,25,5);

        bullet b;
        b.bulletPos = tempBall; //QVector3D();

        b.velocity = 0;
        b.gravity = 0.1;
        b.alive = true;

        shots.push_back(b);
        Mix_PlayChannel(-1, beam, 0);
    }
    update();
}

void Viewer::mousePressEvent ( QMouseEvent * event ) {
    //std::cerr << "Stub: button " << event->button() << " pressed\n";
    lastPos_x = event->x();
    lastPos_y = event->y();
    if(event->button() == 1)
    {
        b1 = true;
    }
    if(event->button() == 4)
    {
        b2 = true;
    }
    if(event->button() == 2)
    {
        b3 = true;
    }

    update();
}

void Viewer::mouseReleaseEvent ( QMouseEvent * event ) {
    //std::cerr << "Stub: button " << event->button() << " released\n";

    if(event->button() == 1)
    {
       b1 = false;
    }
    if(event->button() == 4)
    {
        b2 = false;
    }
    if(event->button() == 2)
    {
        b3 = false;
    }
    //update();   
}

void Viewer::mouseMoveEvent ( QMouseEvent * event ) {
    //std::cerr << "Stub: Motion at " << event->x() << ", " << event->y() << std::endl;
    //std::cerr << "Stub: Motion at " << event->x() << ", " << event->y() << std::endl;
    if(b1)
    {

        if(event->x() < lastPos_x)
        {
            translateWorld(0.5, 0, 0);
        }
        else if(event->x() > lastPos_x)
        {
            translateWorld(-0.5 ,0, 0);
        }
    }
    if(b2)
    {
        if(event->y() < lastPos_y)
        {
            translateWorld(0,0, -0.5);
        }
        else if(event->y() > lastPos_y)
        {
            translateWorld(0,0, 0.5);
        }
    }
    if(b3)
    {
        if(event->y() < lastPos_y)
        {
            rotateWorld(0.5, 1,0,0);
        }
        else if(event->y() > lastPos_y)
        {
            rotateWorld(-0.5, 1,0,0);
        }
    }

    lastPos_x = event->x();
    lastPos_y = event->y();
    update();
}

QMatrix4x4 Viewer::getCameraMatrix() {
    // Todo: Ask if we want to keep this.
    QMatrix4x4 vMatrix;

    QMatrix4x4 cameraTransformation;
    QVector3D cameraPosition = cameraTransformation * QVector3D(0, 0, 20.0);
    QVector3D cameraUpDirection = cameraTransformation * QVector3D(0, 1, 0);

    vMatrix.lookAt(cameraPosition, QVector3D(0, 0, 0), cameraUpDirection);
    vMatrix.translate(0,0,-5);
    return mPerspMatrix * vMatrix * mTransformMatrix;
}

QMatrix4x4 Viewer::getModelViewMatrix()
{
    QMatrix4x4 vMatrix;

    QMatrix4x4 cameraTransformation;
    QVector3D cameraPosition = cameraTransformation * QVector3D(0, 0, 20.0);
    QVector3D cameraUpDirection = cameraTransformation * QVector3D(0, 1, 0);

    vMatrix.lookAt(cameraPosition, QVector3D(0, 0, 0), cameraUpDirection);
    vMatrix.translate(0,0,-5);
    return vMatrix * mTransformMatrix;
}

void Viewer::translateWorld(float x, float y, float z) {
    // Todo: Ask if we want to keep this.
    mTransformMatrix.translate(x, y, z);
}

void Viewer::rotateWorld(float angle, float x, float y, float z) {
    // Todo: Ask if we want to keep this.
    mTransformMatrix.rotate(angle, x, y, z);
}

void Viewer::scaleWorld(float x, float y, float z) {
    // Todo: Ask if we want to keep this. mProgram.bind();

    mProgram.enableAttributeArray("vert");
    mProgram.setAttributeBuffer("vert", GL_FLOAT, 0, 3);

    mTransformMatrix.scale(x, y, z);
}

void Viewer::set_colour(const QColor& col)
{
  mProgram.setUniformValue(mColorLocation, col.red(), col.green(), col.blue());
}

void Viewer::enemy_setup()
{

    while(num_enemy < set_enemy)
    {
            QMatrix4x4 enemyMatrix;
            enemyMatrix.setToIdentity();// = getCameraMatrix();
            monsters enemy;

            int tempPosX = rand() % 50 + 5;
            int tempPosZ = rand() % 50 + 5;
            
            //enemyMatrix.translate(field->getWidth()/2, field->getHeight(field->getWidth()/2 , field->getLength()/2), field->getLength()/2); 
            // enemyMatrix.scale(0.5, 0.5, 0.5);
            enemyMatrix.translate(tempPosX, field->getHeight(tempPosX , tempPosZ), tempPosZ);
            // enemyMatrix.rotate(270, 1.0, 0.0, 0.0);

            int n = rand() % 4;
            //enemy.monsterDir = south;
            switch(n)
            {
                case 0:
                    enemy.monsterDir = north;
                    enemyMatrix.rotate(180, 0, 1, 0);
                    break;
                case 1:
                    enemy.monsterDir = east;
                    enemyMatrix.rotate(90, 0, 1, 0);
                    break;
                case 2:
                    enemy.monsterDir = south;
                    break;
                case 3:
                    enemy.monsterDir = west;
                    enemyMatrix.rotate(270, 0, 1, 0);
           
                    break;
                default:
                    break;
            }

            enemy.monsterMatrix = enemyMatrix;
            enemy.monsterPos =  enemyMatrix.column(3);
            enemy.alive = true;
            enemy.monsterId = monsterCounter;
            monsterCounter ++;

            if(spawns.size() == 0 && !humanToEnemyCollision())
            {
                spawns.push_back(enemy);
                num_enemy ++;
            }
            else if(!humanToEnemyCollision())
            {
                spawns.push_back(enemy);
                num_enemy ++;
            }
            
    }
}
void Viewer::draw_sky()
{
    if(!initSky)
    {
        skyMatrix.setToIdentity();
        skyMatrix.scale(150, 150, 150);
    }
    QMatrix4x4 mvpMatrix = getCameraMatrix() * skyMatrix;
    QMatrix4x4 mvMatrix = getModelViewMatrix();

    mProgram.setUniformValue(modelView, mvMatrix);
    mProgram.setUniformValue(mMvpMatrixLocation, mvpMatrix);
    mProgram.setUniformValue(mNorm, mvpMatrix.normalMatrix());
    mProgram.setUniformValue(drawSky, true);
    sky->renderSky();
    mProgram.setUniformValue(drawSky, false);
}

void Viewer::draw_enemy(QMatrix4x4 m)
{
    // mCircleBufferObject.allocate(&enemyModel->iMeshIndices[0], sizeof(enemyModel->iMeshIndices[0]) * enemyModel->iMeshIndices.size());

    // mCircleBufferObject.bind();

    QMatrix4x4 mvpMatrix = getCameraMatrix() * m;
    QMatrix4x4 mvMatrix = getModelViewMatrix();

    mProgram.setUniformValue(modelView, mvMatrix);
    mProgram.setUniformValue(mMvpMatrixLocation, mvpMatrix);
    mProgram.setUniformValue(mNorm, mvpMatrix.normalMatrix());
    //mProgram.setUniformValue(drawSky, false);
    enemyModel->RenderModel();
    //glDrawArrays(GL_TRIANGLES, 0, enemyModel->iMeshIndices.size());
}

void Viewer::sphere_setup()
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
    //std::cerr << "sphere size: " << sphereVerts.size() << std::endl;
}


void Viewer::draw_sphere(QMatrix4x4 b, int type)
{
    //mVertexArrayObject.bind();
    set_colour(QColor(1.0, 1.0, 1.0));
    // mCircleBufferObject.release();
    // mCircleBufferObject.allocate(&sphereVerts[0], 40 * 40 * 6 * sizeof(float));

    // mCircleBufferObject.bind();

    QMatrix4x4 mvpMatrix = getCameraMatrix() * b;
    QMatrix4x4 mvMatrix = getModelViewMatrix();

    mProgram.setUniformValue(modelView, mvMatrix);
    mProgram.setUniformValue(mMvpMatrixLocation, mvpMatrix);
    mProgram.setUniformValue(mNorm, mvpMatrix.normalMatrix());
    //mProgram.setUniformValue(drawSky, false);
    if(type == 0)
    {
        sphereModel->RenderModel();
    }
    else if(type == 1)
    {
        smokeModel->RenderModel();
    }
    
    //std::cerr << "bout to draw" << std::endl;
    //glDrawArrays(GL_TRIANGLE_STRIP, 0, 3200);
    //std::cerr << "drew" << std::endl;
    /*shadeToggle = 1;
    mProgram.setUniformValue(shadeLocation, shadeToggle);*/
    //mVertexArrayObject.release();
}

void Viewer::draw_model()
{
    set_colour(QColor(1.0, 1.0, 1.0));
    //mCircleBufferObject.release();
    //mCircleBufferObject.allocate(&heroModel->iMeshIndices[0], sizeof(heroModel->iMeshIndices[0]) * heroModel->iMeshIndices.size());

    // heroModel->Posbuffer();
    // heroModel->indbuffer();
    // heroModel->

    //mCircleBufferObject.bind();

    if(!hero)
    {
            modelMatrix.setToIdentity();// = getCameraMatrix();
            modelMatrix.translate(field->getWidth()/2, field->getHeight(field->getWidth()/2 , field->getLength()/2), field->getLength()/2);
            //modelMatrix.rotate(270, 1, 0, 0);
            //std::cerr << "height: " <<  field->getHeight(0, -11111111) << std::endl;
            heroDir = south;
            heroPos = QVector3D(field->getWidth()/2, field->getHeight(field->getWidth()/2 , field->getLength()/2), field->getLength()/2);


            QMatrix4x4 tempBall;
            tempBall = modelMatrix;
            tempBall.scale(0.17,0.17,0.17);
            tempBall.translate(0,15,-3);

            particle s;
            s.smokePos = tempBall; //QVector3D();
            int lives = rand() % 5;
            s.gravity = 0.1;
            s.smokeSize = lives;
            s.alphaSmoke = 0.4;
            s.initZ = -3;
            initSmoke = s;
            smokes.push_back(s);
            num_particle ++;
            smokeTimer->start(170);
            hero = true;
    }

    QMatrix4x4 mvpMatrix = getCameraMatrix() * modelMatrix;
    QMatrix4x4 mvMatrix = getModelViewMatrix();

    mProgram.setUniformValue(modelView, mvMatrix);
    mProgram.setUniformValue(mMvpMatrixLocation, mvpMatrix);
    mProgram.setUniformValue(mNorm, mvpMatrix.normalMatrix());
    //mProgram.setUniformValue(drawSky, false);
    heroModel->RenderModel();
    //glDrawArrays(GL_TRIANGLES, 0, heroModel->iMeshIndices.size());

}


terrain* Viewer::loadTerrain(char* filename, float height)
{
    Image* image = loadBMP(filename);
    terrain* t = new terrain(image->width, image->height);
    for(int y = 0; y < image->height; y++)
    {
        std::vector<float> v;
        for(int x = 0; x < image->width; x++)
        {
            unsigned char colour = image->pixels[3 * (y * image->width + x)];
            float tHeight = height * ((colour/255.0f) * 0.5f);
            v.push_back(tHeight);
        }
        t->setHeight(v);
    }
    delete image;
    t->computeNormal();
    return t;
}

void Viewer::field_setup()
{
    translateWorld(-(float)(field->getWidth())/2, -3.0f, -(float)(field->getLength())/2 - 15);
    rotateWorld(10, 1,0,0);
    field->field_setup();
}

void Viewer::draw_field()
{
    
    //mCircleBufferObject.release();
    //std::cerr << field->getWidth() << std::endl; 
    set_colour(QColor(0.3, 0.9, 0.0));
    // mCircleBufferObject.allocate(&iFieldVertices[0], sizeof(iFieldVertices[0]) * iFieldVertices.size());
    // mCircleBufferObject.bind();
    if(!initField)
    {
        fieldMatrix.setToIdentity();
        initField = true;
    }
    QMatrix4x4 mvpMatrix = getCameraMatrix() * fieldMatrix;
    QMatrix4x4 mvMatrix = getModelViewMatrix();

    mProgram.setUniformValue(modelView, mvMatrix);
    // mvpMatrix.rotate(48 , 1, 0, 0);
    // mvpMatrix.translate( 0, -5.0f, 0);

    mProgram.setUniformValue(mMvpMatrixLocation, mvpMatrix);
    mProgram.setUniformValue(mNorm, mvpMatrix.normalMatrix());
    //mProgram.setUniformValue(drawSky, false);
    field->Render();
    //glDrawArrays(GL_TRIANGLE_STRIP, 0, iFieldVertices.size());    
}

bool Viewer::terrainCollision(QVector4D location)
{

    float h = field->getHeight(location.x(), location.z());

    if(h >= location.y() || location.x() < 0 || location.x() > field->getWidth() || location.z() < 0 || location.z() > field->getLength())
    {
        return true;
    }

    return false;

}

void Viewer::moveEnemy(monsters &m)
{
    // std::cerr << "hero Pos: " << heroPos.x() << " : "  << heroPos.y() << " : "  << heroPos.z() << std::endl;
    // std::cerr << "enemy Pos: " << m.monsterPos.x() << " : "  << m.monsterPos.y() << " : "  << m.monsterPos.z() << std::endl;

    if(m.monsterDir == north)
    {
        if (m.monsterPos.z() > heroPos.z()) 
        {
                
                m.monsterMatrix.translate(0, field->getHeight(m.monsterPos.x(), m.monsterPos.z() - 1) - m.monsterPos.y(), 1);
                m.monsterPos.setY(field->getHeight(m.monsterPos.x(), m.monsterPos.z() - 1));
                m.monsterPos.setZ(m.monsterPos.z() - 1);
        }
        else if (m.monsterPos.z() < heroPos.z() ) 
        {
                m.monsterDir = south;
                m.monsterMatrix.rotate(180, 0, 1, 0);
        }
        else if (m.monsterPos.x() < heroPos.x()) 
        {
                m.monsterDir = east;
                m.monsterMatrix.rotate(270, 0, 1, 0);
        }
        else if (m.monsterPos.x() > heroPos.x()) 
        {
                m.monsterDir = west;
                m.monsterMatrix.rotate(90, 0, 1, 0);
        }
    }
    if(m.monsterDir == east)
    {

        if (m.monsterPos.x() < heroPos.x()) 
        {
                m.monsterMatrix.translate(0, field->getHeight(m.monsterPos.x() + 1, m.monsterPos.z()) - m.monsterPos.y(), 1);
                m.monsterPos.setY(field->getHeight(m.monsterPos.x() + 1, m.monsterPos.z()));
                m.monsterPos.setX(m.monsterPos.x() + 1);
        }
        else if (m.monsterPos.z() < heroPos.z() ) 
        {
                m.monsterDir = south;
                m.monsterMatrix.rotate(270, 0, 1, 0);
        }
        else if (m.monsterPos.z() > heroPos.z()) 
        {
                m.monsterDir = north;
                m.monsterMatrix.rotate(90, 0, 1, 0);
        }
        else if (m.monsterPos.x() > heroPos.x()) 
        {
                m.monsterDir = west;
                m.monsterMatrix.rotate(180, 0, 1, 0);
        }
    }
    if(m.monsterDir == south)
    {


        if (m.monsterPos.z() < heroPos.z()) 
        {
            
                m.monsterMatrix.translate(0, field->getHeight(m.monsterPos.x(), m.monsterPos.z()  + 1) - m.monsterPos.y(), 1);
                m.monsterPos.setY(field->getHeight(m.monsterPos.x(), m.monsterPos.z() + 1));
                m.monsterPos.setZ(m.monsterPos.z() + 1);
        }
        else if (m.monsterPos.x() < heroPos.x()) 
        {
                m.monsterDir = east;
                m.monsterMatrix.rotate(90, 0, 1, 0);
        }
        else if (m.monsterPos.z() > heroPos.z()) 
        {
                m.monsterDir = north;
                m.monsterMatrix.rotate(180, 0, 1, 0);
        }
        else if (m.monsterPos.x() > heroPos.x()) 
        {
                m.monsterDir = west;
                m.monsterMatrix.rotate(270, 0, 1, 0);
        }
    }
    if(m.monsterDir == west)
    {
        if (m.monsterPos.x() > heroPos.x()) 
        {
                m.monsterMatrix.translate(0, field->getHeight(m.monsterPos.x() - 1, m.monsterPos.z()) - m.monsterPos.y(), 1);
                m.monsterPos.setY(field->getHeight(m.monsterPos.x() - 1, m.monsterPos.z()));
                m.monsterPos.setX(m.monsterPos.x() - 1);
        }
        else if (m.monsterPos.x() < heroPos.x() && (field->getHeight(m.monsterPos.x() + 1, m.monsterPos.z()) != -11111)) 
        {
                m.monsterDir = east;
                m.monsterMatrix.rotate(180, 0, 1, 0);
        }
        else if (m.monsterPos.z() < heroPos.z() ) 
        {
                m.monsterDir = south;
                m.monsterMatrix.rotate(90, 0, 1, 0);
        }
        else if (m.monsterPos.z() > heroPos.z()) 
        {
                m.monsterDir = north;
                m.monsterMatrix.rotate(270, 0, 1, 0);
        }
    }
    update();
}

bool Viewer::enemyToEnemyCollision(monsters &m)
{
    // std::vector<bullet> temp = shots;
    // int index = 0;
    for(std::vector<monsters>::iterator it = spawns.begin(); it != spawns.end(); ++it)
    {
        if(it->alive == true)
        {
            if(it->monsterId != m.monsterId)
            {
                if(m.monsterPos.x()  == it->monsterPos.x() || m.monsterPos.z() == it->monsterPos.z())
                {
                    return true;
                }
            }

        }
    }

    return false;
}

bool Viewer::humanToEnemyCollision()
{
    for(std::vector<monsters>::iterator it = spawns.begin(); it != spawns.end(); ++it)
    {
        if(it->alive)
        {
            QVector3D monsterMaxBound = it->monsterMatrix * enemyModel->maxBox;
            QVector3D monsterMinBound = it->monsterMatrix * enemyModel->minBox;

            QVector3D modelMaxBound = modelMatrix * heroModel->maxBox;
            QVector3D modelMinBound = modelMatrix * heroModel->minBox;
            // std::cerr << "bulletx: " << location.x() << " bullety: " << location.y() << " bulletz: " << location.z() << std::endl; 
            // std::cerr << "Mon max_x: " << monsterMaxBound.x() << " Mon max_y: " << monsterMaxBound.y() << " Mon max_z: " << monsterMaxBound.z() << std::endl; 
                // std::cerr << "Mon min_x: " << monsterMinBound.x() << " Mon min_y: " << monsterMinBound.y() << " Mon min_z: " << monsterMinBound.z() << std::endl; 

                // std::cerr << "robo max_x: " << modelMaxBound.x() << " robo max_y: " << modelMaxBound.y() << " robo max_z: " << modelMaxBound.z() << std::endl; 
                // std::cerr << "robo min_x: " << modelMinBound.x() << " robo min_y: " << modelMinBound.y() << " robo min_z: " << modelMinBound.z() << std::endl; 
           
            if(it->monsterDir == north)
            {
                if(monsterMaxBound.x() < modelMaxBound.x() && monsterMinBound.x() > modelMaxBound.x() && monsterMaxBound.x() < modelMinBound.x() && monsterMinBound.x() > modelMinBound.x()
                    && monsterMaxBound.z() < modelMaxBound.z() && monsterMinBound.z() > modelMaxBound.z() && monsterMaxBound.z() < modelMinBound.z() && monsterMinBound.z() > modelMinBound.z())
                {
                    monsterTimer->stop();
                    return true;
                }
                
            }
            else if(it->monsterDir == south)
            {
                if(monsterMaxBound.x() > modelMaxBound.x() && monsterMinBound.x() < modelMaxBound.x() && monsterMaxBound.x() > modelMinBound.x() && monsterMinBound.x() < modelMinBound.x()
                    && monsterMaxBound.z() > modelMaxBound.z() && monsterMinBound.z() < modelMaxBound.z() && monsterMaxBound.z() > modelMinBound.z() && monsterMinBound.z() < modelMinBound.z())
                {
                    monsterTimer->stop();
                    return true;
                }
                
            }
            else if(it->monsterDir == west)
            {
                if(monsterMaxBound.x() < modelMaxBound.x() && monsterMinBound.x() > modelMaxBound.x() && monsterMaxBound.x() < modelMinBound.x() && monsterMinBound.x() > modelMinBound.x()
                    && monsterMaxBound.z() > modelMaxBound.z() && monsterMinBound.z() < modelMaxBound.z() && monsterMaxBound.z() > modelMinBound.z() && monsterMinBound.z() < modelMinBound.z())
                {
                    monsterTimer->stop();
                    return true;
                }
                
            }
            else if(it->monsterDir == east)
            {

                if(monsterMaxBound.x() > modelMaxBound.x() && monsterMinBound.x() < modelMaxBound.x() && monsterMaxBound.x() > modelMinBound.x() && monsterMinBound.x() < modelMinBound.x()
                    && monsterMaxBound.z() < modelMaxBound.z() && monsterMinBound.z() > modelMaxBound.z() && monsterMaxBound.z() < modelMinBound.z() && monsterMinBound.z() > modelMinBound.z())
                {
                    //std::cerr << "caught west && east" << std::endl;
                    //monsterTimer->stop();
                    return true;
                }
            }
        }
    }
    return false;
}

bool Viewer::enemyCollision(QVector4D location)
{
    

    for(std::vector<monsters>::iterator it = spawns.begin(); it != spawns.end(); ++it)
    {
        if(it->alive)
        {
            QVector3D maxBound = it->monsterMatrix * enemyModel->maxBox;
            QVector3D minBound = it->monsterMatrix * enemyModel->minBox;
            // std::cerr << "bulletx: " << location.x() << " bullety: " << location.y() << " bulletz: " << location.z() << std::endl; 
                //     std::cerr << "max_x: " << maxBound.x() << " max_y: " << maxBound.y() << " max_z: " << maxBound.z() << std::endl; 
                //     std::cerr << "min_x: " << minBound.x() << " min_y: " << minBound.y() << " min_z: " << minBound.z() << std::endl; 
            
            if(it->monsterDir == south)
            {
                if(location.x()  < maxBound.x() && location.x() > minBound.x() 
                    && location.y() < maxBound.y() && location.y() > minBound.y()
                    && location.z() < maxBound.z() && location.z() > minBound.z())
                {
                    //std::cerr << "shot south" << std::endl;
                    Mix_PlayChannel(-1, explode, 0);
                    it->alive = false;
                    num_enemy--;
                    return true;
                }
            }
            else if(it->monsterDir == east)
            {
                if(location.x()  < maxBound.x() && location.x() > minBound.x() 
                    && location.y() < maxBound.y() && location.y() > minBound.y()
                    && location.z() > maxBound.z() && location.z() < minBound.z())
                {
                    //std::cerr << "shot east" << std::endl;
                    Mix_PlayChannel(-1, explode, 0);
                    it->alive = false;
                    num_enemy--;
                    return true;
                }
            }
            else if(it->monsterDir == west)
            {
                if(location.x()  > maxBound.x() && location.x() < minBound.x() 
                    && location.y() < maxBound.y() && location.y() > minBound.y()
                    && location.z() < maxBound.z() && location.z() > minBound.z())
                {
                    //std::cerr << "shot west" << std::endl;
                    Mix_PlayChannel(-1, explode, 0);
                    it->alive = false;
                    num_enemy--;
                    return true;
                }
                
            }
            else if(it->monsterDir == north)
            {
                if(location.x()  > maxBound.x() && location.x() < minBound.x() 
                    && location.y() < maxBound.y() && location.y() > minBound.y()
                    && location.z() > maxBound.z() && location.z() < minBound.z())
                {
                    //std::cerr << "shot north" << std::endl;
                    Mix_PlayChannel(-1, explode, 0);
                    it->alive = false;
                    num_enemy--;
                    return true;
                }
                    
            }
        }
    }
    return false;
}

void Viewer::shotFired(){
    update();
}

void Viewer::smokeIgnite(){
    for(std::vector<particle>::iterator it = smokes.begin(); it != smokes.end(); ++it)
    {
        //draw_sphere(it->smokePos);
        it->smokePos.scale(0.5,0.5,0.5);
        it->smokePos.translate(0, 0 , -(it->gravity) - 5);
        it->gravity = it->gravity + 0.5;
        it->smokeSize = it->smokeSize - 0.5;
        it->alphaSmoke = it->alphaSmoke - 0.05;
        if(it->smokeSize <= 0)
        {
            it->smokePos = modelMatrix;
            it->smokePos.scale(0.17,0.17,0.17);
            it->smokePos.translate(0,15, it->initZ);
            it->gravity = 0.5;
            int random_lives = rand() % 5;
            it->alphaSmoke = 0.4;
            it->smokeSize = random_lives;
        }
    }

    if(num_particle < 7)
    {
        int posY = rand() % 5;
        int lives = rand() % 5;
        initSmoke.smokePos.translate(0,0, initSmoke.initZ - (posY * 0.05)  );
        initSmoke.alphaSmoke = 0.4;
        initSmoke.initZ = initSmoke.initZ - (posY * 0.05);
        initSmoke.smokeSize = lives;
        smokes.push_back(initSmoke);
        num_particle ++;
    }
    update();
}

void Viewer::enemyMoved()
{
    for(std::vector<monsters>::iterator it = spawns.begin(); it != spawns.end(); ++it)
    {
        if(it->alive == true)
        {
            moveEnemy(*it);
        }
    }
    update();
}

bool Viewer::soundInit()
{
    if( SDL_Init( SDL_INIT_EVERYTHING ) == -1)
    {
        return false;
    }

    if(Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
    {
        return false;
    }

    return true;
}

bool Viewer::load_soundFile()
{
    beam = Mix_LoadWAV("./pew.wav");
    explode = Mix_LoadWAV("./explode.wav");
    if(beam == NULL || explode == NULL)
    {
        return false;
    }

    return true;
}