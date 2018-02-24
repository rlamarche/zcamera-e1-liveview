#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QByteArray>
#include <QGraphicsPixmapItem>

#include <QDebug>

#include <arpa/inet.h>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    tcpSocket(new QTcpSocket(this)),
    scene(new QGraphicsScene(this)),
    reconnectTimer(new QTimer(this))
{
    ui->setupUi(this);
    ui->graphicsView->setScene(scene);

    connect(ui->actionConnect, SIGNAL(triggered(bool)), this, SLOT(doConnect(bool)));


    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(this, SIGNAL(frameAvailable(QByteArray)), this, SLOT(decodeFrame(QByteArray)));
    connect(this, SIGNAL(imageAvailable(QImage)), this, SLOT(showImage(QImage)));
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
    connect(reconnectTimer, SIGNAL(timeout()), this, SLOT(reconnect()));

    avcodec_register_all();
    codec = avcodec_find_decoder ( AV_CODEC_ID_H264 );
    codecCtx = avcodec_alloc_context3 ( codec );
    avcodec_open2 ( codecCtx, codec, NULL );

    static const char initData[] = {
          0x1
      };

    init = QByteArray::fromRawData(initData, sizeof(initData));
}

MainWindow::~MainWindow()
{
    reconnectTimer->stop();
    tcpSocket->close();

    if (item != Q_NULLPTR)
    {
        scene->removeItem(item);
        delete item;
    }

    delete ui;
}

void MainWindow::socketError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "Error with the socket:" << tcpSocket->errorString();
}

void MainWindow::disconnected()
{
    qDebug() << "Disconnected, trying to reconnect.";
    tcpSocket->close();
}

void MainWindow::doConnect(bool triggered)
{
    qDebug() << "Connecting ...";
    tcpSocket->connectToHost("192.168.168.1", 9876, QTcpSocket::ReadWrite);
//    tcpSocket->connectToHost("10.98.32.1", 9876, QTcpSocket::ReadWrite);
    if (!tcpSocket->waitForConnected())
    {
        qDebug() << "Connection timeout";
        return;
    }
    qDebug() << "Connected.";

    payload = 0;
    remainingToRead = 0;

    tcpSocket->write(init);
    if (!tcpSocket->waitForBytesWritten())
    {
        qDebug() << "Unable to write initial bytes";
    }

    qDebug() << "Initial bytes written";
    reconnectTimer->setInterval(100);
    reconnectTimer->start();
}

void MainWindow::reconnect()
{
    if (!tcpSocket->isOpen())
    {
        doConnect(false);
    }
}

void MainWindow::readyRead()
{
//    qDebug() << "Ready read:" << tcpSocket->bytesAvailable();
    if (remainingToRead == 0) {
        QByteArray payloadBytes = tcpSocket->read(4);
        payload = ntohl(* (quint32*) payloadBytes.constData());
//        qDebug() << "Payload size:" << payload;
        remainingToRead = payload;
        currentFrame = QByteArray();
    }

    const QByteArray frame = tcpSocket->read(tcpSocket->bytesAvailable());
    currentFrame += frame;
    remainingToRead -= frame.size();
//    qDebug() << "Data readen:" << frame.size();
//    qDebug() << "remainingToRead:" << remainingToRead;

    if (remainingToRead <= 0) {
        emit frameAvailable(currentFrame);
        if (tcpSocket->isWritable())
        {
            tcpSocket->write(init);
        }
    }

}



void MainWindow::showImage(QImage img)
{
    if (item != Q_NULLPTR)
    {
        scene->removeItem(item);
        delete item;
        item = Q_NULLPTR;
    }
    item = new QGraphicsPixmapItem(QPixmap::fromImage(img));
    scene->addItem(item);
    ui->graphicsView->show();
}

void MainWindow::decodeFrame(QByteArray frame)
{
    AVFrame *avFrame = nullptr;
    AVFrame *avFrameRGB = nullptr;
    AVPacket packet;

    int width = 1024;
    int height = 768;
    QImage img( width, height, QImage::Format_RGB888 );


    // Set avpkt data and size here
    avFrame = av_frame_alloc();
    if(!avFrame)
    {
        qDebug() << "Could not allocate video frame, stop thread.";
        return;
    }

    av_init_packet(&packet);
    av_new_packet(&packet, frame.size());
    memcpy(packet.data, frame.constData(), frame.size());

    err = avcodec_decode_video2 ( codecCtx, avFrame, &frame_decoded, &packet );
    if (err < 0)
    {
        qDebug() << "Unable to decode h264";
    }
    if (err == 0)
    {
        qDebug() << "No frame could be decompressed";
    }

    if (err > 0 && frame_decoded)
    {
        //qDebug() << "Frame width/height:" << codecCtx->width << codecCtx->height;

        avFrameRGB = av_frame_alloc();
        if(!avFrameRGB)
        {
            qDebug() << "Could not allocate video frame, stop thread.";
            return;
        }

        //Allocate memory for the pixels of a picture and setup the AVPicture fields for it.
        avpicture_alloc( ( AVPicture *) avFrameRGB, AV_PIX_FMT_RGB24, width, height);

        if (imgConvertCtx == nullptr) {
            qDebug() << "Image size:" << codecCtx->width << codecCtx->height;
            imgConvertCtx = sws_getContext( codecCtx->width, codecCtx->height, codecCtx->pix_fmt, width, height, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
        }

        sws_scale(imgConvertCtx, avFrame->data, avFrame->linesize, 0, codecCtx->height, avFrameRGB->data, avFrameRGB->linesize);
        //setting QImage from frameRGB
        for( int y = 0; y < height; ++y )
        {
           memcpy( img.scanLine(y), avFrameRGB->data[0]+y * avFrameRGB->linesize[0], avFrameRGB->linesize[0] );
        }

        if (!img.isNull())
        {
            emit imageAvailable(img);
        }
        av_frame_unref(avFrameRGB);
    }

    av_frame_unref(avFrame);
    av_free_packet(&packet);
}
