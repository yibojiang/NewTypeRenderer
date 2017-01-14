
#include <QApplication>
#include "raytracer.h"
#include <string>
#include "window.h"
#include <QBuffer>
#include <QByteArray>
#include <QDebug>
#include <QFileDialog>
#include <QTimer>



RenderThread::RenderThread(QObject *parent)
    : QThread(parent)
{
    restart = false;
    abort = false;
}

void RenderThread::setTracer(Raytracer *tracer){
    this->tracer = tracer;
    
}

RenderThread::~RenderThread()
{
    mutex.lock();
    abort = true;
    condition.wakeOne();
    mutex.unlock();
    // terminate();
    wait();
}

void RenderThread::run()
{
    while(!abort) {
        mutex.lock();
        int width = tracer->width;
        int height = tracer->height;
        QImage image(width, height, QImage::Format_RGB32);
        mutex.unlock();

        double indirectTime = 0;

        if (restart){
            break;
        }

        if (abort){
            return;
        }

        // window->renderAction->setEnabled(false);
        // ((Window*)parentWidget())->render -> setEnabled(false); 
        // tracer->renderIndirect(indirectTime, image);        
        int samples = tracer->samples;
        vec3 *colorArray = new vec3[width*height];
        

        struct timeval start, end;
        gettimeofday(&start, NULL);

        tracer->isRendering = true;
        for (int  s = 0; s < samples; ++s) {
            qDebug() << "samples: " << s;
            qDebug() << "before color: " << colorArray[0];
            tracer->curSamples = s + 1;
            tracer->renderIndirectProgressive(colorArray, s);
            
            if (abort){
                return;
            }

            if (restart){
                break;
            }

            for (int i = 0; i < height; ++i){
                for (int j = 0; j < width; ++j){
                    vec3 c = colorArray[i*width+j] * 255;
                    image.setPixel(j, i, qRgb(c.x, c.y, c.z));

                }
                emit renderedImage(indirectTime, s, image); 
            }   
            
        }

        
        
        gettimeofday(&end, NULL);
        indirectTime = ((end.tv_sec  - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;
        // qDebug() << "Render time: " << indirectTime;

        tracer->isRendering = false;
        if (!restart){
            // window->renderAction->setEnabled(true);
            qDebug()<<"done.";
            // emit renderedImage(indirectTime, samples, image);
            emit renderedImagePostProcess(indirectTime, samples, image);
        }

        mutex.lock();
        if (!restart)
            condition.wait(&mutex);
        restart = false;
        mutex.unlock();

    }
}

void RenderThread::render()
{
    QMutexLocker locker(&mutex);


    if (!isRunning()) {
        start(LowPriority);
    } 
    else {
        restart = true;
        condition.wakeOne();
    }

    
}

Window::Window(QWidget *parent) :
 QMainWindow(parent) {
    width = 800, height = 600;
    resize(width, height);

    setWindowTitle("Render View " + QString::number(width) + "x" + QString::number(height));
    displayMode = 0;
    samples = 4;
    QAction *open = new QAction("&Open", this);
    QAction *save = new QAction("&Save", this);

    QMenu *file;
    file = menuBar()->addMenu("&File");
    file->addAction(save);
    file->addAction(open);

    connect(save, SIGNAL(triggered()), this, SLOT(saveImage()));
    connect(open, SIGNAL(triggered()), this, SLOT(openScene()));

    

    QToolBar *toolbar = addToolBar("main toolbar");
    QPixmap renderpix("new.png");
    // QPixmap openpix("open.png");
    QPixmap quitpix("quit.png");

    
    // toolbar->addAction(QIcon(openpix), " File");
    toolbar->addSeparator();
    renderAction = toolbar->addAction(QIcon(renderpix), "Render" );


    // QAction *quit = toolbar->addAction(QIcon(quitpix), "Quit Application");

    connect(renderAction, SIGNAL(triggered()), this, SLOT(render()));
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
    camRotateXSlider->hide();
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


    status = new QStatusBar(this);
    status->setStyleSheet("QStatusBar{padding-left:8px;background:rgba(128,128,128,255);color:black;font-weight:bold;}");
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

    renderThread.setTracer(tracer);
    renderThread.window = this;
    
    connect(&renderThread, SIGNAL(renderedImage(double, double, const QImage&)),
            this, SLOT(updateIndirect(double, double, QImage)));

    connect(&renderThread, SIGNAL(renderedImagePostProcess(double, double, const QImage&)),
            this, SLOT(updatePostProcess(double, double, QImage)));
    
    displayMode = 2;
    update();
}

void Window::updatePostProcess(double time, double samples, const QImage &image){

    status->showMessage("time: " + QString::number(time));
    indirectImage = image;
    postImage = postProcess(indirectImage);
    displayMode = 0;
    update();
}

void Window::updateIndirect(double time, double samples, const QImage &image){
    indirectImage = image;
    postImage = image;
    displayMode = 0;
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

void Window::openScene(){
    tracer->unloadScene();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Scene"),
                           "~/",
                           tr("Json (*.json)"));

    if (fileName != ""){
        qDebug() << "open file at: " << fileName;
        tracer->setupScene(fileName.toUtf8().constData());
        double directTime;
        tracer->renderDirect(directTime, directImage, normalImage, boundingBoxImage);
        update();    
    }
    
}

void Window::saveImage(){

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                           "~/",
                           tr("Images (*.png)"));
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    postImage.save(&buffer, "PNG"); // writes image into ba in PNG format
    file.write(ba);
    file.close();
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

void Window::updateProgress(){
    
    if (tracer->isRendering){
        statusBar()->showMessage("Rendering..." + QString::number(tracer->progress) + '%' +  '(' + QString::number(tracer->curSamples) + '/' + QString::number(tracer->samples) + ')' );    
    }
    
    
}

void Window::render(){
    statusBar()->showMessage("Rendering...");
    
    if (tracer->width != width || tracer->height != height){
        resize(width, height);
    }
    
    tracer->setResolution(width, height);
    tracer->samples = samples;
    

    // double indirectTime;
    // tracer->renderIndirect(indirectTime, indirectImage);
    // postImage = postProcess(indirectImage);
    // displayMode = 0;
    // update();

    // statusBar()->showMessage("time: " + QString::number(indirectTime));

    renderThread.render();

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateProgress()));
    timer->start(16);

}

QImage Window::postProcess(const QImage &image){
    // qDebug()<<"postProcess";
    QImage reuslt = QImage(image.width(), image.height(), QImage::Format_RGB32);
    
    qDebug() << rgbBox->currentText();
    for (int i = 0; i < image.width(); ++i){
        for (int j = 0; j < image.height(); ++j){
            // qDebug() << "getpxiel" << i << j;
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
    qDebug()<<"gamma correct";

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