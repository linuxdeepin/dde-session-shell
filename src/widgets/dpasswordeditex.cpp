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

#include "dpasswordeditex.h"

#include <QPainter>
#include <QPropertyAnimation>
#include <DStyleHelper>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFontMetrics>

#define KB_MARGIN 5

LoadSlider::LoadSlider(QWidget *parent)
    : QWidget(parent)
    , m_loadSliderColor(Qt::gray)
{
}

void LoadSlider::setLoadSliderColor(const QColor &color)
{
    m_loadSliderColor = color;
    update();
}

void LoadSlider::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QLinearGradient grad(0, height() / 2, width(), height() / 2);
    grad.setColorAt(0.0, Qt::transparent);
    grad.setColorAt(1.0, m_loadSliderColor);
    painter.fillRect(0, 1, width(), height() - 2, grad);

    QWidget::paintEvent(event);
}

DPasswordEditEx::DPasswordEditEx(QWidget *parent)
    : DLineEdit(parent)
{
    initUI();
    initAnimation();
}

void DPasswordEditEx::initUI()
{
    //设置密码框文本显示模式
    setEchoMode(QLineEdit::Password);
    //禁用DLineEdit类的删除按钮
    setClearButtonEnabled(false);

    //设置键盘和大小写锁定按钮
    m_KBButton = new QPushButton(this);
    m_KBButton->setFocusPolicy(Qt::NoFocus);
    m_KBButton->setCursor(Qt::ArrowCursor);
    m_KBButton->hide();

    m_capsButton = new QPushButton(this);
    m_capsButton->setFocusPolicy(Qt::NoFocus);
    m_capsButton->setCursor(Qt::ArrowCursor);
    m_capsButton->setFlat(true);
    m_capsButton->setIcon(QIcon(":/icons/dedpin/builtin/caps_lock.svg"));
    m_capsButton->setFixedWidth(m_capsButton->iconSize().width());

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(m_KBButton);
    layout->addStretch();
    layout->addWidget(m_capsButton);
    layout->setContentsMargins(KB_MARGIN, KB_MARGIN, 8, KB_MARGIN);
    lineEdit()->setLayout(layout);

    connect(lineEdit(), &QLineEdit::textChanged, this, &DPasswordEditEx::onTextChanged);
}

void DPasswordEditEx::initAnimation()
{
    //初始化动画
    m_loadSlider = new LoadSlider(this);
    m_loadSlider->setLoadSliderColor(QColor(255, 255, 255, 90));
    m_loadSlider->hide();
    m_loadSliderAnim = new QPropertyAnimation(m_loadSlider, "pos", m_loadSlider);
    m_loadSliderAnim->setDuration(1000);
    m_loadSliderAnim->setLoopCount(-1);
    m_loadSliderAnim->setEasingCurve(QEasingCurve::Linear);

    m_loadAnimEnable = true;
    m_isLoading = false;

    m_clipEffectWidget = new DClipEffectWidget(this);
    connect(this,  &DPasswordEditEx::returnPressed, this, &DPasswordEditEx::inputDone);
}

void DPasswordEditEx::setKBLayoutList(QStringList kbLayoutList)
{
    m_KBLayoutList = kbLayoutList;
    if (kbLayoutList.size() > 1) {
        //连接点击键盘布局Action槽函数
        connect(m_KBButton, &QPushButton::clicked, this, &DPasswordEditEx::toggleKBLayoutWidget);
    } else {
        //解除点击键盘布局Action槽函数
        disconnect(m_KBButton, &QPushButton::clicked, this, &DPasswordEditEx::toggleKBLayoutWidget);
    }
}

