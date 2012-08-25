#include "htmlhighlighter.h"

tdHtmlHighlighter::tdHtmlHighlighter(QTextDocument *document)
    : QSyntaxHighlighter(document)
{
    QTextCharFormat entityFormat;
    entityFormat.setForeground(QColor(0, 168, 0));
    entityFormat.setFontWeight(QFont::Bold);
    setFormatFor(Entity, entityFormat);

    QTextCharFormat tagFormat;
    tagFormat.setForeground(QColor(192, 16, 112));
    tagFormat.setFontWeight(QFont::Bold);
    setFormatFor(Tag, tagFormat);

    QTextCharFormat commentFormat;
    commentFormat.setForeground(QColor(128, 10, 74));
    commentFormat.setFontItalic(true);
    setFormatFor(Comment, commentFormat);

    QTextCharFormat tagAttrFormat;
    tagAttrFormat.setForeground(QColor(255, 96, 15));
    tagAttrFormat.setFontWeight(QFont::Bold);
    setFormatFor(TagAttr, tagAttrFormat);

    QTextCharFormat quoteFormat;
    quoteFormat.setForeground(QColor(0, 168, 0));
    setFormatFor(Quote, quoteFormat);

    QColor linkBlue(0, 10, 255);
    QTextCharFormat quoteUriFormat;
    quoteUriFormat.setForeground(linkBlue);
    quoteUriFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    quoteUriFormat.setUnderlineColor(linkBlue);
    setFormatFor(QuoteUri, quoteUriFormat);
}

void tdHtmlHighlighter::setFormatFor(Construct construct, const QTextCharFormat &format)
{
    m_formats[construct] = format;
    rehighlight();
}

void tdHtmlHighlighter::highlightBlock(const QString &text)
{
    int state = previousBlockState();
    int len = text.length();
    int start = 0;
    int pos = 0;
    QChar quote = QChar::Null;

    while (pos < len) {
        switch (state)
        {
        case NormalState:
        default:
            while (pos < len) {
                QChar ch = text.at(pos);
                if (ch == '<') {
                    if (text.mid(pos, 4) == "<!--") {
                        state = InComment;
                    } else {
                        state = InTag;
                    }
                    break;
                } else if (ch == '&') {
                    start = pos;
                    while (pos < len
                           && text.at(pos++) != ';')
                        ;
                    setFormat(start, pos - start, m_formats[Entity]);
                } else {
                    ++pos;
                }
            }
            break;
        case InComment:
            start = pos;
            while (pos < len) {
                if (text.mid(pos, 3) == "-->") {
                    pos += 3;
                    state = NormalState;
                    break;
                } else {
                    ++pos;
                }
            }
            setFormat(start, pos - start, m_formats[Comment]);
            break;
        case InTag:
            start = pos;
            while (pos < len) {
                QChar ch = text.at(pos);
                if (ch == ' ') {
                    ++pos;
                    state = InTagAttr;
                    break;
                } else if (ch == '>') {
                    ++pos;
                    state = NormalState;
                    break;
                }
                ++pos;
            }
            setFormat(start, pos - start, m_formats[Tag]);
            break;
        case InTagAttr:
            start = pos;
            attr.clear();
            while (pos < len) {
                QChar ch = text.at(pos);
                if (ch == '>') {
                    state = InTag;
                    break;
                } else if (ch == '\'' || ch == '"') {
                    if (quote != QChar::Null) {
                        state = InTag;
                        quote = QChar::Null;
                    } else {
                        quote = ch;
                        ++pos;
                        state = (attrName == "href") ? InQuoteUri : InQuote;
                        break;
                    }
                } else if (ch != ' ' && ch != '=') {
                    attr.append(ch);
                } else {
                    if (!attr.isEmpty())
                        attrName = attr;
                    attr.clear();
                }
                ++pos;
            }
            setFormat(start, pos - start, m_formats[TagAttr]);
            break;
        case InQuote:
        case InQuoteUri:
            int formt = (state == InQuote) ? Quote : QuoteUri;
            start = pos;
            while (pos < len) {
                QChar ch = text.at(pos);
                if (ch == quote) {
                    state = InTagAttr;
                    break;
                }
                ++pos;
            }
            setFormat(start, pos - start, m_formats[formt]);
        } // end switch
    } // end while
    setCurrentBlockState(state);
}
