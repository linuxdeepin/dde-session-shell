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
    , m_loginButton(new QPushButton)
    , m_otherUserInput(new OtherUserInput(this))
    , m_showType(NormalType)
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
    switch (m_showType) {
    case NoPasswordType: {
        m_passwordEdit->abortAuth();
        m_passwordEdit->hide();
        m_otherUserInput->hide();
        break;
    }
    case NormalType: {
        m_passwordEdit->abortAuth();
        m_passwordEdit->show();
        m_otherUserInput->hide();
        break;
    }
    case IDAndPasswordType: {
        m_passwordEdit->hide();
        m_otherUserInput->show();
        m_otherUserInput->setFocus();
        break;
    }
    default:
        break;
    }
}

void UserLoginWidget::disablePassword(bool disable)
{
    m_passwordEdit->lineEdit()->setDisabled(disable);
    m_passwordEdit->setVisible(!disable);
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

//初始化窗体控件
void UserLoginWidget::initUI()
{
    m_userAvatar->setAvatarSize(UserAvatar::AvatarLargeSize);

    QPalette palette = m_nameLbl->palette();
    palette.setColor(QPalette::WindowText, Qt::white);
    m_nameLbl->setPalette(palette);
    DFontSizeManager::instance()->bind(m_nameLbl, DFontSizeManager::T2);
    m_nameLbl->setFixedSize(DDESESSIONCC::PASSWDLINEEIDT_WIDTH, DDESESSIONCC::PASSWDLINEEDIT_HEIGHT);
    m_nameLbl->setAlignment(Qt::AlignCenter);

    //暂时先将键盘布局按钮\解锁按钮等隐藏
    m_passwordEdit->setKeyboardButtonEnable(false);
    m_passwordEdit->setEyeButtonEnable(false);
    m_passwordEdit->setSubmitButtonEnable(false);
    m_passwordEdit->setContentsMargins(0, 0, 0, 0);
    m_passwordEdit->lineEdit()->setContextMenuPolicy(Qt::NoContextMenu);
    m_passwordEdit->setFixedSize(QSize(DDESESSIONCC::PASSWDLINEEIDT_WIDTH, DDESESSIONCC::PASSWDLINEEDIT_HEIGHT));
    m_passwordEdit->setFocusPolicy(Qt::StrongFocus);

    m_passwordEdit->setFocus();
    m_passwordEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);
    this->setFocusProxy(m_passwordEdit->lineEdit());

    m_otherUserInput->setFixedWidth(DDESESSIONCC::PASSWDLINEEIDT_WIDTH);
    m_otherUserInput->setFixedHeight(DDESESSIONCC::PASSWDLINEEDIT_HEIGHT * 2);

    m_userLayout = new QVBoxLayout;
    m_userLayout->setMargin(WidgetsSpacing);
    m_userLayout->setSpacing(WidgetsSpacing);

    m_userLayout->addWidget(m_userAvatar, 0, Qt::AlignHCenter);
    m_userLayout->addWidget(m_nameLbl, 0, Qt::AlignHCenter);
    m_userLayout->addWidget(m_otherUserInput, 0, Qt::AlignHCenter);
    m_userLayout->addWidget(m_passwordEdit, 0, Qt::AlignHCenter);

    m_blurEffectWidget->setMaskColor(DBlurEffectWidget::LightColor);
    m_blurEffectWidget->setBlurRectXRadius(BlurRectRadius);
    m_blurEffectWidget->setBlurRectYRadius(BlurRectRadius);

    m_lockLayout = new QVBoxLayout;
    m_lockLayout->setMargin(0);
    m_lockLayout->setSpacing(0);
    //TODO
    //test code 后续需要将此按钮替换为设计好的登录按钮
    m_loginButton->setText(tr("Login"));
    m_lockLayout->addWidget(m_loginButton, 0, Qt::AlignHCenter);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->addLayout(m_userLayout);
    mainLayout->addLayout(m_lockLayout);

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

    connect(m_loginButton, &QPushButton::clicked, this, [ = ] {
        QString password = m_passwordEdit->lineEdit()->text();
        if (password.isEmpty()) return;
        emit requestAuthUser(password);
    });
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
