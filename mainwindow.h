#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTimer>
#include <QMediaPlayer>
#include <QVideoFrame>
#include <QVideoProbe>

#include <QGraphicsScene>
#include <QByteArray>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QTcpSocket *tcpSocket = Q_NULLPTR;
    QTimer *reconnectTimer = Q_NULLPTR;

    // Display
    QGraphicsScene *scene = Q_NULLPTR;
    QGraphicsPixmapItem *item = Q_NULLPTR;

    // Reading data
    quint32 payload = 0;
    quint32 remainingToRead = 0;
    QByteArray currentFrame;

    // AVCodec
    AVPacket avpkt;
    int err, frame_decoded = 0;
    AVCodec *codec;
    AVCodecContext *codecCtx;

    // SWS
    SwsContext *imgConvertCtx = nullptr;


    // DATA
    QByteArray init;

public slots:
    void doConnect(bool triggered);
    void reconnect();
    void disconnected();
    void socketError(QAbstractSocket::SocketError socketError);
    void readyRead();
    void showImage(QImage img);
    void decodeFrame(QByteArray frame);

signals:
    void imageAvailable(QImage image);
    void frameAvailable(QByteArray frame);
};

#endif // MAINWINDOW_H
