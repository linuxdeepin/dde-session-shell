#ifndef ACCESSIBILITYCHECKEREX_H
#define ACCESSIBILITYCHECKEREX_H

#if defined(DSS_CHECK_ACCESSIBILITY) && defined(QT_DEBUG)

#include <QObject>
#include <QWidget>
#include <DAccessibilityChecker>

DWIDGET_USE_NAMESPACE

class AccessibilityCheckerEx : public DAccessibilityChecker
{
    Q_OBJECT
public:
    void addIgnoreName(const QString &name);

protected:
    virtual bool isIgnore(Role role, const QWidget *w) override ;

private:
    QStringList m_nameList;
};
#endif //defined(DSS_CHECK_ACCESSIBILITY) && defined(QT_DEBUG)

#endif // ACCESSIBILITYCHECKEREX_H
