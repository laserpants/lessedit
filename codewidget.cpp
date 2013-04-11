#include <QApplication>
#include <QFont>
#include <QPainter>
#include <QTextBlock>
#include <QMimeData>
#include <QUrl>
#include <QDebug>
#include "codewidget.h"

tdCodeWidget::tdCodeWidget(QWidget *parent)
    : QPlainTextEdit(parent),
      m_lineNumberWidget(new tdCodeWidgetLineNumbers(this)),
      m_paintLineNumbers(true)
{
    setFont(QFont("monospace"));

    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(updateLineNumberArea(QRect, int)));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    setWordWrapMode(QTextOption::NoWrap);
    setTabStopWidth(fontMetrics().width(QLatin1Char('X')) * 4);
}

int tdCodeWidget::lineNumberAreaWidth() const
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    int space = 6 + fontMetrics().width(QLatin1Char('9')) * qMax(digits, 2);
    return space;
}

bool tdCodeWidget::canInsertFromMimeData(const QMimeData *source) const
{
    if (source->hasUrls() && (1 == source->urls().count()))
        return true;
    return QPlainTextEdit::canInsertFromMimeData(source);
}

void tdCodeWidget::insertFromMimeData(const QMimeData *source)
{
    if (source->hasUrls())
        emit loadFileRequest(source->urls().first().toEncoded());
    QPlainTextEdit::insertFromMimeData(source);
}

void tdCodeWidget::setWordWrapEnabled(bool enabled)
{
    setWordWrapMode(enabled ? QTextOption::WordWrap : QTextOption::NoWrap);
}

void tdCodeWidget::setLineNumbersEnabled(bool enabled)
{
    m_paintLineNumbers = enabled;
    setViewportMargins(enabled ? lineNumberAreaWidth() : 0, 0, 0, 0);
    update();
}

void tdCodeWidget::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    QRect cr = contentsRect();
    m_lineNumberWidget->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void tdCodeWidget::changeEvent(QEvent *e)
{
    if (QEvent::FontChange == e->type())
        m_lineNumberWidget->setFont(font());
    QPlainTextEdit::changeEvent(e);
}

void tdCodeWidget::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor(Qt::yellow).lighter(160);
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    setExtraSelections(extraSelections);
}

void tdCodeWidget::updateLineNumberAreaWidth(int)
{
    setViewportMargins(m_paintLineNumbers ? lineNumberAreaWidth() : 0, 0, 0, 0);
}

void tdCodeWidget::updateLineNumberArea(QRect rect, int dy)
{
    if (dy)
        m_lineNumberWidget->scroll(0, dy);
    else
        m_lineNumberWidget->update(0, rect.y(), m_lineNumberWidget->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void tdCodeWidget::paintLineNumbers(QPaintEvent *event)
{
    if (!m_paintLineNumbers)
        return;

    QPainter painter(m_lineNumberWidget);
    painter.fillRect(event->rect(), QColor::fromRgb(238, 238, 238));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::darkGray);
            painter.drawText(4, top, m_lineNumberWidget->width() - 8, fontMetrics().height(),
                             Qt::AlignRight, number);
        }
        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}
