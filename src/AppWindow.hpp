#ifndef APPWINDOW_HPP
#define APPWINDOW_HPP

#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <vector>
#include "Viewer.hpp"
#include "scene.hpp"
class AppWindow : public QMainWindow
{
    Q_OBJECT

public:
    AppWindow(SceneNode *n);
    static Viewer* m_viewer;
    void keyPressEvent(QKeyEvent * event);
private slots:
    void Normal();
    void Easy();
    void Hard();
    void Start();
    void Pause();

    void set_one();
    void set_two();
    void set_three();

private:
    void createActions();
    void createMenu();

    // Each menu itself
    QMenu* m_menu_app;
    QMenu* m_menu_mode;
    QMenu* m_menu_enemy;
    std::vector<QAction*> m_menu_actions;
    std::vector<QAction*> m_menu_mode_actions;
    std::vector<QAction*> m_menu_mode_set_enemy;
    
};

#endif