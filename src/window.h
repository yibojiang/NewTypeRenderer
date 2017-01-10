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
#include <QCheckBox>
#include <QColor>
#include <QSlider>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

class Window;
class RenderThread : public QThread {
    Q_OBJECT

public:
    Raytracer *tracer;
    Window *window;
    RenderThread(QObject *parent = 0);
    ~RenderThread();
    void setTracer(Raytracer *);
    void render();

signals:
    void renderedImage(double time, double samples, const QImage &image);
    void renderedImagePostProcess(double, double, const QImage &image);


protected:
    void run();

private:
    QMutex mutex;
    QWaitCondition condition;
    
    bool restart;
    bool abort;

    double width;
    double height;
};

class Window : public QMainWindow
{
 Q_OBJECT
 public:
    
    explicit Window(QWidget *parent = 0);
    void paintEvent(QPaintEvent *event);
    int width;
    int height;
    int samples;
    QImage directImage;
    QImage indirectImage;
    QImage normalImage;
    QImage postImage;
    QImage boundingBoxImage;
    
    QStatusBar *status;
    Raytracer *tracer;
    QCheckBox *gammaCheckbox;
    QComboBox *rgbBox;
    QAction *renderAction;

    RenderThread renderThread;

private slots:
    void render();
    void saveImage();
    void switchChannel(const QString&);
    void switchRGBChannel(const QString&);
    void changeSample(const QString&);
    void changeResolutionWidth(const QString&);
    void changeResolutionHeight(const QString&);
    void gammaState(int state);
    QImage postProcess(const QImage&);
    void updateIndirect(double, double, const QImage&);
    void updatePostProcess(double, double, const QImage&);
    void camRotateY(int);
    void updateProgress();

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