void DPasswordEditEx::receiveUserKBLayoutChanged(const QString &layout)
{
    QString layoutName = layout;
    layoutName = layoutName.split(";").first();

    if (m_KBLayoutList.size() == 1) {
        layoutName = "";
        m_KBButton->hide();
        lineEdit()->setTextMargins(KB_MARGIN, 0, m_capsButton->width() + KB_MARGIN, 0);
    } else {
        m_KBButton->show();
        QImage image = generateImageFromString(layoutName);
        m_KBButton->setIcon(QIcon(QPixmap::fromImage(image)));
    }
}

QImage DPasswordEditEx::generateImageFromString(const QString &name)
{
    QFont font = this->font();
    font.setPixelSize(32);
    font.setWeight(QFont::DemiBold);
    int word_size = QFontMetrics(font).height();
    qreal device_ratio = this->devicePixelRatioF();
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

void DPasswordEditEx::capslockStatusChanged(bool on)
{
    m_capsButton->setVisible(on);
    updateTextMargins();
}

void DPasswordEditEx::inputDone()
{
    this->hideAlertMessage();

    QString input = this->text();
    if (input.length() > 0) {
        showLoadSlider();
    }
}

void DPasswordEditEx::showLoadSlider()
{
    if (m_loadAnimEnable) {
        if (!m_isLoading) {
            m_isLoading = true;
            m_loadSlider->show();
            m_loadSlider->setGeometry(0, 0, LoadSliderWidth, this->height());
            m_loadSliderAnim->setStartValue(QPoint(0 - LoadSliderWidth, 0));
            m_loadSliderAnim->setEndValue(QPoint(this->width(), 0));
            lineEdit()->setEnabled(false);

            m_loadSliderAnim->start();
        }
    }
}

void DPasswordEditEx::hideLoadSlider()
{
    if (m_isLoading) {
        m_isLoading = false;
        m_loadSliderAnim->stop();
        m_loadSlider->hide();
        lineEdit()->setEnabled(true);
        lineEdit()->setFocus();
    }
}

void DPasswordEditEx::updateTextMargins()
{
    int kbtn_width = m_KBButton->isHidden() ? 0 : m_KBButton->width();
    int kbtn_height = m_capsButton->isHidden()  && m_showCaps ? 0 : m_capsButton->width();
    lineEdit()->setTextMargins(kbtn_width, 0, kbtn_height, 0);
}

void DPasswordEditEx::onTextChanged(const QString &text)
{
    QFontMetrics fm = lineEdit()->fontMetrics();
    QMargins margins = lineEdit()->textMargins();

    int text_width = lineEdit()->width() - margins.left() - margins.right();
    int width = fm.width(text);
    if (width >= text_width / 2) {
        m_showCaps = true;
    } else {
        m_showCaps = false;
    }
    updateTextMargins();
}

//重写QLineEdit paintEvent函数，实现当文本设置居中后，holderText仍然显示的需求
void DPasswordEditEx::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    if (lineEdit()->hasFocus() && lineEdit()->alignment() == Qt::AlignCenter
            && !lineEdit()->placeholderText().isEmpty() && lineEdit()->text().isEmpty()) {
        QPainter pa(this);
        QPalette pal = palette();
        QColor col = pal.text().color();
        col.setAlpha(128);
        QPen oldpen = pa.pen();
        pa.setPen(col);
        QTextOption option;
        option.setAlignment(Qt::AlignCenter);
        pa.drawText(lineEdit()->rect(), lineEdit()->placeholderText(), option);
    }
}

void DPasswordEditEx::resizeEvent(QResizeEvent *event)
{
    DLineEdit::resizeEvent(event);

    QPainterPath path;
    DStyleHelper dstyle(style());
    const int round = dstyle.pixelMetric(DStyle::PM_FrameRadius);
    path.addRoundedRect(rect(), round, round);
    m_clipEffectWidget->setClipPath(path);

    int kb_height = lineEdit()->height() - KB_MARGIN * 2;
    m_KBButton->setFixedSize(kb_height, kb_height);
    m_capsButton->setFixedWidth(kb_height);

    updateTextMargins();
}
