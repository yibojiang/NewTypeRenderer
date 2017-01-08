
#include <QApplication>
#include "raytracer.h"
#include <string>
#include "window.h"
#include <QBuffer>
#include <QByteArray>
#include <QDebug>
#include <QFileDialog>

Window::Window(QWidget *parent) :
 QMainWindow(parent) {
    width = 800, height = 600;
    resize(width, height);

    
    
    setWindowTitle("Render View " + QString::number(width) + "x" + QString::number(height));
    // renderButton = new QPushButton("Render", this);
    displayMode = 0;
    samples = 4;
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
    sampleText->setText(QString::number(samples));
    sampleText->setMaxLength(5);
    // sampleText->setGeometry(QRect(10, 560, 200, 30));

    QLineEdit *widthText = new QLineEdit();
    widthText->setText(QString::number(width));
    widthText->setMaxLength(5);

    QLineEdit *heightText = new QLineEdit();
    heightText->setText(QString::number(height));
    heightText->setMaxLength(5);

    QComboBox *channelBox = new QComboBox;
    channelBox->addItem(tr("InDirect"));
    channelBox->addItem(tr("Normal"));
    channelBox->addItem(tr("Direct"));
    channelBox->addItem(tr("Boundingbox"));

    rgbBox = new QComboBox;
    rgbBox->addItem(tr("RGB"));
    rgbBox->addItem(tr("R"));
    rgbBox->addItem(tr("G"));
    rgbBox->addItem(tr("B"));
    rgbBox->addItem(tr("Grey"));

    QSlider *camRotateXSlider = new QSlider(this);
    camRotateXSlider->setGeometry(QRect(QPoint(0, 200), QSize(200, 50)));
    camRotateXSlider->setOrientation(Qt::Horizontal);
    // camRotateXSlider->setMinimum(0.0);
    // camRotateXSlider->setMaximum(M_PI);

    toolbar->addWidget(channelBox);
    toolbar->addWidget(rgbBox);
    toolbar->addWidget(new QLabel("Samples: ", this));
    toolbar->addWidget(sampleText);
    toolbar->addWidget(new QLabel("w: ", this));
    toolbar->addWidget(widthText);
    toolbar->addWidget(new QLabel("h", this));
    toolbar->addWidget(heightText);

    gammaCheckbox = new QCheckBox("Gamma", this);
    toolbar->addWidget(gammaCheckbox);
    gammaCheckbox->setCheckState(Qt::Checked);


    // debugLabel = new QLabel(this);
    // debugLabel->setGeometry(QRect(750, 560, 200, 30));
    // debugLabel->setText("");
    // debugLabel->setAlignment(Qt::AlignBottom | Qt::AlignLeft);

    status = new QStatusBar(this);
    setStatusBar(status);
    status->showMessage("test", 10000);
    // status->addWidget(stat0,1);

    connect(channelBox, SIGNAL(currentIndexChanged(const QString&)),
        this, SLOT(switchChannel(const QString&)));

    connect(rgbBox, SIGNAL(currentIndexChanged(const QString&)),
        this, SLOT(switchRGBChannel(const QString&)));

    connect(sampleText, SIGNAL(textEdited(const QString&)),
        this, SLOT(changeSample(const QString&)));

    connect(widthText, SIGNAL(textEdited(const QString&)),
        this, SLOT(changeResolutionWidth(const QString&)));

    connect(heightText, SIGNAL(textEdited(const QString&)),
        this, SLOT(changeResolutionHeight(const QString&)));

    connect(gammaCheckbox, SIGNAL(stateChanged(int)),
        this, SLOT(gammaState(int)));


    connect(camRotateXSlider, SIGNAL(valueChanged(int)),
        this, SLOT(camRotateY(int)));

    tracer = new Raytracer(width, height, samples);

    normalImage = QImage(width, height, QImage::Format_RGB32);
    directImage = QImage(width, height, QImage::Format_RGB32);
    indirectImage = QImage(width, height, QImage::Format_RGB32);
    boundingBoxImage = QImage(width, height, QImage::Format_RGB32);
    double directTime;
    tracer->renderDirect(directTime, directImage, normalImage, boundingBoxImage);
    
    displayMode = 2;
    update();

 }

