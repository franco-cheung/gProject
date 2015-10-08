#include <QtWidgets>
#include <iostream>
#include "AppWindow.hpp"

Viewer * AppWindow::m_viewer = NULL;

AppWindow::AppWindow(SceneNode *n) {
    setWindowTitle("Final Project");

    QGLFormat glFormat;
    glFormat.setVersion(4,0);
    glFormat.setProfile(QGLFormat::CoreProfile);
    glFormat.setSampleBuffers(true);

    QVBoxLayout *layout = new QVBoxLayout;
    m_viewer = new Viewer(glFormat, n, this);
    layout->addWidget(m_viewer);
    setCentralWidget(new QWidget);
    centralWidget()->setLayout(layout);

    createActions();
    createMenu();
}

void AppWindow::keyPressEvent ( QKeyEvent * event) {
    //std::cerr << "Stub: button " << event->key() << " pressed\n";
    m_viewer->keyPressEvent(event);
}

void AppWindow::createActions() {
    QAction* hardAct = new QAction(tr("&Hard"), this);
    m_menu_mode_actions.push_back(hardAct); 
    hardAct->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_H));
    connect(hardAct, SIGNAL(triggered()), this, SLOT(Hard()));

    QAction* normalAct = new QAction(tr("&Normal"), this);
    m_menu_mode_actions.push_back(normalAct); 
    normalAct->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_N));
    connect(normalAct, SIGNAL(triggered()), this, SLOT(Normal()));

    QAction* easyAct = new QAction(tr("&Easy"), this);
    m_menu_mode_actions.push_back(easyAct); 
    easyAct->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_E));
    connect(easyAct, SIGNAL(triggered()), this, SLOT(Easy()));

    QAction* oneAct = new QAction(tr("&Set 1 Enemy"), this);
    m_menu_mode_set_enemy.push_back(oneAct); 
    oneAct->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_1));
    connect(oneAct, SIGNAL(triggered()), this, SLOT(set_one()));

    QAction* twoAct = new QAction(tr("&Set 2 Enemy"), this);
    m_menu_mode_set_enemy.push_back(twoAct); 
    twoAct->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_2));
    connect(twoAct, SIGNAL(triggered()), this, SLOT(set_two()));

    QAction* threeAct = new QAction(tr("&Set 3 Enemy"), this);
    m_menu_mode_set_enemy.push_back(threeAct); 
    threeAct->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_3));
    connect(threeAct, SIGNAL(triggered()), this, SLOT(set_three()));

    //start
    QAction* startAct = new QAction(tr("&Start"), this);
    m_menu_actions.push_back(startAct); 
    startAct->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_S));
    connect(startAct, SIGNAL(triggered()), this, SLOT(Start()));

    //Pause
    QAction* pauseAct = new QAction(tr("&Pause"), this);
    m_menu_actions.push_back(pauseAct); 
    pauseAct->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_P));
    connect(pauseAct, SIGNAL(triggered()), this, SLOT(Pause()));

    // Creates a new action for quiting and pushes it onto the menu actions vector 
    QAction* quitAct = new QAction(tr("&Quit"), this);
    m_menu_actions.push_back(quitAct);

    // We set the accelerator keys
    // Alternatively, you could use: setShortcuts(Qt::CTRL + Qt::Key_P); 
    quitAct->setShortcuts(QKeySequence::Quit);

    // Set the tip
    quitAct->setStatusTip(tr("Exits the file"));

    // Connect the action with the signal and slot designated
    connect(quitAct, SIGNAL(triggered()), this, SLOT(close())); 
    
     //Radio button for Mode
    QActionGroup* modeGroup = new QActionGroup(this);
    modeGroup->addAction(easyAct);
    modeGroup->addAction(normalAct);
    modeGroup->addAction(hardAct);
    modeGroup->setExclusive(true);
    easyAct->setCheckable(true);
    normalAct->setCheckable(true);
    hardAct->setCheckable(true);
    normalAct->setChecked(true);

    QActionGroup* enemyGroup = new QActionGroup(this);
    enemyGroup->addAction(oneAct);
    enemyGroup->addAction(twoAct);
    enemyGroup->addAction(threeAct);
    enemyGroup->setExclusive(true);
    oneAct->setCheckable(true);
    twoAct->setCheckable(true);
    threeAct->setCheckable(true);
    oneAct->setChecked(true);

}

void AppWindow::createMenu() {
    m_menu_app = menuBar()->addMenu(tr("&Application"));
    m_menu_mode = menuBar()->addMenu(tr("&Mode"));
    m_menu_enemy = menuBar()->addMenu(tr("&Set Enemy"));
    for (auto& action : m_menu_actions) {
        m_menu_app->addAction(action);
    }
    for (auto& action : m_menu_mode_actions) {
        m_menu_mode->addAction(action);
    }
    for (auto& action : m_menu_mode_set_enemy) {
        m_menu_enemy->addAction(action);
    }
}

void AppWindow::Normal()
{
    m_viewer->monsterTimer->stop();
    m_viewer->monsterTimer->start(150);
}

void AppWindow::Hard()
{
    m_viewer->monsterTimer->stop();
    m_viewer->monsterTimer->start(75);
}

void AppWindow::Easy()
{
    m_viewer->monsterTimer->stop();
    m_viewer->monsterTimer->start(300);
}

void AppWindow::Start()
{
   m_viewer->monsterTimer->start(150);
}

void AppWindow::Pause()
{
   m_viewer->monsterTimer->stop();
}

void AppWindow::set_one()
{
    m_viewer->set_enemy = 1;
}

void AppWindow::set_two()
{
   m_viewer->set_enemy = 2;
}

void AppWindow::set_three()
{
   m_viewer->set_enemy = 3;
}