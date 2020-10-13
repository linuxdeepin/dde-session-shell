#include "shutdownworker.h"
#include "src/session-widgets/userinfo.h"
#include "src/session-widgets/sessionbasemodel.h"
#include <unistd.h>

using namespace Auth;

ShutdownWorker::ShutdownWorker(SessionBaseModel * const model, QObject *parent)
    : AuthInterface(model, parent)
    , m_hotZoneInter(new DBusHotzone("com.deepin.daemon.Zone", "/com/deepin/daemon/Zone", QDBusConnection::sessionBus(), this))
{
    if (valueByQSettings<bool>("", "loginPromptAvatar", true)) {
        initDBus();
        initData();

        m_currentUserUid = getuid();
        onUserAdded(ACCOUNTS_DBUS_PREFIX + QString::number(m_currentUserUid));
        model->setCurrentUser(model->findUserByUid(getuid()));
    }

    connect(model, &SessionBaseModel::onStatusChanged, this, [ = ](SessionBaseModel::ModeStatus status) {
        if (status == SessionBaseModel::ModeStatus::PowerMode) {
            checkPowerInfo();
        }
    });
}

void ShutdownWorker::switchToUser(std::shared_ptr<User> user)
{
    Q_UNUSED(user);
}

void ShutdownWorker::authUser(const QString &password)
{
    Q_UNUSED(password);
}

void ShutdownWorker::enableZoneDetected(bool enable)
{
    m_hotZoneInter->EnableZoneDetected(enable);
}
