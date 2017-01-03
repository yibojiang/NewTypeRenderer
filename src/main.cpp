
// #include <stdlib.h> 
// #include <stdio.h> 
// #include <iostream>
// #include <vector>
#include <QApplication>
#include "raytracer.h"
#include <string>
// #include <QtGui>
// #include <random>
// #include <cmath>
// #include <vector>
// using namespace std;

#include "window.h"
#include <QBuffer>
#include <QByteArray>
#include <QDebug>

Window::Window(QWidget *parent) :
 QMainWindow(parent) {
    width = 800, height = 600;
    resize(width, height);
    
    setWindowTitle("Render View " + QString::number(width) + "x" + QString::number(height));
    // renderButton = new QPushButton("Render", this);
    displayMode = 2;
    sample = 4;
    // set size and location of the button
    // renderButton->setGeometry(QRect(QPoint(0, 0),
    // QSize(200, 50)));
 
    // Connect button signal to appropriate slot
    // connect(renderButton, SIGNAL (released()), this, SLOT (handleRender()));

    QAction *save = new QAction("&Save", this);

    QMenu *file;
    file = menuBar()->addMenu("&File");
    file->addAction(save);

    connect(save, SIGNAL(triggered()), this, SLOT(saveImage()));


    QToolBar *toolbar = addToolBar("main toolbar");
    QPixmap renderpix("new.png");
    // QPixmap openpix("open.png");
    QPixmap quitpix("quit.png");

    
    // toolbar->addAction(QIcon(openpix), " File");
    toolbar->addSeparator();
    QAction *render = toolbar->addAction(QIcon(renderpix), "Render" );
    // QAction *quit = toolbar->addAction(QIcon(quitpix), "Quit Application");


    
    connect(render, SIGNAL(triggered()), this, SLOT(render()));
    // connect(quit, SIGNAL(triggered()), qApp, SLOT(quit()));

    QLineEdit *sampleText = new QLineEdit();
    sampleText->setText(QString::number(sample));
    sampleText->setMaxLength(5);
    // textEdit->setGeometry(QRect(10, 560, 200, 30))

    QComboBox *channelBox = new QComboBox;
    channelBox->addItem(tr("InDirect"));
    channelBox->addItem(tr("Normal"));
    channelBox->addItem(tr("Direct"));
    

    toolbar->addWidget(channelBox);
    toolbar->addWidget(sampleText);

    debugLabel = new QLabel(this);
    debugLabel->setGeometry(QRect(750, 560, 200, 30));
    debugLabel->setText("");
    debugLabel->setAlignment(Qt::AlignBottom | Qt::AlignLeft);

    status = new QStatusBar(this);
    setStatusBar(status);
    status->showMessage("test", 10000);
    // status->addWidget(stat0,1);

    connect(channelBox, SIGNAL(currentIndexChanged(const QString&)),
        this, SLOT(switchChannel(const QString&)));

    connect(sampleText, SIGNAL(textEdited(const QString&)),
        this, SLOT(changeSample(const QString&)));
    

 }
void Window::changeSample(const QString& _text){
    debugLabel->setText(_text);
    sample = _text.toInt();
}

void Window::saveImage(){
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    indirectImage.save(&buffer, "PNG"); // writes image into ba in PNG format
    debugLabel->setText("saved");
}

void Window::switchChannel(const QString& _channel){
    debugLabel->setText(_channel);
    if (_channel == "Direct"){
        displayMode = 2;
    }
    else if (_channel == "Normal"){
        displayMode = 1;
    }
    else if (_channel == "InDirect"){
        displayMode = 0;
    }

    update();
}

void Window::render(){
    statusBar()->showMessage("Rendering...");
    QPainter painter(this);
    Raytracer tracer(width, height, sample);
    double indirectTime;
    indirectImage = tracer.render(indirectTime);

    double directTime;
    normalImage = QImage(width, height, QImage::Format_RGB32);
    directImage = QImage(width, height, QImage::Format_RGB32);
    tracer.renderDirect(directTime, directImage, normalImage);
    // normalImage.fill(qRgb(255, 0, 0));

    // renderButton->setText("Render Time: " + QString::number(time));
    
    
    update();
    // debugLabel->setText("time: " + QString::number(indirectTime));
    statusBar()->showMessage("time: " + QString::number(indirectTime));
    

    // #pragma omp parallel for schedule(dynamic, 1)        // OpenMP
    // for (unsigned short i = 0; i < height; ++i){
    //     unsigned short Xi[3] = {0, 0, i*i * i};
    //     for (unsigned short j = 0; j < width; ++j){
    //         vec3 color = tracer.render_pixel(i, j, Xi);
    //         painter.setPen(QColor(color.x, color.y, color.z));
    //         painter.drawPoint(j, i);
    //     }
    // }
}

void Window::paintEvent(QPaintEvent *){

    QPainter painter(this);
    QRectF target(0.0, 0.0, 800.0, 600.0);
    QRectF source(0.0, 0.0, 800.0, 600.0);
    if (displayMode == 0){
        painter.drawImage(target, indirectImage, source);    
    }
    else if (displayMode == 1){
        painter.drawImage(target, normalImage, source);    
    }
    else if (displayMode == 2){
        painter.drawImage(target, directImage, source);   
    }
    
    // if (images){
    //     for (unsigned short i = 0; i < height; ++i){
    //         // unsigned short Xi[3] = {0, 0, i*i * i};
    //         for (unsigned short j = 0; j < width; ++j){
    //             // tracer.render_pixel(i, j, Xi);
    //             int idx = i*width+j;
    //             painter.setPen(QColor(images[idx].x, images[idx].y, images[idx].z));
    //             painter.drawPoint(j, i);
    //         }
    //     }
    // }

    // delete images;
}

int main(int argc, char *argv[]) {

    
    // Raytracer tracer;
    // vec3* images = tracer.render(4);
    // FILE *f = fopen("test_image.ppm", "w");         // Write image to PPM file.
    // int w = 800, h = 600;
    // fprintf(f, "P3\n%d %d\n%d\n", w, h, 255);
    // for (int i = 0; i < w * h; i++)
    //     fprintf(f, "%d %d %d ", int(images[i].x), int(images[i].y), int(images[i].z));   

    QApplication app (argc, argv);
    Window window;
    
    window.show();
    return app.exec();
}