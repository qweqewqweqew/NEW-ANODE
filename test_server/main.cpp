#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include "testwmsserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "========================================";
    qDebug() << "WMS测试服务器";
    qDebug() << "========================================";
    qDebug() << "按 Ctrl+C 或关闭窗口停止服务器\n";
    
    TestWmsServer server;
    
    // 启动服务器
    if (!server.start(9090)) {
        qCritical() << "服务器启动失败！";
        return 1;
    }
    
    // 保持程序运行
    QObject::connect(&server, &TestWmsServer::requestReceived,
                     [](const QJsonObject &data) {
        // 可以在这里添加额外的处理
    });
    
    return app.exec();
}






