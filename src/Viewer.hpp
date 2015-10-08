#ifndef CS488_VIEWER_HPP
#define CS488_VIEWER_HPP

#include <QGLWidget>
#include <QGLShaderProgram>
#include <QMatrix4x4>
#include <QtGlobal>
#include "scene.hpp"
#if (QT_VERSION >= QT_VERSION_CHECK(5, 1, 0))
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#else 
#include <QGLBuffer>
#endif
#include "CAssimpModel.hpp"

#include "terrain.hpp"
#include "imageloader.hpp"
#include "skybox.hpp"
#include <QOpenGLFunctions_3_3_Core>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

class Viewer : public QGLWidget {
    
    Q_OBJECT

public:
    Viewer(const QGLFormat& format, SceneNode *n, QWidget *parent = 0);
    virtual ~Viewer();
    
    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    void draw_sphere(QMatrix4x4 b, int type);

    void draw_enemy(QMatrix4x4 m);
    // If you want to render a new frame, call do not call paintGL(),
    // instead, call update() to ensure that the view gets a paint 
    // event.
    QGLShaderProgram mProgram;

    int mkd;
    int mselect;
    int mks;
    int mld;
    int mlight;
    int mshiny;
    int mNorm;
    int mMode;
    int shadeToggle;
    int shadeLocation;
    bool circle;
    bool zBuffer;
    bool backface;
    bool frontface;

    int alpha;
    int drawSky;

    float colourV;

    void setZBuffer();
    void setface();

    int modelView;
    int mTexture;

    int num_particle;
    int num_enemy;
    int set_enemy;
    
    bool hero;
    bool heroMove;
    bool ball;
    bool initField;
    bool initSky;
    bool fired;
    virtual void keyPressEvent ( QKeyEvent * event );

    
    struct particle{
        QMatrix4x4 smokePos;
        float gravity;
        float smokeSize;
        float alphaSmoke;
        float initZ;
    };

    struct bullet{
        QMatrix4x4 bulletPos;
        float velocity;
        float gravity;
        bool alive;
    };

    struct monsters{
        int monsterId;
        QMatrix4x4 monsterMatrix;
        QVector3D monsterDir;
        QVector4D monsterPos;

        bool alive;
    };

    bool soundInit();
    bool load_soundFile();

    QMatrix4x4 modelMatrix;
    QMatrix4x4 ballMatrix;
    QMatrix4x4 fieldMatrix;
    QMatrix4x4 skyMatrix;

    particle initSmoke;
    QTimer* shootTimer; 
    QTimer* smokeTimer; 
    QTimer* monsterTimer; 

    bool terrainCollision(QVector4D location);
    bool enemyToEnemyCollision(monsters &m);
    bool enemyCollision(QVector4D location);
    bool humanToEnemyCollision();
    void moveEnemy(monsters &m);

    QOpenGLFunctions_3_3_Core *temp;
protected:

    // Events we implement

    // Called when GL is first initialized
    virtual void initializeGL();
    // Called when our window needs to be redrawn
    virtual void paintGL();
    // Called when the window is resized (formerly on_configure_event)
    virtual void resizeGL(int width, int height);
    // Called when a mouse button is pressed
    virtual void mousePressEvent ( QMouseEvent * event );
    // Called when a mouse button is released
    virtual void mouseReleaseEvent ( QMouseEvent * event );
    // Called when the mouse moves
    virtual void mouseMoveEvent ( QMouseEvent * event );


    void sphere_setup();
    void enemy_setup();
    void draw_model();
    void draw_field();
    void draw_sky();
    
    terrain* loadTerrain(char* filename, float height);

    bool b1;
    bool b2;
    bool b3;
    float circleData[120];
    float lastPos_x; 
    float lastPos_y;

    skybox* sky;
    terrain* field;
    void field_setup();

    std::vector<QVector3D> iFieldVertices;
    std::vector<QVector3D> iFieldNormal;
    std::vector<GLfloat> sphereVerts;

    QVector3D north;
    QVector3D east;
    QVector3D south;
    QVector3D west;

private slots:
    void shotFired();    
    void smokeIgnite();  
    void enemyMoved(); 
private:
    CAssimpModel* sphereModel;
    CAssimpModel* smokeModel;
    CAssimpModel* heroModel;
    QVector3D heroDir;
    QVector3D heroPos;

    CAssimpModel* enemyModel;
    bool enemyInit;
    int monsterCounter;

    
    //void removeBullet();
    std::vector<bullet> shots;
    std::vector<monsters> spawns;
    std::vector<particle> smokes;

    QMatrix4x4 getCameraMatrix();
    QMatrix4x4 getModelViewMatrix();
    void translateWorld(float x, float y, float z);
    void rotateWorld(float angle, float x, float y, float z);
    void scaleWorld(float x, float y, float z);
    void set_colour(const QColor& col);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 1, 0))
    QOpenGLBuffer mCircleBufferObject;
    QOpenGLVertexArrayObject mVertexArrayObject;
#else 
    QGLBuffer mCircleBufferObject;
#endif
    
    int mMvpMatrixLocation;
    int mColorLocation;
    
    QMatrix4x4 mPerspMatrix;
    QMatrix4x4 mTransformMatrix;

    SceneNode *root;
    QVector3D fVec;



};

#endif