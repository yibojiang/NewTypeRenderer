#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QPainter>
#include <QPushButton>
#include <QImage>
#include "raytracer.h"

#include <QMenu>
#include <QMenuBar>
#include <QMainWindow>
#include <QToolBar>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QStatusBar>

class Window : public QMainWindow
{
 Q_OBJECT
 public:
    
    explicit Window(QWidget *parent = 0);
    void paintEvent(QPaintEvent *event);
    int width;
    int height;
    int sample;
    QImage directImage;
    QImage indirectImage;
    QImage normalImage;
    QLabel *debugLabel;
    QStatusBar *status;

private slots:
    void render();
    void saveImage();
    void switchChannel(const QString&);
    void changeSample(const QString&);
private:
    int displayMode; // 0 - render, 1 - normal



};

// class RenderWidget : public QWidget
// {
//  Q_OBJECT
//  public:
    
//     explicit RenderWidget(QWidget *parent = 0);
//     void paintEvent(QPaintEvent *event);
//     int width;
//     int height;
//     QImage image;
//     QImage normalImage;
// private slots:
//     void handleRender();
//     void saveImage();
// private:
//     QPushButton *renderButton;
//     int displayMode; // 0 - render, 1 - normal

// };
 
#endif // WINDOW_H