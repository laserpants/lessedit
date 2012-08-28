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
};

class QAction;
class tdCodeWidget;

class tdToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit tdToolBar(tdCodeWidget *widget, QWidget *parent = 0);
    ~tdToolBar() {}

public slots:
    void refreshButtonStatus();

protected slots:
    void insertHash();
    void removeHash();
    void makeCode();
    void emphasize();
    void makeStrong();
    void removeCode();
    void makeBlockquote();

private:
    void makeCodeIndentation(bool indent = true);
    void wrap(QChar ch, int t = 1);

    tdCodeWidget *const editor;
    QAction      *const hashAction;
    QAction      *const unhashAction;
    QAction      *const codeAction;
    QAction      *const emphasizeAction;
    QAction      *const strongAction;
    QAction      *const blockquoteAction;
    QAction      *const uncodeAction;
};

#endif // TOOLBAR_H
