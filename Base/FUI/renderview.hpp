#ifndef RENDERVIEW_HPP
#define RENDERVIEW_HPP

#include <QMainWindow>
#include <QWidget>
#include <QDockWidget>
#include <QOpenGLWidget>
#include <QToolBar>
#include <QFrame>
#include <QPalette>
#include <QString>

#include <memory>
#include <iostream>

#include "fluoglwidget.hpp"
#include "testTriangle.hpp"
#include "renderviewLeftTB.hpp"
#include "renderviewRightTB.hpp"
#include "renderviewBottomTB.hpp"
#include "renderviewTopTB.hpp"

class RenderView : public QMainWindow
{
    Q_OBJECT

  public:
    RenderView(QWidget *parent = nullptr, bool hasFeatures = true);
    RenderView(bool hasFeatures) : RenderView(nullptr,hasFeatures) {}

    bool getMainWindowStatus();
    void updateID(int i);
    int getId() const { return id; }

  private:
    bool isMainWindow = false;
    LeftToolbar* leftToolBar = new LeftToolbar();
    TopToolbar* topToolBar = new TopToolbar();
    RightToolbar* rightToolBar = new RightToolbar();
    BottomToolbar* bottomToolBar = new BottomToolbar();
    QDockWidget* baseDockWidget = new QDockWidget();

    int id;
    const int samples = 16;

};

#endif // RENDERVIEW_HPP
