#include "userloginwidget.h"
#include "lockpasswordwidget.h"
#include "framedatabind.h"
#include "userinfo.h"
#include "src/widgets/useravatar.h"
#include "src/widgets/loginbutton.h"
#include "src/global_util/constants.h"
#include "src/widgets/otheruserinput.h"

#include <DFontSizeManager>
#include <DPalette>

#include <QVBoxLayout>

static const int BlurRectRadius = 15;
static const int WidgetsSpacing = 10;

UserLoginWidget::UserLoginWidget(QWidget *parent)
    : QWidget(parent)
    , m_blurEffectWidget(new DBlurEffectWidget(this))
    , m_userAvatar(new UserAvatar(this))
    , m_nameLbl(new QLabel(this))
    , m_passwordEdit(new DPasswdEditAnimated(this))
    , m_lockPasswordWidget(new LockPasswordWidget)
    , m_otherUserInput(new OtherUserInput(this))
    , m_lockButton(new DFloatingButton(tr("Login")))
    , m_showType(NormalType)
    , m_isLock(false)
    , m_selected(false)
    , m_isLogin(false)
{
    initUI();
    initConnect();
}

//重置控件的状态
void UserLoginWidget::resetAllState()
{
    m_passwordEdit->lineEdit()->clear();
    m_passwordEdit->abortAuth();
}

void UserLoginWidget::grabKeyboard()
{
    if (m_passwordEdit->isVisible()) {
        m_passwordEdit->lineEdit()->grabKeyboard();
        return;
    }
}

//密码连续输入错误5次，设置提示信息
void UserLoginWidget::setFaildMessage(const QString &message)
{
    m_passwordEdit->abortAuth();

    if (m_isLock) {
        m_lockPasswordWidget->setMessage(message);
        return;
    }

    m_passwordEdit->lineEdit()->setPlaceholderText(message);
}

//密码输入错误,设置错误信息
void UserLoginWidget::setFaildTipMessage(const QString &message)
{
    if (message.isEmpty()) {
        m_passwordEdit->hideAlert();
    } else {
        m_passwordEdit->showAlert(message);
    }
}

//设置窗体显示模式
void UserLoginWidget::setWidgetShowType(UserLoginWidget::WidgetShowType showType)
{
    m_showType = showType;
    updateUI();
}

//更新窗体控件显示
void UserLoginWidget::updateUI()
{
    m_lockPasswordWidget->hide();
    switch (m_showType) {
    case NoPasswordType: {
        m_passwordEdit->abortAuth();
        m_passwordEdit->hide();
        m_otherUserInput->hide();
        m_lockButton->show();
        break;
    }
    case NormalType: {
        m_passwordEdit->abortAuth();
        m_passwordEdit->setVisible(!m_isLock);
        m_otherUserInput->hide();
        m_lockButton->show();
        m_lockPasswordWidget->setVisible(m_isLock);
        break;
    }
    case IDAndPasswordType: {
        m_passwordEdit->hide();
        m_otherUserInput->show();
        m_otherUserInput->setFocus();
        m_lockButton->show();
        break;
    }
    case UserFrameType: {
        m_passwordEdit->hide();
        m_otherUserInput->hide();
        m_lockButton->hide();
        break;
    }
    case UserFrameLoginType: {
        m_passwordEdit->hide();
        m_otherUserInput->hide();
        m_lockButton->hide();
        break;
    }
    default:
        break;
    }
}

//设置密码输入框不可用
void UserLoginWidget::disablePassword(bool disable, uint lockNum)
{
    m_isLock = disable;
    m_passwordEdit->lineEdit()->setDisabled(disable);
    m_passwordEdit->setVisible(!disable);
    m_lockPasswordWidget->setVisible(disable);

    if (disable) {
        setFaildMessage(tr("Please try again %n minute(s) later", "", lockNum));
    } else {
        if (isVisible()) {
            grabKeyboard();
        }
    }
}

void UserLoginWidget::updateAuthType(SessionBaseModel::AuthType type)
{
    m_authType = type;
}

void UserLoginWidget::refreshBlurEffectPosition()
{
    QRect rect = m_userLayout->geometry();
    rect.setTop(rect.top() + m_userAvatar->height() / 2 + m_userLayout->margin());

    m_blurEffectWidget->setGeometry(rect);
}

//窗体resize事件,更新阴影窗体的位置
void UserLoginWidget::resizeEvent(QResizeEvent *event)
{
    refreshBlurEffectPosition();

    return QWidget::resizeEvent(event);
}

void UserLoginWidget::showEvent(QShowEvent *event)
{
    updateUI();

    return QWidget::showEvent(event);
}

