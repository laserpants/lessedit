#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QTextCursor>
#include <QToolBar>

class tdToolCursor : public QTextCursor
{
public:
    tdToolCursor(const QTextCursor &cursor);
    void takeOne();
    void prependSpaces(int s = 4);
    void stripLeading(QChar ch);
    void stripCharAtPos(int pos, QChar ch);
    int charCount(QChar ch) const;
    void blockquote();
    void linebreak();
};

class QAction;
class QToolButton;
class tdCodeWidget;

class tdToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit tdToolBar(tdCodeWidget *widget, QWidget *parent = 0);
    ~tdToolBar() {}

public slots:
    void refreshButtonStatus();

protected:
    bool eventFilter(QObject *object, QEvent *event);

protected slots:
    void insertHash();
    void removeHash();
    void makeCode();
    void emphasize();
    void makeStrong();
    void removeCode();
    void makeBlockquote();
    void insertLineBreak();

private:
    void makeCodeIndentation(bool indent = true);
    void wrap(QChar ch, int t = 1);

    tdCodeWidget *const editor;
    QAction      *const hashAction;
    QAction      *const codeAction;
    QAction      *const emphasizeAction;
    QAction      *const strongAction;
    QAction      *const blockquoteAction;
    QAction      *const linebreakAction;
    QAction      *const uncodeAction;
    QToolButton  *const hashActionButton;
};

#endif // TOOLBAR_H
