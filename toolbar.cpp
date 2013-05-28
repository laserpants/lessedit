#include <QTextDocument>
#include <QTextBlock>
#include <QAction>
#include <QToolButton>
#include "toolbar.h"
#include "codewidget.h"

tdToolCursor::tdToolCursor(const QTextCursor &cursor)
    : QTextCursor(cursor)
{
}

void tdToolCursor::takeOne()
{
    movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    removeSelectedText();
}

void tdToolCursor::prependSpaces(int s)
{
    movePosition(QTextCursor::StartOfLine);
    stripCharAtPos(position(), '>');
    int d = charCount(' ');
    while (d++ < s)
        insertText(" ");
}

void tdToolCursor::stripLeading(QChar ch)
{
    movePosition(QTextCursor::StartOfLine);
    int d = charCount(ch);
    if (!d)
        return;
    movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, d);
    removeSelectedText();
}

void tdToolCursor::stripCharAtPos(int pos, QChar ch)
{
    setPosition(pos);
    if (ch == document()->characterAt(position())) {
        movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
        removeSelectedText();
    }
}

int tdToolCursor::charCount(QChar ch) const
{
    int pos = position();
    QTextDocument *doc = document();
    while (ch == doc->characterAt(pos))
        ++pos;
    return (pos - position());
}

void tdToolCursor::blockquote()
{
    movePosition(QTextCursor::StartOfLine);
    if ('>' != document()->characterAt(position()))
        insertText(">");
    else
        movePosition(QTextCursor::Right);
    if (' ' != document()->characterAt(position()))
        insertText(" ");
    int d = charCount(' ');
    if (d > 1) {
        movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, d - 1);
        removeSelectedText();
    }
}

void tdToolCursor::linebreak()
{
    insertText("  \n");
}

tdToolBar::tdToolBar(tdCodeWidget *widget, QWidget *parent)
    : QToolBar(parent),
      editor(widget),
      hashAction(addAction(tr("Hash"))),
      codeAction(addAction(tr("Code"))),
      emphasizeAction(addAction(tr("Emphasize"))),
      strongAction(addAction(tr("Strong"))),
      blockquoteAction(addAction(tr("Blockquote"))),
      linebreakAction(addAction(tr("Insert linebreak"))),
      uncodeAction(addAction(tr("Clear formatting"))),
      hashActionButton(qobject_cast<QToolButton *>(widgetForAction(hashAction)))
{
    uncodeAction->setIcon(QIcon::fromTheme("edit-clear"));
    codeAction->setIcon(QIcon::fromTheme("terminal"));
    emphasizeAction->setIcon(QIcon::fromTheme("text_italic"));
    strongAction->setIcon(QIcon::fromTheme("text_bold"));
    hashAction->setIcon(QIcon(":/marker.png"));
    blockquoteAction->setIcon(QIcon::fromTheme("stock_text_indent"));
    linebreakAction->setIcon(QIcon(":/enter.png"));

    strongAction->setShortcut(QKeySequence("Ctrl+B"));
    emphasizeAction->setShortcut(QKeySequence("Ctrl+I"));

    setMovable(false);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setFloatable(false);

    connect(hashAction, SIGNAL(triggered()), this, SLOT(insertHash()));
    connect(codeAction, SIGNAL(triggered()), this, SLOT(makeCode()));
    connect(emphasizeAction, SIGNAL(triggered()), this, SLOT(emphasize()));
    connect(strongAction, SIGNAL(triggered()), this, SLOT(makeStrong()));
    connect(blockquoteAction, SIGNAL(triggered()), this, SLOT(makeBlockquote()));
    connect(linebreakAction, SIGNAL(triggered()), this, SLOT(insertLineBreak()));
    connect(uncodeAction, SIGNAL(triggered()), this, SLOT(removeCode()));
    connect(editor, SIGNAL(cursorPositionChanged()), this, SLOT(refreshButtonStatus()));
    connect(editor, SIGNAL(selectionChanged()), this, SLOT(refreshButtonStatus()));

    hashActionButton->installEventFilter(this);

    widgetForAction(codeAction)->installEventFilter(this);
    widgetForAction(emphasizeAction)->installEventFilter(this);
    widgetForAction(strongAction)->installEventFilter(this);
    widgetForAction(blockquoteAction)->installEventFilter(this);
}

void tdToolBar::refreshButtonStatus()
{
    tdToolCursor cursor(editor->textCursor());
    QTextDocument *doc = cursor.document();
    bool ml = false;
    bool hs = cursor.hasSelection();
    if (hs) {
        // Is multi-line selection?
        ml = doc->findBlock(cursor.selectionStart()) != doc->findBlock(cursor.selectionEnd());
        emphasizeAction->setEnabled(!ml);
        strongAction->setEnabled(!ml);
    } else {
        emphasizeAction->setEnabled(false);
        strongAction->setEnabled(false);
    }
    hashAction->setEnabled(!hs || !ml);
    linebreakAction->setEnabled(!hs);
}

bool tdToolBar::eventFilter(QObject *object, QEvent *event)
{
    int type = event->type();
    if (QEvent::MouseButtonPress == type || QEvent::MouseButtonRelease == type) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (Qt::RightButton != mouseEvent->button())
            return QToolBar::eventFilter(object, event);
        QToolButton *tb;
        if (hashActionButton == object) {
            tb = hashActionButton;
            if (QEvent::MouseButtonRelease == type)
                removeHash();
        } else {
            tb = qobject_cast<QToolButton *>(object);
            if (QEvent::MouseButtonRelease == type)
                removeCode();
        }
        if (tb) tb->setDown(QEvent::MouseButtonPress == type);
    }
    return QToolBar::eventFilter(object, event);
}

