#include "widget.h"
#include <QMainWindow>
#include <QApplication>
#include <QIcon>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDateTime>
#include <QMutex>
#include <QMutexLocker>

// 全局日志处理：将 Qt 的控制台输出同步写入日志文件
static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);

    static QMutex mutex;
    QMutexLocker locker(&mutex);

    const QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString level;
    switch (type) {
    case QtDebugMsg:    level = "DEBUG"; break;
    case QtInfoMsg:     level = "INFO";  break;
    case QtWarningMsg:  level = "WARN";  break;
    case QtCriticalMsg: level = "ERROR"; break;
    case QtFatalMsg:    level = "FATAL"; break;
    }

    const QString line = QString("[%1][%2] %3").arg(timestamp, level, msg);

    const QString logDirPath = QDir(QCoreApplication::applicationDirPath()).filePath("logs");
    QDir().mkpath(logDirPath);
    const QString logFilePath = QDir(logDirPath).filePath(QDate::currentDate().toString("yyyyMMdd") + ".txt");

    QFile file(logFilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream ts(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        ts.setCodec("UTF-8");
#endif
        ts << line << '\n';
    }

    // 同时输出到标准错误，保持原有控制台可见性
    fprintf(stderr, "%s\n", line.toLocal8Bit().constData());
    fflush(stderr);

    if (type == QtFatalMsg) {
        abort();
    }
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(messageHandler);
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/image/YRK.ico"));
    Widget w;
    w.showMaximized();
    return a.exec();
}
