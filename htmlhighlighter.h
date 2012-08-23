#ifndef HTMLHIGHLIGHTER_H
#define HTMLHIGHLIGHTER_H

#include <QSyntaxHighlighter>

class tdHtmlHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    enum Construct {
        Entity,
        Tag,
        Comment,
        LastConstruct = Comment
    };

    tdHtmlHighlighter(QTextDocument *document);

    void setFormatFor(Construct construct, const QTextCharFormat &format);

    inline QTextCharFormat formatFor(Construct construct) const
    { return m_formats[construct]; }

protected:
    enum State {
        NormalState = -1,
        InComment,
        InTag
    };

    void highlightBlock(const QString &text);

private:
    QTextCharFormat m_formats[LastConstruct + 1];
};

#endif // HTMLHIGHLIGHTER_H
