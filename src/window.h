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

class RenderThread : public QThread
{
    Q_OBJECT

public:
    Raytracer *tracer;

    
    RenderThread(QObject *parent = 0);
    ~RenderThread();
    void setTracer(Raytracer *);
    void render();

signals:
    void renderedImage(const QImage &image);
    void updateProgress(const QImage &image);

protected:
    void run();

private:
    uint rgbFromWaveLength(double wave);
    QMutex mutex;
    QWaitCondition condition;
    double centerX;
    double centerY;
    double scaleFactor;
    QSize resultSize;
    bool restart;
    bool abort;

    double width;
    double height;

    enum { ColormapSize = 512 };
    uint colormap[ColormapSize];
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
    // QLabel *debugLabel;
    QStatusBar *status;
    Raytracer *tracer;
    QCheckBox *gammaCheckbox;
    QComboBox *rgbBox;

    RenderThread renderThread;

    // QSlider *camRotateXSlider;
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
    void updateIndirect(const QImage&);
    void camRotateY(int);
    void updateProgress(double);
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