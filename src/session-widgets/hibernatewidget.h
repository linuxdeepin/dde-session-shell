#ifndef HIBERNATEWIDGET_H
#define HIBERNATEWIDGET_H
#include <QWidget>
#include <memory>
#include <QPainter>
#include <dloadingindicator.h>
#include <QPaintEvent>


#include <com_deepin_daemon_imageblur.h>
#include <com_deepin_sessionmanager.h>

#include "sessionbasewindow.h"
#include "sessionbasemodel.h"
#include "src/widgets/mediawidget.h"

class HibernateWidget :public SessionBaseWindow
{
public:
    HibernateWidget(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *e);
private:
    DLoadingIndicator *rotateIcon;
    QLabel *label;
    QWidget *widget;
    QVBoxLayout *vlayout;
};

#endif // HIBERNATEWIDGET_H