//paintEvent函数中绘制背景，当UserFrameList中用户被选中时绘制，颜色和不透明度是V15版本的，后续样式确定后再调整
void UserLoginWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    if (!m_selected)
        return;

    QPainter painter(this);
    painter.setPen(QPen(QColor(255, 255, 255, 51), 2));
    painter.setBrush(QColor(0, 0, 0, 76));
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawRoundedRect(QRect(0, 0, width(), height()), BlurRectRadius, BlurRectRadius, Qt::RelativeSize);
}

//初始化窗体控件
void UserLoginWidget::initUI()
{
    m_userAvatar->setAvatarSize(UserAvatar::AvatarLargeSize);

    QPalette palette = m_nameLbl->palette();
    palette.setColor(QPalette::WindowText, Qt::white);
    m_nameLbl->setPalette(palette);
    m_nameLbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    DFontSizeManager::instance()->bind(m_nameLbl, DFontSizeManager::T2);
    m_nameLbl->setAlignment(Qt::AlignCenter);

    //暂时先将键盘布局按钮\解锁按钮等隐藏
    m_passwordEdit->setKeyboardButtonEnable(false);
    m_passwordEdit->setEyeButtonEnable(false);
    m_passwordEdit->setSubmitButtonEnable(false);
    m_passwordEdit->setContentsMargins(0, 0, 0, 0);
    m_passwordEdit->lineEdit()->setContextMenuPolicy(Qt::NoContextMenu);
    m_passwordEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_passwordEdit->setFocusPolicy(Qt::StrongFocus);

    m_passwordEdit->setVisible(true);
    m_passwordEdit->setFocus();
    m_passwordEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);
    this->setFocusProxy(m_passwordEdit->lineEdit());

    m_userLayout = new QVBoxLayout;
    m_userLayout->setMargin(WidgetsSpacing);
    m_userLayout->setSpacing(WidgetsSpacing);

    m_userLayout->addWidget(m_userAvatar, 0, Qt::AlignHCenter);
    m_userLayout->addWidget(m_nameLbl, 0, Qt::AlignHCenter);
    m_userLayout->addWidget(m_otherUserInput);
    m_userLayout->addWidget(m_passwordEdit);
    m_userLayout->addWidget(m_lockPasswordWidget);
    m_lockPasswordWidget->hide();

    m_blurEffectWidget->setMaskColor(DBlurEffectWidget::LightColor);
    m_blurEffectWidget->setBlurRectXRadius(BlurRectRadius);
    m_blurEffectWidget->setBlurRectYRadius(BlurRectRadius);

    m_lockLayout = new QVBoxLayout;
    m_lockLayout->setMargin(0);
    m_lockLayout->setSpacing(0);
    m_lockLayout->addWidget(m_lockButton, 0, Qt::AlignHCenter);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->addStretch();
    mainLayout->addLayout(m_userLayout);
    mainLayout->addLayout(m_lockLayout);
    mainLayout->addStretch();

    setLayout(mainLayout);
    updateUI();
}

//初始化槽函数连接
void UserLoginWidget::initConnect()
{
    connect(m_passwordEdit->lineEdit(), &QLineEdit::textChanged, this, [ = ](const QString & value) {
        FrameDataBind::Instance()->updateValue(tr("Password"), value);
    });
    connect(m_passwordEdit, &DPasswdEditAnimated::submit, this, [ = ](const QString & passwd) {
        if (passwd.isEmpty()) return;
        emit requestAuthUser(passwd);
    });

    connect(m_lockButton, &QPushButton::clicked, this, [ = ] {
        QString password = m_passwordEdit->lineEdit()->text();
        if (password.isEmpty()) return;
        emit requestAuthUser(password);
    });
    connect(m_userAvatar, &UserAvatar::clicked, this, &UserLoginWidget::clicked);
}

//设置用户名
void UserLoginWidget::setName(const QString &name)
{
    m_nameLbl->setText(name);
}

//设置用户头像
void UserLoginWidget::setAvatar(const QString &avatar)
{
    m_userAvatar->setIcon(avatar);
}

//设置用户头像尺寸
void UserLoginWidget::setUserAvatarSize(int width, int height)
{
    m_userAvatar->setFixedSize(width, height);
}

bool UserLoginWidget::getSelected() const
{
    return m_selected;
}

void UserLoginWidget::setSelected(bool selected)
{
    m_selected = selected;
    update();
}

void UserLoginWidget::setWidgetWidth(int width)
{
    this->setFixedWidth(width);
}

void UserLoginWidget::setIsLogin(bool isLogin)
{
    m_isLogin = isLogin;
    if (!m_isLogin) return;
    setWidgetShowType(UserFrameLoginType);
}

bool UserLoginWidget::getIsLogin()
{
    return  m_isLogin;
}
