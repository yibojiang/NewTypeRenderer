
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

void RenderThread::restartRender(){
    restart = true;
}

void RenderThread::abortRender(){
    abort = true;
}

RenderThread::~RenderThread(){
    mutex.lock();
    abort = true;
    condition.wakeOne();
    mutex.unlock();
    wait();
}



void RenderThread::run(){
    restart = false;
    abort = false;
    while(!abort) {
        mutex.lock();
        int width = tracer->width;
        int height = tracer->height;
        QImage image(width, height, QImage::Format_RGB32);
        QImage postImage(width, height, QImage::Format_RGB32);
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
            // qDebug() << "before color: " << colorArray[0];
            tracer->curSamples = s + 1;
            tracer->renderIndirectProgressive(colorArray, abort, restart, s);
            
            if (abort){
                return;
            }

            if (restart){
                break;
            }

            for (int i = 0; i < height; ++i){
                for (int j = 0; j < width; ++j){
                    vec3 raw = colorArray[i*width+j];
                    image.setPixel(j, i, qRgb(raw.x*255, raw.y*255, raw.z*255));
                    double r = pow(raw.x, 1.0/2.2);
                    double g = pow(raw.y, 1.0/2.2);
                    double b = pow(raw.z, 1.0/2.2);
                    postImage.setPixel(j, i, qRgb(r*255, g*255, b*255));
                    

                }
                emit renderedImage(indirectTime, s, image, postImage); 
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
    width = 960, height = 540;
    resize(width, height);

    setContextMenuPolicy(Qt::NoContextMenu);

    setWindowTitle("Render View " + QString::number(width) + "x" + QString::number(height));
    displayMode = 0;
    samples = 16;
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
    
    connect(&renderThread, SIGNAL(renderedImage(double, double, const QImage&, const QImage&)),
            this, SLOT(updateIndirect(double, double, QImage, QImage)));

    connect(&renderThread, SIGNAL(renderedImagePostProcess(double, double, const QImage&)),
            this, SLOT(updatePostProcess(double, double, QImage)));
    
    displayMode = 2;
    update();
}

void Window::updatePostProcess(double time, double, const QImage &image){

    status->showMessage("time: " + QString::number(time));
    indirectImage = image;
    postImage = postProcess(indirectImage);
    displayMode = 0;
    this->renderTime = time;
    update();
}

void Window::updateIndirect(double time, double, const QImage &image, const QImage &postImage){
    this->indirectImage = image;
    this->postImage = postImage;
    this->renderTime = time;
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

    std::string saveName = "image_" + std::to_string(tracer->curSamples) + "_" + std::to_string(int(this->renderTime)) + 's' ;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                           saveName.c_str(),
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
        // qDebug() << "mode: boundingBoxImage";
    }
    
}

void Window::mousePressEvent(QMouseEvent *event)
{
    this->pressPos = event->pos();
    if (event->button() == Qt::LeftButton) {
        // lastPoint = event->pos();
        // scribbling = true;
        

        // qDebug() << "mouse press event";
    }
}

void Window::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && QApplication::queryKeyboardModifiers() == Qt::AltModifier){
        // drawLineTo(event->pos());
        // qDebug() << QApplication::queryKeyboardModifiers();
        // qDebug() << "mouse move event" << event->pos();
        QPoint dr = event->pos() - this->pressPos;
        tracer->rotateCamera(-dr.y() * 0.01, -dr.x() * 0.01, 0.0);
        double directTime;
        tracer->renderDirect(directTime, directImage, normalImage, boundingBoxImage);
        update();

        this->pressPos = event->pos();
        renderThread.restartRender();
        
    }

    if ((event->buttons() & Qt::RightButton) && QApplication::queryKeyboardModifiers() == Qt::AltModifier){
        QPoint dr = event->pos() - this->pressPos;
        tracer->scaleCamera(dr.x()*0.01);
        // qDebug() << "scale" << dr.x() * 0.01;
        double directTime;
        tracer->renderDirect(directTime, directImage, normalImage, boundingBoxImage);
        update();
        this->pressPos = event->pos();
        renderThread.restartRender();
    }

    if(event->buttons() & Qt::MiddleButton && QApplication::queryKeyboardModifiers() == Qt::AltModifier)
    {
        QPoint dr = event->pos() - this->pressPos;
        tracer->moveCamera(dr.x(),dr.y());
        // qDebug() << "move" << dr.x() * 0.1;
        double directTime;
        tracer->renderDirect(directTime, directImage, normalImage, boundingBoxImage);
        update();
        this->pressPos = event->pos();
        renderThread.restartRender();
        
    }
}

void Window::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // drawLineTo(event->pos());
        // scribbling = false;
        // qDebug() << "mouse release event";
    }
}

int main(int argc, char *argv[]) {

 
    QApplication app (argc, argv);
    Window window;
    window.show();

    
    
    return app.exec();
}