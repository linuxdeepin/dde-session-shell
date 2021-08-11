#include "accessibilitycheckerex.h"

#if defined(DSS_CHECK_ACCESSIBILITY) && defined(QT_DEBUG)

/**
 * @brief addIgnoreName 添加需要忽略的控件的objectName或accessibleName
 * @param name 只能是objectName或accessibleName
 */
void AccessibilityCheckerEx::addIgnoreName(const QString &name)
{
    Q_ASSERT(!name.isEmpty());

    if (!m_nameList.contains(name)) {
        m_nameList.append(name);
    }
}

/**
 * @brief isIgnore 忽略objectName和accessibleName在m_nameList中的对象
 * @param role
 * @param w
 * @return true:不检查此widget，false:检查
 */
bool AccessibilityCheckerEx::isIgnore(Role role, const QWidget *w)
{
    //TODO DDialog添加button的时候报错，暂时自行过滤VLine，dtk修复后移除掉
    if (!w || "VLine" == w->objectName()) {
        return true;
    }

    if (m_nameList.contains(w->objectName()) || m_nameList.contains(w->accessibleName())) {
        return true;
    }

    return DAccessibilityChecker::isIgnore(role, w);
}

#endif //defined(DSS_CHECK_ACCESSIBILITY) && defined(QT_DEBUG)
