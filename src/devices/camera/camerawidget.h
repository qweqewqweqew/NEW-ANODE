#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTimer>
#include <QImage>

class CameraWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CameraWidget(const QString &cameraName, QWidget *parent = nullptr);
    ~CameraWidget();

    void setImage(const QImage &image);
    void clearImage();  // 清除图像，恢复到黑色背景
    void setCameraName(const QString &name);
    QString getCameraName() const { return m_cameraName; }

signals:
    void cameraClicked(const QString &cameraName);

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void setupUI();
    void updateDisplay();

private:
    QString m_cameraName;
    QLabel *m_imageLabel;
    QImage m_currentImage;
};

#endif // CAMERAWIDGET_H

