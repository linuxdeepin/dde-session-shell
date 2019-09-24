/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     lixin <lixin_cm@deepin.com>
 *
 * Maintainer: lixin <lixin_cm@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "userloginwidget.h"
#include "lockpasswordwidget.h"
#include "framedatabind.h"
#include "userinfo.h"
#include "src/widgets/useravatar.h"
#include "src/widgets/loginbutton.h"
#include "src/global_util/constants.h"
#include "src/widgets/otheruserinput.h"
#include "src/widgets/kblayoutwidget.h"
#include "src/session-widgets/framedatabind.h"
#include "src/widgets/keyboardmonitor.h"
#include "src/widgets/dpasswordeditex.h"

#include <DFontSizeManager>
#include <DPalette>

#include <QVBoxLayout>
#include <QAction>
#include <QImage>

static const int BlurRectRadius = 15;
static const int WidgetsSpacing = 10;
static const int Margins = 16;

UserLoginWidget::UserLoginWidget(QWidget *parent)
    : QWidget(parent)
    , m_blurEffectWidget(new DBlurEffectWidget(this))
    , m_userAvatar(new UserAvatar(this))
    , m_nameLbl(new QLabel(this))
    , m_passwordEdit(new DPasswordEditEx(this))
    , m_lockPasswordWidget(new LockPasswordWidget)
    , m_otherUserInput(new OtherUserInput(this))
    , m_lockButton(new DFloatingButton(DStyle::SP_UnlockElement))
    , m_kbLayoutBorder(new DArrowRectangle(DArrowRectangle::ArrowTop))
    , m_kbLayoutWidget(new KbLayoutWidget(QStringList()))
    , m_showType(NormalType)
    , m_isLock(false)
    , m_isLogin(false)
    , m_isSelected(false)
    , m_capslockMonitor(KeyboardMonitor::instance())
{
    initUI();
    initConnect();
}

UserLoginWidget::~UserLoginWidget()
{
    m_kbLayoutBorder->deleteLater();
}

//重置控件的状态
void UserLoginWidget::resetAllState()
{
    m_passwordEdit->clear();
}

//密码连续输入错误5次，设置提示信息
void UserLoginWidget::setFaildMessage(const QString &message)
{
    if (m_isLock && !message.isEmpty()) {
        m_lockPasswordWidget->setMessage(message);
        return;
    }

    m_passwordEdit->setPlaceholderText(message);
}

//密码输入错误,设置错误信息
void UserLoginWidget::setFaildTipMessage(const QString &message)
{
    if (message.isEmpty()) {
        m_passwordEdit->hideAlertMessage();
    } else {
        m_passwordEdit->hideLoadSlider();
        m_passwordEdit->showAlertMessage(message);
        m_passwordEdit->selectAll();
    }
}

//设置窗体显示模式
void UserLoginWidget::setWidgetShowType(UserLoginWidget::WidgetShowType showType)
{
    m_showType = showType;
    updateUI();
    if (m_showType == NormalType || m_showType == IDAndPasswordType) {
        QMap<QString, int> registerFunctionIndexs;
        std::function<void (QVariant)> passwordChanged = std::bind(&UserLoginWidget::onOtherPagePasswordChanged, this, std::placeholders::_1);
        registerFunctionIndexs["UserLoginPassword"] = FrameDataBind::Instance()->registerFunction("UserLoginPassword", passwordChanged);
        std::function<void (QVariant)> kblayoutChanged = std::bind(&UserLoginWidget::onOtherPageKBLayoutChanged, this, std::placeholders::_1);
        registerFunctionIndexs["UserLoginKBLayout"] = FrameDataBind::Instance()->registerFunction("UserLoginKBLayout", kblayoutChanged);
        connect(this, &UserLoginWidget::destroyed, this, [ = ] {
            for (auto it = registerFunctionIndexs.constBegin(); it != registerFunctionIndexs.constEnd(); ++it)
            {
                FrameDataBind::Instance()->unRegisterFunction(it.key(), it.value());
            }
        });

        QTimer::singleShot(0, this, [ = ] {
            FrameDataBind::Instance()->refreshData("UserLoginPassword");
            FrameDataBind::Instance()->refreshData("UserLoginKBLayout");
        });
    }
}