void tdToolBar::insertHash()
{
    tdToolCursor cursor(editor->textCursor());
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::StartOfLine);
    int pos = cursor.position();
    while ('#' == cursor.document()->characterAt(pos++))
        ;
    if ((pos - cursor.position()) < 7) {
        cursor.insertText("#");
        if (' ' != cursor.document()->characterAt(pos))
            cursor.insertText(" ");
        else {
            int d = cursor.charCount(' ');
            if (d > 1) {
                cursor.movePosition(QTextCursor::Right,
                                    QTextCursor::KeepAnchor, d - 1);
                cursor.removeSelectedText();
            }
        }
    }
    cursor.endEditBlock();
    refreshButtonStatus();
}

void tdToolBar::removeHash()
{
    tdToolCursor cursor(editor->textCursor());
    cursor.movePosition(QTextCursor::StartOfLine);
    if ('#' == cursor.document()->characterAt(cursor.position())) {
        cursor.beginEditBlock();
        cursor.takeOne();
        if (' ' == cursor.document()->characterAt(cursor.position()))
            cursor.takeOne();
        cursor.endEditBlock();
    }
    refreshButtonStatus();
}

void tdToolBar::makeCode()
{
    makeCodeIndentation(true);
}

void tdToolBar::emphasize()
{
    wrap('*');
}

void tdToolBar::makeStrong()
{
    wrap('*', 2);
}

void tdToolBar::removeCode()
{
    makeCodeIndentation(false);
}

void tdToolBar::makeBlockquote()
{
    tdToolCursor cursor(editor->textCursor());
    QTextDocument *doc = editor->document();
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();

    if (!cursor.hasSelection()) {
        cursor.beginEditBlock();
        cursor.blockquote();
        cursor.endEditBlock();
    } else {
        cursor.beginEditBlock();
        int endBlock = doc->findBlock(end).blockNumber();
        cursor.setPosition(start);
        int i = cursor.block().blockNumber();
        while (i++ <= endBlock) {
            cursor.blockquote();
            cursor.movePosition(QTextCursor::NextBlock);
        }
        cursor.endEditBlock();
    }
    refreshButtonStatus();
}

void tdToolBar::insertLineBreak()
{
    tdToolCursor cursor(editor->textCursor());
    if (cursor.hasSelection())
        return;
    else
        cursor.linebreak();
    refreshButtonStatus();
}

void tdToolBar::makeCodeIndentation(bool indent)
{
    tdToolCursor cursor(editor->textCursor());
    QTextDocument *doc = editor->document();
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();

    if (!cursor.hasSelection()) {
        cursor.beginEditBlock();
        if (indent) {
            cursor.prependSpaces();
        } else {
            cursor.stripLeading('#');
            cursor.stripLeading('>');
            cursor.stripLeading(' ');
        }
        cursor.endEditBlock();
    } else if (doc->findBlock(start) != doc->findBlock(end)) {
        cursor.beginEditBlock();
        int endBlock = doc->findBlock(end).blockNumber();
        cursor.setPosition(start);
        int i = cursor.block().blockNumber();
        while (i++ <= endBlock) {
            if (indent) {
                cursor.prependSpaces();
            } else {
                cursor.stripLeading('#');
                cursor.stripLeading('>');
                cursor.stripLeading(' ');
            }
            cursor.movePosition(QTextCursor::NextBlock);
        }
        cursor.endEditBlock();
    } else {
        cursor.beginEditBlock();
        int offs = 0;
        if (indent) {
            QString text = cursor.selectedText();
            QString str = text.remove('`');
            if ('`' != doc->characterAt(cursor.selectionStart() - 1)) {
                str.prepend('`');
                ++offs;
            }
            if ('`' != doc->characterAt(cursor.selectionEnd()))
                str.append('`');
            cursor.insertText(str);
        } else {
            cursor.stripCharAtPos(start - 1, '`');
            cursor.stripCharAtPos(end - 1, '`');
            // ---------------------------------
            cursor.stripCharAtPos(start - 1, '*');
            cursor.stripCharAtPos(end - 1, '*');
            cursor.stripCharAtPos(start - 2, '*');
            cursor.stripCharAtPos(end - 2, '*');
        }
        cursor.endEditBlock();
        if (indent) {
            cursor.setPosition(start + offs);
            cursor.movePosition(QTextCursor::Right,
                                QTextCursor::KeepAnchor, end - start);
            editor->setTextCursor(cursor);
        }
    }
    refreshButtonStatus();
}

void tdToolBar::wrap(QChar ch, int t)
{
    tdToolCursor cursor(editor->textCursor());
    QTextDocument *doc = editor->document();
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();
    if (cursor.hasSelection() && doc->findBlock(start)
                              == doc->findBlock(end)) {
        int offs = 0;
        cursor.beginEditBlock();
        QString text = cursor.selectedText();
        QString str = text.remove(ch);
        while (t--) {
            if (ch != doc->characterAt(cursor.selectionStart() - 1)) {
                str.prepend(ch);
                ++offs;
            }
            if (ch != doc->characterAt(cursor.selectionEnd()))
                str.append(ch);
        }
        cursor.insertText(str);
        cursor.endEditBlock();
        cursor.setPosition(start + offs);
        cursor.movePosition(QTextCursor::Right,
                            QTextCursor::KeepAnchor, end - start);
        editor->setTextCursor(cursor);
    }
}

