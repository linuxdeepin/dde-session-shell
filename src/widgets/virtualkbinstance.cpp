#include "virtualkbinstance.h"

#include <QProcess>
#include <QWindow>
#include <QWidget>
#include <QTimer>
#include <QDebug>
#include <QResizeEvent>
#include <QBoxLayout>

VirtualKBInstance &VirtualKBInstance::Instance()
{
    static VirtualKBInstance virtualKB;
    return virtualKB;
}

QWidget *VirtualKBInstance::virtualKBWidget() {
    return m_virtualKBWidget;
}

void VirtualKBInstance::init(QWidget *parent)
{
    if (m_virtualKBWidget) {
        emit initFinished();
        return;
    }

    QProcess * p = new QProcess(this);
    connect(p, &QProcess::readyReadStandardOutput, [this, p, parent] {
        QByteArray output = p->readAllStandardOutput();
        if (output.isEmpty()) return;
        int xid = atoi(output.trimmed().toStdString().c_str());
        debug("init", "xid: " + QString::number(xid));
        QWindow * w = QWindow::fromWinId(xid);
        m_virtualKBWidget = QWidget::createWindowContainer(w, nullptr, Qt::FramelessWindowHint);
        m_virtualKBWidget->installEventFilter(this);
        m_virtualKBWidget->setFixedSize(600, 200);
        m_virtualKBWidget->setParent(parent);
        m_virtualKBWidget->raise();

        QTimer::singleShot(1000, [=] {
//            m_virtualKBWidget->setVisible(false);
            emit initFinished();
            this->debug("inti", "create virtual kb end!");
        });
    });

    debug("inti", "start onboard...");

    p->start("onboard", QStringList() << "-e" << "--layout" << "Small" << "--size" << "60x5" << "-a");
}

void VirtualKBInstance::debug(const QString &type, const QString &info)
{
    qDebug() << QString("[VirtualKB-%1]").arg(type) << info << endl;
}

bool VirtualKBInstance::eventFilter(QObject *watched, QEvent *event)
{
//    this->debug("init", QString("event type: %1").arg(QString::number(event->type())));

    if (watched == m_virtualKBWidget && event->type() == QEvent::Resize) {
        QResizeEvent *e = static_cast<QResizeEvent*>(event);
        return e->size() != QSize(600, 200);
    }

    return QObject::eventFilter(watched, event);
}

VirtualKBInstance::VirtualKBInstance(QObject *parent)
    : QObject(parent)
    , m_virtualKBWidget(nullptr)
{

}