//更新窗体控件显示
void UserLoginWidget::updateUI()
{
    m_lockPasswordWidget->hide();
    m_otherUserInput->hide();
    switch (m_showType) {
    case NoPasswordType: {
        bool isNopassword = true;
        if (m_authType == SessionBaseModel::LockType) {
            isNopassword = false;
            m_passwordEdit->setFocus();
        } else {
            m_lockButton->setFocus();
        }
        m_passwordEdit->setVisible(!isNopassword && !m_isLock);
        m_lockPasswordWidget->setVisible(m_isLock);

        m_lockButton->show();
        break;
    }
    case NormalType: {
        m_passwordEdit->setVisible(!m_isLock);
        updateKBLayout(m_KBLayoutList);
        setDefaultKBLayout(m_defkBLayout);
        m_lockButton->show();
        m_lockPasswordWidget->setVisible(m_isLock);
        m_passwordEdit->setFocus();
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
        m_lockButton->hide();
        break;
    }
    case UserFrameLoginType: {
        m_passwordEdit->hide();
        m_lockButton->hide();
        break;
    }
    default:
        break;
    }

    if (m_passwordEdit->isVisible())
        m_userAvatar->setFocusProxy(m_passwordEdit);
}

void UserLoginWidget::onOtherPagePasswordChanged(const QVariant &value)
{
    int cursorIndex =  m_passwordEdit->cursorPosition();
    m_passwordEdit->setText(value.toString());
    m_passwordEdit->setCursorPosition(cursorIndex);
}

void UserLoginWidget::onOtherPageKBLayoutChanged(const QVariant &value)
{
    if (value.toBool()) {
        m_kbLayoutBorder->setParent(window());
    }

    m_kbLayoutBorder->setVisible(value.toBool());

    if (m_kbLayoutBorder->isVisible()) {
        m_kbLayoutBorder->raise();
    }

    refreshKBLayoutWidgetPosition();
}

void UserLoginWidget::toggleKBLayoutWidget()
{
    if (m_kbLayoutBorder->isVisible()) {
        m_kbLayoutBorder->hide();
    } else {
        // 保证布局选择控件不会被其它控件遮挡
        // 必须要将它作为主窗口的子控件
        m_kbLayoutBorder->setParent(window());
        m_kbLayoutBorder->setVisible(true);
        m_kbLayoutBorder->raise();
        refreshKBLayoutWidgetPosition();
    }
    FrameDataBind::Instance()->updateValue("UserLoginKBLayout", m_kbLayoutBorder->isVisible());
}

void UserLoginWidget::refreshKBLayoutWidgetPosition()
{
    const QPoint &point = mapTo(m_kbLayoutBorder->parentWidget(), QPoint(m_passwordEdit->geometry().x() + (m_passwordEdit->width() / 2),
                                                                         m_passwordEdit->geometry().bottomLeft().y()));
    m_kbLayoutBorder->move(point.x(), point.y());
    m_kbLayoutBorder->setArrowX(15);
}

void UserLoginWidget::capslockStatusChanged(bool on)
{
    if (on) {
        m_capsAction->setIcon(QIcon(":/icons/dedpin/builtin/caps_lock.svg"));
    } else {
        m_capsAction->setIcon(QIcon());
    }
}

void UserLoginWidget::receiveUserKBLayoutChanged(const QString &layout)
{
    QString layoutName = layout;
    layoutName = layoutName.replace(";", "");

    if (m_KBLayoutList.size() == 1) {
        layoutName = "";
    }

    QImage image = generateImageFromString(layoutName);
    m_KBAction->setIcon(QIcon(QPixmap::fromImage(image)));
    emit requestUserKBLayoutChanged(layout);
}

QImage UserLoginWidget::generateImageFromString(const QString &name)
{
    QFont font = m_passwordEdit->font();
    font.setPixelSize(32);
    font.setWeight(QFont::DemiBold);
    int word_size = QFontMetrics(font).height();
    qreal device_ratio = m_passwordEdit->devicePixelRatioF();
    QImage image(QSize(word_size, word_size) * device_ratio, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    image.setDevicePixelRatio(device_ratio);

    QPainter painter(&image);
    painter.setFont(font);
    painter.setPen(Qt::white);

    QRect image_rect = image.rect();
    QRect r(image_rect.left(), image_rect.top(), word_size, word_size);
    painter.drawText(r, Qt::AlignCenter, name);
    return image;
}

//设置密码输入框不可用
void UserLoginWidget::disablePassword(bool disable, uint lockNum)
{
    m_isLock = disable;
    m_passwordEdit->setDisabled(disable);
    m_passwordEdit->setVisible(!disable);
    m_lockPasswordWidget->setVisible(disable);

    if (disable) {
        setFaildMessage(tr("Please try again %n minute(s) later", "", lockNum));
    }
}

void UserLoginWidget::updateAuthType(SessionBaseModel::AuthType type)
{
    m_authType = type;
    if (m_authType == SessionBaseModel::LightdmType) {
        m_lockButton->setIcon(DStyle::SP_ArrowNext);
    }
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
    QTimer::singleShot(0, this, &UserLoginWidget::refreshBlurEffectPosition);
    QTimer::singleShot(0, this, &UserLoginWidget::refreshKBLayoutWidgetPosition);

    return QWidget::resizeEvent(event);
}

void UserLoginWidget::showEvent(QShowEvent *event)
{
    updateUI();
    refreshBlurEffectPosition();

    return QWidget::showEvent(event);
}

void UserLoginWidget::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    emit clicked();
}

void UserLoginWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    if (!m_isSelected)
        return;
    QPainter painter(this);
    //选中时，在窗体底端居中，绘制92*4尺寸的圆角矩形，样式数据来源于设计图
    painter.setPen(QColor(255, 255, 255, 76));
    painter.setBrush(QColor(255, 255, 255, 76));
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawRoundedRect(QRect(width() / 2 - 46, rect().bottom() - 4, 92, 4), 2, 2);
}

//初始化窗体控件
void UserLoginWidget::initUI()
{
    m_userAvatar->setAvatarSize(UserAvatar::AvatarLargeSize);
    m_userAvatar->setFocusPolicy(Qt::NoFocus);

    m_capslockMonitor->start(QThread::LowestPriority);

    QPalette palette = m_nameLbl->palette();
    palette.setColor(QPalette::WindowText, Qt::white);
    m_nameLbl->setPalette(palette);
    m_nameLbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    DFontSizeManager::instance()->bind(m_nameLbl, DFontSizeManager::T2);
    m_nameLbl->setAlignment(Qt::AlignCenter);

    m_passwordEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_passwordEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_passwordEdit->setAlignment(Qt::AlignCenter);
    m_KBAction = new QAction;
    m_capsAction = new QAction;
    capslockStatusChanged(m_capslockMonitor->isCapslockOn());
    m_passwordEdit->addAction(m_KBAction, QLineEdit::LeadingPosition);
    m_passwordEdit->addAction(m_capsAction, QLineEdit::TrailingPosition);

    m_kbLayoutBorder->hide();
    m_kbLayoutBorder->setBackgroundColor(QColor(255, 255, 255, 51));    //255*0.2
    m_kbLayoutBorder->setBorderColor(QColor(0, 0, 0, 0));
    m_kbLayoutBorder->setBorderWidth(0);
    m_kbLayoutBorder->setMargin(0);
    m_kbLayoutBorder->setContent(m_kbLayoutWidget);
    m_kbLayoutBorder->setFixedWidth(DDESESSIONCC::PASSWDLINEEIDT_WIDTH);
    m_kbLayoutWidget->setFixedWidth(DDESESSIONCC::PASSWDLINEEIDT_WIDTH);

    m_lockPasswordWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_otherUserInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_passwordEdit->setVisible(true);

    m_userLayout = new QVBoxLayout;
    m_userLayout->setMargin(WidgetsSpacing);
    m_userLayout->setSpacing(WidgetsSpacing);

    m_userLayout->addWidget(m_userAvatar, 0, Qt::AlignHCenter);

    m_nameLayout = new QHBoxLayout;
    m_nameLayout->setMargin(0);
    m_nameLayout->setSpacing(5);
    //在用户名前，插入一个图标(m_loginLabel)用来表示多用户切换时已登录用户的标记
    m_loginLabel = new QLabel();
    m_loginLabel->setPixmap(QPixmap(":/icons/dedpin/builtin/select.svg"));
    m_loginLabel->hide();
    m_nameLayout->addWidget(m_loginLabel);
    m_nameLayout->addWidget(m_nameLbl);
    m_nameFrame = new QFrame;
    m_nameFrame->setLayout(m_nameLayout);
    m_userLayout->addWidget(m_nameFrame, 0, Qt::AlignHCenter);

    m_userLayout->addWidget(m_otherUserInput);
    m_userLayout->addWidget(m_passwordEdit);
    m_userLayout->addWidget(m_lockPasswordWidget);
    m_lockPasswordWidget->hide();

    m_blurEffectWidget->setMaskColor(DBlurEffectWidget::LightColor);
    m_blurEffectWidget->setBlurRectXRadius(BlurRectRadius);
    m_blurEffectWidget->setBlurRectYRadius(BlurRectRadius);

    m_lockButton->setFocusPolicy(Qt::StrongFocus);

    m_lockLayout = new QVBoxLayout;
    m_lockLayout->setMargin(0);
    m_lockLayout->setSpacing(0);
    m_lockLayout->addWidget(m_lockButton, 0, Qt::AlignHCenter);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->addStretch();
    mainLayout->addLayout(m_userLayout);
    mainLayout->addLayout(m_lockLayout);
    //此处插入18的间隔，是为了在登录和锁屏多用户切换时，绘制选中的样式（一个92*4的圆角矩形，距离阴影下边间隔14像素）
    mainLayout->addSpacing(18);
    mainLayout->addStretch();

    setLayout(mainLayout);
}

