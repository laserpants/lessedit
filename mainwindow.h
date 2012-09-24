#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QMessageBox>
#include <QPaintEvent>
#include <QPainter>
#include <QUrl>

class QCheckBox;

class tdExtensionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit tdExtensionsDialog(QWidget *parent = 0);
    ~tdExtensionsDialog() {}

    void updateCheckBoxes(int flags);
    int flags() const;

private:
    QList<QCheckBox *> m_checkBoxes;
};

class tdAboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit tdAboutDialog(QWidget *parent = 0);
    ~tdAboutDialog() {}
};

class QMenu;
class QAction;
class QSplitter;
class QWebView;
class QTabWidget;
class QTextEdit;
class tdCodeWidget;
class tdToolBar;
class FindReplaceDialog;

struct tdMainWindowUi
{
    enum {
        MaxRecentFiles = 10
    };

    tdMainWindowUi(QMainWindow *mainWindow);

    QTabWidget   *const tabWidget;
    QSplitter    *const splitter;
    QWebView     *const view;
    tdCodeWidget *const editor;
    QTextEdit    *const source;
    tdAboutDialog*const aboutDialog;
    QMenu        *const fileMenu;
    QMenu        *const editMenu;
    QMenu        *const optionsMenu;
    QMenu        *const helpMenu;
    QAction      *const newAction;
    QAction      *const openAction;
    QMenu        *const openRecentMenu;
    QAction      *clearRecentFilesAction;
    QAction      *const saveAction;
    QAction      *const saveAsAction;
    QAction      *const revertAction;
    QAction      *const exportPdfAction;
    QAction      *const exportHtmlAction;
    QAction      *const printAction;
    QAction      *const undoAction;
    QAction      *const redoAction;
    QAction      *const cutAction;
    QAction      *const copyAction;
    QAction      *const pasteAction;
    QAction      *const selectAllAction;
    QAction      *const findAction;
    QAction      *const findNextAction;
    QAction      *const editorModeAction;
    QAction      *const wordWrapAction;
    QAction      *const lineNumbersAction;
    QAction      *const smartypantsAction;
    QAction      *const extensionsAction;
    QAction      *const aboutAction;
    QAction      *const refreshViewAction;
    FindReplaceDialog *const findReplaceDialog;
    tdToolBar    *const toolBar;
    tdExtensionsDialog *const extensionsDialog;
    QAction      *recentFileActs[MaxRecentFiles];
};

class QFile;
class tdHtmlHighlighter;
class tdTidy;
class tdRenderer;

class tdMainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    enum {
        TabEditor = 0,
        TabHtml   = 1
    };

    enum FocusWidget {
        WidgetEditor = 1,
        WidgetView,
        WidgetSource
    };

    explicit tdMainWindow(QWidget *parent = 0);
    ~tdMainWindow();

protected:
    bool eventFilter(QObject *object, QEvent *event);
    void closeEvent(QCloseEvent *event);

protected slots:
    void showEditorContextMenu(const QPoint &pos);
    void showSourceContextMenu(const QPoint &pos);
    void showViewContextMenu(const QPoint &pos);
    void updateStatusBarMessage();
    void refreshTab(int tab);
    void selectAll();
    void updateSource();
    void triggerUndo();
    void triggerRedo();
    void cut();
    void copy();
    void setModificationStatus(bool modified);
    void newFile();
    void openFile();
    void saveFile();
    void saveFileAs();
    void revertFile();
    void exportPdf();
    void exportHtml();
    void loadFile(QString filename, bool confirm = true);
    void setEditorEnabled(bool enabled);
    void openRecentFile();
    void clearRecentFiles();
    void openUrl(QUrl url);
    void showExtensionsDialog();

private:
    void writeToFile(QString name);
    bool confirmSaveIfModified();
    QString filePath() const;
    void updateViewStyle();
    void setToolBarActionsEnabled(bool enabled);
    void updateRecentFilesList();
    void updateRecentFilesActions();
    void readSettings();
    void writeSettings();

    tdMainWindowUi    *const ui;
    tdHtmlHighlighter *const highlighter;
    tdTidy            *const tidy;
    tdRenderer        *const renderer;
    FocusWidget        focus;
    QString            file;
    QString            viewCss;
};

#endif // MAINWINDOW_H
