#include "appeventfilter.h"

AppEventFilter::AppEventFilter(QObject *parent) : QObject(parent)
{

}

bool AppEventFilter::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)

    //如果有鼠标或者键盘事件,则认为用户处于活跃状态
    if (event->type() >= QEvent::MouseButtonPress && event->type() <= QEvent::KeyRelease){
        Q_EMIT userIsActive();
    }

    return false;
}