//初始化槽函数连接
void UserLoginWidget::initConnect()
{
    connect(m_passwordEdit, &QLineEdit::textChanged, this, [ = ](const QString & value) {
        FrameDataBind::Instance()->updateValue(tr("UserLoginPassword"), value);
    });
    connect(m_passwordEdit, &DPasswordEditEx::returnPressed, this, [ = ] {
        const QString passwd = m_passwordEdit->text();
        if (passwd.isEmpty()) return;
        emit requestAuthUser(passwd);
    });

    connect(m_lockButton, &QPushButton::clicked, this, [ = ] {
        QString password = m_passwordEdit->text();

        if (m_passwordEdit->isVisible())
        {
            m_passwordEdit->setFocus();
        }

        if (password.isEmpty() && m_showType != NoPasswordType) return;
        emit requestAuthUser(password);
    });
    connect(m_userAvatar, &UserAvatar::clicked, this, &UserLoginWidget::clicked);

    connect(m_kbLayoutWidget, &KbLayoutWidget::setButtonClicked, this, &UserLoginWidget::receiveUserKBLayoutChanged);
    //鼠标点击切换键盘布局，就将DArrowRectangle隐藏掉
    connect(m_kbLayoutWidget, &KbLayoutWidget::setButtonClicked, m_kbLayoutBorder, &DArrowRectangle::hide);
    //大小写锁定状态改变
    connect(m_capslockMonitor, &KeyboardMonitor::capslockStatusChanged, this, &UserLoginWidget::capslockStatusChanged);
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
void UserLoginWidget::setUserAvatarSize(const AvatarSize &avatarSize)
{
    if (avatarSize == AvatarSmallSize) {
        m_userAvatar->setAvatarSize(m_userAvatar->AvatarSmallSize);
    } else if (avatarSize == AvatarNormalSize) {
        m_userAvatar->setAvatarSize(m_userAvatar->AvatarNormalSize);
    } else {
        m_userAvatar->setAvatarSize(m_userAvatar->AvatarLargeSize);
    }
    m_userAvatar->setFixedSize(avatarSize, avatarSize);
}

void UserLoginWidget::setWidgetWidth(int width)
{
    this->setFixedWidth(width);
}

void UserLoginWidget::setIsLogin(bool isLogin)
{
    m_isLogin = isLogin;
    m_loginLabel->setVisible(isLogin);
    if (m_isLogin) {
        m_nameFrame->setContentsMargins(0, 0, Margins, 0);
    } else {
        m_nameFrame->setContentsMargins(0, 0, 0, 0);
    }
}

bool UserLoginWidget::getIsLogin()
{
    return  m_isLogin;
}

bool UserLoginWidget::getSelected()
{
    return  m_isSelected;
}

void UserLoginWidget::updateKBLayout(const QStringList &list)
{
    m_kbLayoutWidget->updateButtonList(list);
    m_kbLayoutBorder->setContent(m_kbLayoutWidget);
}

void UserLoginWidget::setDefaultKBLayout(const QString &layout)
{
    m_kbLayoutWidget->setDefault(layout);
}

void UserLoginWidget::hideKBLayout()
{
    m_kbLayoutBorder->hide();
}

void UserLoginWidget::setKBLayoutList(QStringList kbLayoutList)
{
    m_KBLayoutList = kbLayoutList;
    if (kbLayoutList.size() > 1) {
        //连接点击键盘布局Action槽函数
        connect(m_KBAction, &QAction::triggered, this, &UserLoginWidget::toggleKBLayoutWidget);
    } else {
        //解除点击键盘布局Action槽函数
        disconnect(m_KBAction, &QAction::triggered, this, &UserLoginWidget::toggleKBLayoutWidget);
    }
}

void UserLoginWidget::setDefKBLayout(QString defKBLayout)
{
    m_defkBLayout = defKBLayout;
}

void UserLoginWidget::setSelected(bool isSelected)
{
    m_isSelected = isSelected;
    update();
}
