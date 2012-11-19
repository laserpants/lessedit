#include <QPlainTextEdit>
#include <QTextDocumentFragment>
#include <QStringBuilder>
#include <QDebug>
#include <cassert>
#include "renderer.h"
#include "sundown/markdown.h"

tdRendererCursorCommand::tdRendererCursorCommand(tdRenderer *renderer, int at, int removed, int added, QUndoCommand *parent)
    : QUndoCommand(parent),
      m_renderer(renderer),
      m_at(at),
      m_removed(removed),
      m_added(added),
      m_fresh(true)
{
}

void tdRendererCursorCommand::undo()
{
    QTextCursor cursor = m_renderer->m_editor->textCursor();
    cursor.setPosition(m_at);
    cursor.setPosition(qMin(m_at + m_added, m_renderer->m_count),
                       QTextCursor::KeepAnchor);
    m_renderer->m_editor->setTextCursor(cursor);
}

void tdRendererCursorCommand::redo()
{
    if (m_fresh) {
        m_fresh = false;
    } else {
        QTextCursor cursor = m_renderer->m_editor->textCursor();
        cursor.setPosition(m_at);
        cursor.setPosition(qMin(m_at + m_removed, m_renderer->m_count),
                           QTextCursor::KeepAnchor);
        m_renderer->m_editor->setTextCursor(cursor);
    }
}

tdRenderer::tdRenderer(QPlainTextEdit *editor, int extensions, QWebElement body)
    : QObject(editor),
      m_ext(extensions),
      m_editor(editor),
      m_undoStack(new QUndoStack(this)),
      m_buffer(bufnew(1024)),
      m_tmpbuffer(bufnew(1024)),
      m_markdown(initSundown()),
      m_body(body),
      m_fframe(0),
      m_lframe(0),
      m_count(0),
      m_index(1),
      m_pants(true),
      m_undoSteps(0),
      m_isUndoRedo(false)
{
    connect(m_editor, SIGNAL(cursorPositionChanged()), this, SLOT(updateFrameInterval()));
    connect(m_editor, SIGNAL(selectionChanged()), this, SLOT(updateFrameInterval()));
    connect(editor->document(), SIGNAL(contentsChange(int, int, int)),
            this, SLOT(parseMarkdown(int, int, int)));

    updateFrameInterval();
}

tdRenderer::~tdRenderer()
{
    bufrelease(m_buffer);
    bufrelease(m_tmpbuffer);
    sd_markdown_free(m_markdown);
}

void tdRenderer::setExtensionsFlags(int flags)
{
    sd_markdown_free(m_markdown);
    m_ext = flags;
    m_markdown = sd_markdown_new(m_ext, 16, &m_callbacks, &m_options);
    refreshAll();
    emit rendererSettingsChanged();
}

void tdRenderer::setSmartypantsEnabled(bool enabled)
{
    m_pants = enabled;
    refreshAll();
    emit rendererSettingsChanged();
}

void tdRenderer::updateFrameInterval()
{
    QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection()) {
        m_fframe = block(cursor.selectionStart());
        m_lframe = block(cursor.selectionEnd());
    } else {
        m_fframe = m_lframe = block(cursor.position());
    }

    if (m_fframe != 0)
        --m_fframe;
    if (m_lframe + 1 < m_sizes.count())
        ++m_lframe;
}

void tdRenderer::refreshAll()
{
    m_sizes.clear();
    m_indices.clear();
    m_body.setInnerXml("<div class=\"__tmp__\"></div>");

    int fframe = m_fframe;
    m_fframe = 0;
    render(m_editor->toPlainText().toAscii());
    m_fframe = fframe;
}