void Window::camRotateY(int rotateY){
    // qDebug() << "camRotateY: " << rotateY;
    tracer->rotateCamera(0, rotateY * 0.02f * M_PI, 0);
    double directTime;
    tracer->renderDirect(directTime, directImage, normalImage, boundingBoxImage);
    update();
}

void Window::changeSample(const QString& _text){
    // debugLabel->setText(_text);
    samples = _text.toInt();
}

void Window::changeResolutionWidth(const QString& _text){
    width = _text.toInt();
}

void Window::changeResolutionHeight(const QString& _text){
    height = _text.toInt();
}

void Window::gammaState(int){
    postImage = postProcess(indirectImage);
    update();
}

void Window::saveImage(){

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                           "~/",
                           tr("Images (*.png)"));
    QFile file(fileName);
    file.open( QIODevice::WriteOnly );
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    postImage.save(&buffer, "PNG"); // writes image into ba in PNG format
    file.write(ba);
    file.close();
    // debugLabel->setText("saved");
    
}

void Window::switchChannel(const QString& _channel){
    // debugLabel->setText(_channel);
    if (_channel == "Direct"){
        displayMode = 2;
    }
    else if (_channel == "Normal"){
        displayMode = 1;
    }
    else if (_channel == "InDirect"){
        displayMode = 0;
    }
    else if (_channel == "Boundingbox"){
        displayMode = 3;
    }
    update();
}

void Window::switchRGBChannel(const QString&){
    postImage = postProcess(indirectImage);
    update();
}

void Window::render(){
    statusBar()->showMessage("Rendering...");
    resize(width, height);
    tracer->setResolution(width, height);
    tracer->samples = samples;
    
    double directTime;
    tracer->renderDirect(directTime, directImage, normalImage, boundingBoxImage);

    double indirectTime;
    tracer->renderIndirect(indirectTime, indirectImage);
    postImage = postProcess(indirectImage);
    update();
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

QImage Window::postProcess(const QImage &image){
    QImage reuslt = QImage(image.width(), image.height(), QImage::Format_RGB32);
    
    qDebug() << rgbBox->currentText();
    for (int i = 0; i < image.width(); ++i){
        for (int j = 0; j < image.height(); ++j){
            QColor color(image.pixel(i, j));
            int r = color.red();
            int g = color.green();
            int b = color.blue();           
            
            if (gammaCheckbox->checkState() == Qt::Checked){
                r = 255*pow(color.red()*1.0/255, 1.0/2.2);
                g = 255*pow(color.green()*1.0/255, 1.0/2.2);
                b = 255*pow(color.blue()*1.0/255, 1.0/2.2);
            }

            if (rgbBox->currentText() == "RGB"){

            }
            else if (rgbBox->currentText() == "R"){
                g = 0;
                b = 0;
            }
            else if (rgbBox->currentText() == "G"){
                r = 0;
                b = 0;   
            }
            else if (rgbBox->currentText() == "B"){
                r = 0;
                g = 0;
            }
            else if (rgbBox->currentText() == "Grey"){
                g = r;
                b = r;
            }

            reuslt.setPixel(i, j, qRgb(r, g, b));
        }
    }
    qDebug()<<"gamma correct on";

    return reuslt;
}

void Window::paintEvent(QPaintEvent *){

    QPainter painter(this);
    QRectF target(0.0, 0.0, width, height);
    QRectF source(0.0, 0.0, width, height);
    // qDebug() << "mode: " << displayMode;
    if (displayMode == 0){
        painter.drawImage(target, postImage, source);    
    }
    else if (displayMode == 1){
        painter.drawImage(target, normalImage, source);    
    }
    else if (displayMode == 2){
        painter.drawImage(target, directImage, source);   
    }
    else if (displayMode == 3){
        painter.drawImage(target, boundingBoxImage, source);   
        qDebug() << "mode: boundingBoxImage";
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