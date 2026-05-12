#include "plcconnectdialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QDebug>


class PLCController;
enum class PLCStatus { Disconnected, Connecting, Connected, Error };

class PLCControllerBridge : public QObject { Q_OBJECT public: using QObject::QObject; };

PLCConnectDialog::PLCConnectDialog(PLCController *controller, QWidget *parent)
    : QDialog(parent), m_controller(controller), m_ipEdit(nullptr), m_rackSpin(nullptr), m_slotSpin(nullptr), m_connectBtn(nullptr)
{
    setupUi();
    refreshButton();
}

PLCConnectDialog::~PLCConnectDialog() = default;

void PLCConnectDialog::setupUi()
{
    setWindowTitle(QString::fromUtf8("PLC连接"));
    setModal(true);

    auto *mainLayout = new QVBoxLayout(this);

    auto *ipLayout = new QHBoxLayout();
    ipLayout->addWidget(new QLabel(QString::fromUtf8("IP地址"), this));
    m_ipEdit = new QLineEdit(this);
    m_ipEdit->setPlaceholderText("192.168.1.10");
    ipLayout->addWidget(m_ipEdit, 1);
    mainLayout->addLayout(ipLayout);

    auto *rackLayout = new QHBoxLayout();
    rackLayout->addWidget(new QLabel(QString::fromUtf8("机架号"), this));
    m_rackSpin = new QSpinBox(this);
    m_rackSpin->setRange(0, 10);
    m_rackSpin->setValue(0);
    rackLayout->addWidget(m_rackSpin);
    rackLayout->addSpacing(16);
    rackLayout->addWidget(new QLabel(QString::fromUtf8("槽号"), this));
    m_slotSpin = new QSpinBox(this);
    m_slotSpin->setRange(0, 10);
    m_slotSpin->setValue(0);
    rackLayout->addWidget(m_slotSpin);
    mainLayout->addLayout(rackLayout);

    auto *btnLayout = new QHBoxLayout();
    m_connectBtn = new QPushButton(QString::fromUtf8("连接"), this);
    connect(m_connectBtn, &QPushButton::clicked, this, &PLCConnectDialog::onConnectOrDisconnect);
    btnLayout->addStretch(1);
    btnLayout->addWidget(m_connectBtn);
    mainLayout->addLayout(btnLayout);

    resize(380, 160);
}

void PLCConnectDialog::refreshButton()
{

}

void PLCConnectDialog::onConnectOrDisconnect()
{
    accept();
}

void PLCConnectDialog::onStatusChanged(int)
{
    refreshButton();
}

QString PLCConnectDialog::ip() const { return m_ipEdit ? m_ipEdit->text().trimmed() : QString(); }
int PLCConnectDialog::rack() const { return m_rackSpin ? m_rackSpin->value() : 0; }
int PLCConnectDialog::slot() const { return m_slotSpin ? m_slotSpin->value() : 0; }
void PLCConnectDialog::setIp(const QString &ip) { if (m_ipEdit) m_ipEdit->setText(ip); }
void PLCConnectDialog::setRack(int v) { if (m_rackSpin) m_rackSpin->setValue(v); }
void PLCConnectDialog::setSlot(int v) { if (m_slotSpin) m_slotSpin->setValue(v); }

#include "plcconnectdialog.moc"


