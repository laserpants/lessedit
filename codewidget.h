#ifndef CODEWIDGET_H
#define CODEWIDGET_H

#include <QPlainTextEdit>

class tdCodeWidgetLineNumbers;

class tdCodeWidget : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit tdCodeWidget(QWidget *parent = 0);

    int lineNumberAreaWidth() const;

public slots:
    void setWordWrapEnabled(bool enabled);
    void setLineNumbersEnabled(bool enabled);

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void highlightCurrentLine();
    void updateLineNumberAreaWidth(int);
    void updateLineNumberArea(QRect rect, int dy);

private:
    friend class tdCodeWidgetLineNumbers;
    void paintLineNumbers(QPaintEvent *event);

    tdCodeWidgetLineNumbers *const m_lineNumberWidget;
    bool m_paintLineNumbers;
};

class tdCodeWidgetLineNumbers : public QWidget
{
    Q_OBJECT

public:
    explicit tdCodeWidgetLineNumbers(tdCodeWidget *parent)
        : QWidget(parent),
          m_widget(parent)
    {
    }

    inline QSize sizeHint() const
    { return QSize(m_widget->lineNumberAreaWidth(), 0); }

protected:
    inline void paintEvent(QPaintEvent *event)
    { m_widget->paintLineNumbers(event); }

private:
    tdCodeWidget *const m_widget;
};

#endif // CODEWIDGET_H
