#ifndef SESSIONBASEWINDOW_H
#define SESSIONBASEWINDOW_H

#include <QFrame>
#include <QVBoxLayout>

class SessionBaseWindow : public QFrame
{
    Q_OBJECT
public:
    explicit SessionBaseWindow(QWidget *parent = nullptr);
    virtual void setLeftBottomWidget(QWidget *const widget) final;
    virtual void setCenterBottomWidget(QWidget *const widget) final;
    virtual void setRightBottomWidget(QWidget *const widget) final;
    virtual void setCenterContent(QWidget *const widget, bool responseResizeEvent = true) final;
    virtual void setCenterTopWidget(QWidget *const widget) final;
    QSize getCenterContentSize();

protected:
    void setTopFrameVisible(bool visible);
    void setBottomFrameVisible(bool visible);
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    void initUI();

protected:
    QFrame *m_centerTopFrame;
    QFrame *m_centerFrame;
    QFrame *m_bottomFrame;
    QVBoxLayout *m_mainLayou;
    QHBoxLayout *m_centerTopLayout;
    QHBoxLayout *m_centerLayout;
    QHBoxLayout *m_leftBottomLayout;
    QHBoxLayout *m_centerBottomLayout;
    QHBoxLayout *m_rightBottomLayout;
    QWidget *m_centerTopWidget;
    QWidget *m_centerWidget;
    QWidget *m_leftBottomWidget;
    QWidget *m_centerBottomWidget;
    QWidget *m_rightBottomWidget;
    bool m_responseResizeEvent;
};

#endif // SESSIONBASEWINDOW_H