void tdRenderer::parseMarkdown(int at, int removed, int added)
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.beginEditBlock();

    int steps = m_editor->document()->availableUndoSteps();
    bool undoStepsChanged = (steps != m_undoSteps);
    m_undoSteps = steps;

    if (undoStepsChanged && !m_isUndoRedo)
        m_undoStack->push(new tdRendererCursorCommand(this, at, removed, added));
    m_isUndoRedo = false;

    int start;
    int end;
    if (m_sizes.isEmpty()) {
        start = end = 0;
    } else {
        int i = 0, n = 0;
        while (i < m_fframe)
            n += m_sizes.at(i++);
        start = n;
        while (i <= m_lframe)
            n += m_sizes.at(i++);
        end = n;
    }

    int diff = added - removed;
    m_count += diff;
    if (m_sizes.isEmpty()) end = m_count;
    else end += diff;

    cursor.setPosition(start);
    cursor.setPosition(qMin(end, m_count), QTextCursor::KeepAnchor);

    int c = m_fframe;
    int klass = 0;
    QWebElementCollection collection;
    while (c++ <= m_lframe && !m_sizes.isEmpty()) {
        m_sizes.takeAt(m_fframe);
        klass = m_indices.takeAt(m_fframe);
        collection.append(m_body.findAll(".__" % QString::number(klass) % "__"));
    }
    QList<QWebElement> list = collection.toList();
    QWebElement element;

    if (klass) {
        QString k = "__" % QString::number(klass) % "__";
        element = list.last();
        while (element.parent().hasClass(k))
            element = element.parent();
        list.removeAll(element);
        element.setOuterXml("<div class=\"__tmp__\"></div>");

        QList<QWebElement>::iterator i = list.begin();
        for (; i != list.end(); ++i)
            i->takeFromDocument();
    } else {
        m_body.prependInside("<div class=\"__tmp__\"></div>");
    }

    render(cursor.selection().toPlainText().toAscii());

    cursor.endEditBlock();
    updateFrameInterval();
    emit parsingDone();
}

sd_markdown *tdRenderer::initSundown()
{
    sdhtml_renderer(&m_callbacks, &m_options, 0);
    return sd_markdown_new(m_ext, 16, &m_callbacks, &m_options);
}

int tdRenderer::block(int offset) const
{
    int n = 0, i;
    for (i = 0; i < m_sizes.count(); ++i) {
        n += m_sizes.at(i);
        if (offset < n)
            return i;
    }
    return i ? (i - 1) : 0;
}

void tdRenderer::render(QByteArray ba)
{
    bufreset(m_buffer);
    QWebElement element = m_body.findFirst(".__tmp__");

    const char *data = ba.data();
    uint beg = 0;
    size_t e = ba.size();
    int prevsize = 0;
    int pos = m_fframe;

    while (beg < e) {
        const char *offs = data + beg;
        int n = td_markdown_render(m_buffer, (const uint8_t *) offs,
                                   e - beg, m_markdown);

        QByteArray bytes((const char *) m_buffer->data + prevsize, m_buffer->size - prevsize);

        m_sizes.insert(pos, n);
        m_indices.insert(pos++, m_index);

        if (m_pants) {
            bufreset(m_tmpbuffer);
            sdhtml_smartypants(m_tmpbuffer, (const uint8_t *) bytes.constData(), bytes.size());
            QByteArray pants((const char *) m_tmpbuffer->data, m_tmpbuffer->size);
            element.appendInside(pants);
        } else {
            element.appendInside(bytes);
        }

        QWebElementCollection children = element.findAll("*");
        QString klassName = "__" % QString::number(m_index++) % "__";
        QWebElementCollection::const_iterator i = children.constBegin();
        for (; i != children.constEnd(); ++i) {
            QWebElement e = *i;
            e.addClass(klassName);
            if (!e.parent().hasClass(klassName))
                element.prependOutside(e.takeFromDocument());
        }
        if (m_body.findFirst("." % klassName).isNull())
            element.prependOutside("<span class=\"" % klassName % "\"></span>");

        beg += n;
        prevsize = m_buffer->size;
    }
    element.takeFromDocument();
}
