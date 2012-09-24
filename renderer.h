#ifndef RENDERER_H
#define RENDERER_H

#include <QWebElement>
#include <QUndoStack>
#include <QUndoCommand>
#include <QDebug>
#include "sundown/html.h"

class tdRenderer;

class tdRendererCursorCommand : public QUndoCommand
{
public:
    tdRendererCursorCommand(tdRenderer *renderer, int at, int removed, int added, QUndoCommand *parent = 0);

    void undo();
    void redo();

private:
    tdRenderer *const m_renderer;
    int  m_at;
    int  m_removed;
    int  m_added;
    bool m_fresh;
};

class QPlainTextEdit;
struct buf;
struct sd_markdown;

class tdRenderer : public QObject
{
    Q_OBJECT

public:
    tdRenderer(QPlainTextEdit *editor, int extensions, QWebElement body);
    ~tdRenderer();

    inline QList<int> sizes() const { return m_sizes; }
    inline QList<int> indices() const { return m_indices; }

    inline void undo() { m_isUndoRedo = true; m_undoStack->undo(); }
    inline void redo() { m_isUndoRedo = true; m_undoStack->redo(); }

    inline int extensionsFlags() const { return m_ext; }
    void setExtensionsFlags(int flags);

signals:
    void parsingDone();
    void rendererSettingsChanged();

public slots:
    void setSmartypantsEnabled(bool enabled);
    void refreshAll();

protected slots:
    void updateFrameInterval();
    void parseMarkdown(int at, int removed, int added);

private:
    friend class tdRendererCursorCommand;

    sd_markdown *initSundown();
    int block(int offset) const;
    void render(QByteArray ba);

    int             m_ext;
    QPlainTextEdit *const m_editor;
    QUndoStack     *const m_undoStack;
    buf            *const m_buffer;
    buf            *const m_tmpbuffer;
    sd_markdown    *m_markdown;
    sd_callbacks    m_callbacks;
    html_renderopt  m_options;
    QList<int>      m_sizes;
    QList<int>      m_indices;
    QWebElement     m_body;
    int             m_fframe;
    int             m_lframe;
    int             m_count;
    int             m_index;
    bool            m_pants;
    int             m_undoSteps;
    bool            m_isUndoRedo;
};

#endif // RENDERER_H
