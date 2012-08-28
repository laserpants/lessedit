#include <QMenuBar>
#include <QMenu>
#include <QSplitter>
#include <QStatusBar>
#include <QWebView>
#include <QTableWidget>
#include <QWebElement>
#include <QWebFrame>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTextDocument>
#include <QFileDialog>
#include <QPrintDialog>
#include <QPrinter>
#include <QMessageBox>
#include <QApplication>
#include <QSettings>
//#include <QFontDialog>
#include <QDebug>
#include "mainwindow.h"
#include "codewidget.h"
#include "htmlhighlighter.h"
#include "tidy.h"
#include "renderer.h"
#include "findreplace/findreplacedialog.h"
#include "toolbar.h"

tdAboutDialog::tdAboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("About"));
    setFixedWidth(300);

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    QLabel *img = new QLabel;
    img->setPixmap(QPixmap(":/icon.png"));
    img->setAlignment(Qt::AlignCenter);
    img->setContentsMargins(24, 0, 0, 0);

    QLabel *label = new QLabel;
    label->setText("<h1>LessEdit 0.95</h1><p>A WYSIWYG markdown editor.<br>&copy; 2012 Johannes Hild&eacute;n</p><p><a href=\"https://github.com/johanneshilden/lessedit\">https://github.com/johanneshilden/lessedit</a></p><h3>Credits &amp; License");
    label->setAlignment(Qt::AlignCenter);

    QTextEdit *credits = new QTextEdit;
    credits->setReadOnly(true);

    QFile file(":/credits.txt");
    if (file.open(QFile::ReadOnly))
        credits->setHtml(file.readAll());

    QHBoxLayout *hlayout = new QHBoxLayout;

    layout->addWidget(img);
    layout->addWidget(label);
    layout->addWidget(credits);
    layout->addLayout(hlayout);

    QPushButton *okButton = new QPushButton(tr("Ok"));
    hlayout->addStretch();
    hlayout->addWidget(okButton);
    hlayout->addStretch();

    connect(okButton, SIGNAL(clicked()), this, SLOT(close()));
}

tdMainWindowUi::tdMainWindowUi(QMainWindow *mainWindow)
    : tabWidget(new QTabWidget),
      splitter(new QSplitter(Qt::Horizontal)),
      view(new QWebView),
      editor(new tdCodeWidget),
      source(new QTextEdit),
      aboutDialog(new tdAboutDialog(mainWindow)),
      fileMenu(mainWindow->menuBar()->addMenu(QObject::tr("&File"))),
      editMenu(mainWindow->menuBar()->addMenu(QObject::tr("&Edit"))),
      optionsMenu(mainWindow->menuBar()->addMenu(QObject::tr("&Options"))),
      helpMenu(mainWindow->menuBar()->addMenu(QObject::tr("&Help"))),
      newAction(fileMenu->addAction(QObject::tr("&New"))),
      openAction(fileMenu->addAction(QObject::tr("&Open"))),
      openRecentMenu(fileMenu->addMenu(QObject::tr("Open &Recent"))),
      saveAction(fileMenu->addAction(QObject::tr("&Save"))),
      saveAsAction(fileMenu->addAction(QObject::tr("Save &As"))),
      exportPdfAction(fileMenu->addAction(QObject::tr("Export as P&DF"))),
      exportHtmlAction(fileMenu->addAction(QObject::tr("Export as &HTML"))),
      printAction(fileMenu->addAction(QObject::tr("&Print"))),
      undoAction(editMenu->addAction(QObject::tr("&Undo"))),
      redoAction(editMenu->addAction(QObject::tr("&Redo"))),
      cutAction(editMenu->addAction(QObject::tr("Cu&t"))),
      copyAction(editMenu->addAction(QObject::tr("&Copy"))),
      pasteAction(editMenu->addAction(QObject::tr("&Paste"))),
      selectAllAction(editMenu->addAction(QObject::tr("Select &All"))),
      findAction(editMenu->addAction(QObject::tr("&Find/Replace"))),
      findNextAction(editMenu->addAction(QObject::tr("Find &Next"))),
      editorModeAction(optionsMenu->addAction(QObject::tr("Show Editor"))),
      wordWrapAction(optionsMenu->addAction(QObject::tr("&Word Wrap"))),
      lineNumbersAction(optionsMenu->addAction(QObject::tr("&Line Numbers"))),
      smartypantsAction(optionsMenu->addAction(QObject::tr("&SmartyPants"))),
      //changeFontAction(optionsMenu->addAction(QObject::tr("Change &Font"))),
      aboutAction(helpMenu->addAction(QObject::tr("&About"))),
      findReplaceDialog(new FindReplaceDialog(mainWindow)),
      toolBar(new tdToolBar(editor))
{
    findReplaceDialog->setTextEdit(editor);
    findReplaceDialog->setModal(true);

    QFont font = editor->font();
    font.setPointSize(11);
    editor->setFont(font);
    mainWindow->setCentralWidget(tabWidget);
    tabWidget->setTabPosition(QTabWidget::West);

    view->setHtml("<html><head><style></style></head><body></body></html>");
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setAcceptDrops(false);
    mainWindow->connect(view, SIGNAL(customContextMenuRequested(QPoint)),
                        mainWindow, SLOT(showViewContextMenu(QPoint)));

    source->setContextMenuPolicy(Qt::CustomContextMenu);
    mainWindow->connect(source, SIGNAL(customContextMenuRequested(QPoint)),
                        mainWindow, SLOT(showSourceContextMenu(QPoint)));

    splitter->addWidget(editor);
    splitter->addWidget(view);

    int w = splitter->width()/2;
    QList<int> sizelist;
    splitter->setSizes(sizelist << w << w);

    source->setReadOnly(true);
    source->setFont(QFont("monospace"));

    tabWidget->addTab(splitter, QObject::tr("Markdown"));
    tabWidget->addTab(source, QObject::tr("HTML Source"));

    editor->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(editor, SIGNAL(customContextMenuRequested(QPoint)),
                     mainWindow, SLOT(showEditorContextMenu(QPoint)));
    editor->setAcceptDrops(true);

    /* File menu */

    fileMenu->removeAction(printAction);        // temp
//    fileMenu->insertSeparator(printAction);
    fileMenu->insertSeparator(saveAction);
    fileMenu->insertSeparator(exportPdfAction);
    fileMenu->addSeparator();

    newAction->setIcon(QIcon::fromTheme("document-new"));
    newAction->setIconVisibleInMenu(true);
    openAction->setIcon(QIcon::fromTheme("document-open"));
    openAction->setIconVisibleInMenu(true);
    saveAction->setIcon(QIcon::fromTheme("document-save"));
    saveAction->setIconVisibleInMenu(true);
    saveAsAction->setIcon(QIcon::fromTheme("document-save"));
    saveAsAction->setIconVisibleInMenu(true);
    printAction->setIcon(QIcon::fromTheme("document-print"));
    printAction->setIconVisibleInMenu(true);
//    exportPdfAction->setIcon(QIcon::fromTheme("pdf"));
//    exportPdfAction->setIconVisibleInMenu(true);

    saveAction->setEnabled(false);

    newAction->setShortcut(QKeySequence("Ctrl+N"));
    saveAction->setShortcut(QKeySequence("Ctrl+S"));
    openAction->setShortcut(QKeySequence("Ctrl+O"));
    printAction->setShortcut(QKeySequence("Ctrl+P"));

    mainWindow->connect(newAction, SIGNAL(triggered()), mainWindow, SLOT(newFile()));
    mainWindow->connect(openAction, SIGNAL(triggered()), mainWindow, SLOT(openFile()));
    mainWindow->connect(saveAction, SIGNAL(triggered()), mainWindow, SLOT(saveFile()));
    mainWindow->connect(saveAsAction, SIGNAL(triggered()), mainWindow, SLOT(saveFileAs()));
    //mainWindow->connect(printAction, SIGNAL(triggered()), mainWindow, SLOT(print()));
    mainWindow->connect(exportPdfAction, SIGNAL(triggered()), mainWindow, SLOT(exportPdf()));
    mainWindow->connect(exportHtmlAction, SIGNAL(triggered()), mainWindow, SLOT(exportHtml()));

    QAction *exitAction = fileMenu->addAction(QObject::tr("E&xit"));
    exitAction->setShortcut(QKeySequence("Ctrl+Q"));


    exitAction->setIcon(QIcon::fromTheme("exit"));
    exitAction->setIconVisibleInMenu(true);

    mainWindow->connect(exitAction, SIGNAL(triggered()), mainWindow, SLOT(close()));

    /* Recent file actions */

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActs[i] = openRecentMenu->addAction("");
        //recentFileActs[i] = new QAction(mainWindow);
        recentFileActs[i]->setVisible(false);
        recentFileActs[i]->setIcon(QIcon::fromTheme("ascii"));
        recentFileActs[i]->setIconVisibleInMenu(true);
        mainWindow->connect(recentFileActs[i], SIGNAL(triggered()),
                            mainWindow, SLOT(openRecentFile()));
    }
    openRecentMenu->addSeparator();
    clearRecentFilesAction = openRecentMenu->addAction(QObject::tr("&Clear list"));
    mainWindow->connect(clearRecentFilesAction, SIGNAL(triggered()), mainWindow, SLOT(clearRecentFiles()));

    /* Edit menu */

    editMenu->insertSeparator(cutAction);
    editMenu->insertSeparator(selectAllAction);
    editMenu->insertSeparator(findAction);

    cutAction->setShortcut(QKeySequence("Ctrl+X"));
    copyAction->setShortcut(QKeySequence("Ctrl+C"));
    pasteAction->setShortcut(QKeySequence("Ctrl+V"));

    cutAction->setIcon(QIcon::fromTheme("edit-cut"));
    cutAction->setIconVisibleInMenu(true);
    copyAction->setIcon(QIcon::fromTheme("edit-copy"));
    copyAction->setIconVisibleInMenu(true);
    pasteAction->setIcon(QIcon::fromTheme("edit-paste"));
    pasteAction->setIconVisibleInMenu(true);
    selectAllAction->setIcon(QIcon::fromTheme("stock_select-all"));
    selectAllAction->setIconVisibleInMenu(true);

    mainWindow->connect(editor, SIGNAL(copyAvailable(bool)), copyAction, SLOT(setEnabled(bool)));
    mainWindow->connect(editor, SIGNAL(copyAvailable(bool)), cutAction, SLOT(setEnabled(bool)));

    mainWindow->connect(cutAction, SIGNAL(triggered()), mainWindow, SLOT(cut()));
    mainWindow->connect(copyAction, SIGNAL(triggered()), mainWindow, SLOT(copy()));
    mainWindow->connect(pasteAction, SIGNAL(triggered()), editor, SLOT(paste()));

    selectAllAction->setShortcut(QKeySequence("Ctrl+A"));

    undoAction->setIcon(QIcon::fromTheme("edit-undo"));
    undoAction->setIconVisibleInMenu(true);

    redoAction->setIcon(QIcon::fromTheme("edit-redo"));
    redoAction->setIconVisibleInMenu(true);

    mainWindow->connect(selectAllAction, SIGNAL(triggered()), mainWindow, SLOT(selectAll()));

    undoAction->setShortcut(QKeySequence("Ctrl+Z"));
    redoAction->setShortcut(QKeySequence("Ctrl+Shift+Z"));

    findAction->setShortcut(QKeySequence("Ctrl+F"));
    findNextAction->setShortcut(QKeySequence("F3"));

    findAction->setIcon(QIcon::fromTheme("edit-find"));
    findAction->setIconVisibleInMenu(true);

    mainWindow->connect(editor, SIGNAL(undoAvailable(bool)), undoAction, SLOT(setEnabled(bool)));
    mainWindow->connect(editor, SIGNAL(redoAvailable(bool)), redoAction, SLOT(setEnabled(bool)));
    mainWindow->connect(undoAction, SIGNAL(triggered()), mainWindow, SLOT(triggerUndo()));
    mainWindow->connect(redoAction, SIGNAL(triggered()), mainWindow, SLOT(triggerRedo()));
    mainWindow->connect(findAction, SIGNAL(triggered()), findReplaceDialog, SLOT(show()));
    mainWindow->connect(findNextAction, SIGNAL(triggered()), findReplaceDialog, SLOT(findNext()));

    /* Options menu */

    editorModeAction->setCheckable(true);
    editorModeAction->setChecked(true);
    editorModeAction->setShortcut(QKeySequence("Ctrl+E"));

    wordWrapAction->setCheckable(true);
    wordWrapAction->setChecked(false);

    lineNumbersAction->setCheckable(true);
    lineNumbersAction->setChecked(true);

    smartypantsAction->setCheckable(true);
    smartypantsAction->setChecked(true);

    //changeFontAction->setIcon(QIcon::fromTheme("fonts"));
    //changeFontAction->setIconVisibleInMenu(true);

    //optionsMenu->insertSeparator(changeFontAction);

    mainWindow->connect(wordWrapAction, SIGNAL(toggled(bool)), editor, SLOT(setWordWrapEnabled(bool)));
    mainWindow->connect(lineNumbersAction, SIGNAL(toggled(bool)), editor, SLOT(setLineNumbersEnabled(bool)));
    mainWindow->connect(editorModeAction, SIGNAL(toggled(bool)), mainWindow, SLOT(setEditorEnabled(bool)));
    //mainWindow->connect(changeFontAction, SIGNAL(triggered()), mainWindow, SLOT(changeViewFont()));

    /* Help menu */

    aboutAction->setIcon(QIcon::fromTheme("info"));
    aboutAction->setIconVisibleInMenu(true);
    aboutAction->setShortcut(QKeySequence("F1"));

    mainWindow->connect(aboutAction, SIGNAL(triggered()), aboutDialog, SLOT(show()));    
}

tdMainWindow::tdMainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new tdMainWindowUi(this)),
      highlighter(new tdHtmlHighlighter(ui->source->document())),
      tidy(new tdTidy(this)),
      renderer(new tdRenderer(ui->editor, ui->view->page()->mainFrame()->findFirstElement("body"))),
      focus(WidgetEditor)
{
    connect(ui->editor, SIGNAL(cursorPositionChanged()), this, SLOT(updateStatusBarMessage()));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(refreshTab(int)));
    connect(ui->smartypantsAction, SIGNAL(toggled(bool)), renderer, SLOT(setSmartypantsEnabled(bool)));
    connect(renderer, SIGNAL(smartypantsEnabledChanged()), this, SLOT(updateSource()));
    connect(ui->editor, SIGNAL(modificationChanged(bool)), this, SLOT(setModificationStatus(bool)));
    connect(ui->editor, SIGNAL(loadFileRequest(QString)), this, SLOT(loadFile(QString)));

    statusBar()->show();

    ui->selectAllAction->setEnabled(false);

    ui->editor->installEventFilter(this);
    ui->source->installEventFilter(this);
    ui->view->installEventFilter(this);

    setWindowTitle(tr("(Untitled)"));

    QAction *first = ui->toolBar->actions().first();
    ui->toolBar->insertAction(first, ui->newAction);
    ui->toolBar->insertAction(first, ui->openAction);
    ui->toolBar->insertAction(first, ui->saveAction);
    ui->toolBar->insertSeparator(first);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->findAction);
    //ui->toolBar->addAction(ui->changeFontAction);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->exportHtmlAction);
    ui->toolBar->addAction(ui->exportPdfAction);
    /*
    toolBar->addSeparator();
    QAction *refreshAction = toolBar->addAction(QIcon::fromTheme("reload"), tr("Reload"));
    connect(refreshAction, SIGNAL(triggered()), ui->view, SLOT(reload()));
    */

    ui->exportHtmlAction->setIcon(QIcon::fromTheme("gnome-mime-text-html"));
    ui->exportPdfAction->setIcon(QIcon::fromTheme("gnome-mime-application-pdf"));

    addToolBar(ui->toolBar);

    //

    QFile cssFile(":/styles.css");
    if (cssFile.open(QFile::ReadOnly)) {
        viewCss.append(cssFile.readAll());
        ui->view->page()->mainFrame()->documentElement().findFirst("body").addClass("markdown-body");
    }
    updateViewStyle();

    updateRecentFilesActions();
}

tdMainWindow::~tdMainWindow()
{    
    delete ui;
}

bool tdMainWindow::eventFilter(QObject *object, QEvent *event)
{
    if (ui->editor == object && QEvent::KeyPress == event->type()) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        Qt::KeyboardModifiers modifiers = keyEvent->modifiers();
        if (Qt::Key_Z == keyEvent->key() && modifiers & Qt::CTRL) {
            if (!(modifiers & Qt::SHIFT)) {
                // Ctrl+Z
                if (ui->editor->document()->isUndoAvailable())
                    renderer->undo();
            } else {
                // Ctrl+Shift+Z
                if (ui->editor->document()->isRedoAvailable())
                    renderer->redo();
            }
        } else if (Qt::Key_Tab == keyEvent->key()) {
            ui->editor->insertPlainText("    ");
            return true;
        } else if (Qt::Key_Return == keyEvent->key() && modifiers & Qt::SHIFT) {
            ui->editor->insertPlainText("\n");
            return true;
        }
    }

    if (QEvent::FocusIn == event->type()) {
        if (ui->editor->hasFocus()) {
            focus = WidgetEditor;
        } else if (ui->view->hasFocus()) {
            focus = WidgetView;
        } else if (ui->source->hasFocus()) {
            focus = WidgetSource;
        }
    } else if (QEvent::FocusOut == event->type()) {
        bool canSelect = false;
        bool canCopy   = false;
        bool canPaste  = false;
        bool canSearch = false;
        switch (ui->tabWidget->currentIndex())
        {
        case TabEditor:
            if (WidgetEditor == focus) {
                if (!ui->editor->document()->isEmpty())
                    canSelect = true;
                if (ui->editor->textCursor().hasSelection())
                    canCopy = true;
                if (ui->editor->canPaste())
                    canPaste = true;
            } else if (WidgetView == focus) {
                if (!ui->editor->document()->isEmpty())
                    canSelect = true;
                if (ui->view->page()->hasSelection())
                    // (!ui->view->page()->selectedText().isEmpty())
                    canCopy = true;
            }
            canSearch = true;
            break;
        case TabHtml:
            if (WidgetSource == focus) {
                if (!ui->source->toPlainText().simplified().isEmpty())
                    canSelect = true;
                if (ui->source->textCursor().hasSelection())
                    canCopy = true;
            }
        } // end switch
        ui->selectAllAction->setEnabled(canSelect);
        ui->copyAction->setEnabled(canCopy);
        ui->cutAction->setEnabled(WidgetEditor == focus && canCopy);
        ui->pasteAction->setEnabled(canPaste);
        //
        ui->findAction->setEnabled(canSearch);
        ui->findNextAction->setEnabled(canSearch);
    }
    return QMainWindow::eventFilter(object, event);
}

void tdMainWindow::closeEvent(QCloseEvent *event)
{
    if (!confirmSaveIfModified()) {
        event->ignore();
        return;
    }
    QMainWindow::closeEvent(event);
}

void tdMainWindow::showEditorContextMenu(const QPoint &pos)
{
    QMenu menu;
    menu.addAction(ui->undoAction);
    menu.addAction(ui->redoAction);
    menu.addSeparator();
    menu.addAction(ui->cutAction);
    menu.addAction(ui->copyAction);
    menu.addAction(ui->pasteAction);
    menu.addSeparator();
    menu.addAction(ui->selectAllAction);
    menu.exec(ui->editor->mapToGlobal(pos));
}

void tdMainWindow::showSourceContextMenu(const QPoint &pos)
{
    QMenu menu;
    menu.addAction(ui->selectAllAction);
    menu.exec(ui->source->mapToGlobal(pos));
}

void tdMainWindow::showViewContextMenu(const QPoint &pos)
{
    QMenu menu;
    menu.addAction(ui->selectAllAction);
    menu.addSeparator();
    QAction *viewSourceAction = menu.addAction(tr("View source"));
    QAction *action = menu.exec(ui->view->mapToGlobal(pos));
    if (viewSourceAction == action) {
        ui->tabWidget->setCurrentIndex(TabHtml);
    }
}

void tdMainWindow::updateStatusBarMessage()
{
    QTextCursor cursor = ui->editor->textCursor();
    QString str;
    QTextStream stream(&str);
    stream << "("       << cursor.columnNumber()
           << ", "      << cursor.blockNumber() + 1
           << ") pos: " << cursor.position();

    statusBar()->showMessage(str);
}

void tdMainWindow::refreshTab(int tab)
{
    switch (tab)
    {
    case TabHtml:
    {
        updateSource();
        connect(ui->undoAction, SIGNAL(triggered()), this, SLOT(updateSource()));
        connect(ui->redoAction, SIGNAL(triggered()), this, SLOT(updateSource()));
        setToolBarActionsEnabled(false);
        break;
    }
    default:
        disconnect(ui->undoAction, SIGNAL(triggered()), this, SLOT(updateSource()));
        disconnect(ui->redoAction, SIGNAL(triggered()), this, SLOT(updateSource()));
        setToolBarActionsEnabled(true);
        ui->toolBar->refreshButtonStatus();
    } // end switch
}

void tdMainWindow::selectAll()
{
    switch (ui->tabWidget->currentIndex())
    {
    case TabEditor:
        if (ui->editor->hasFocus()) {
            ui->editor->selectAll();
        } else if (ui->view->hasFocus()) {
            QWebPage *page = ui->view->page();
            page->triggerAction(QWebPage::SelectAll);
        }
        break;
    case TabHtml:
        if (ui->source->hasFocus()) {
            ui->source->selectAll();
        }
    } // end switch
}

void tdMainWindow::updateSource()
{
    QWebElement body = ui->view->page()->mainFrame()->findFirstElement("body");
    QString str = body.toInnerXml().remove(QRegExp("\\s?class=\"__[\\d]*__\""));
    ui->source->setPlainText(tidy->tidy(str));
}

void tdMainWindow::triggerUndo()
{
    if (ui->editor->document()->isUndoAvailable()) {
        renderer->undo();
        ui->editor->document()->undo();
    }
}

void tdMainWindow::triggerRedo()
{
    if (ui->editor->document()->isRedoAvailable()) {
        renderer->redo();
        ui->editor->document()->redo();
    }
}

void tdMainWindow::cut()
{
    if (TabEditor == ui->tabWidget->currentIndex() && ui->editor->hasFocus())
        ui->editor->cut();
}

void tdMainWindow::copy()
{
    switch (ui->tabWidget->currentIndex())
    {
    case TabEditor:
        if (ui->editor->hasFocus()) {
            ui->editor->copy();
        } else if (ui->view->hasFocus()) {
            ui->view->pageAction(QWebPage::Copy);
        }
        break;
    case TabHtml:
        ui->source->copy();
    } // end switch
}

void tdMainWindow::setModificationStatus(bool modified)
{
    QString title = windowTitle();
    bool hasStar = title.startsWith("*");
    if (!hasStar && modified)
        setWindowTitle("*" + title);
    else if (hasStar && !modified)
        setWindowTitle(title.remove(0, 1));
    ui->saveAction->setEnabled(modified);
}

void tdMainWindow::newFile()
{
    if (!confirmSaveIfModified())
        return;

    ui->editor->clear();
    renderer->refreshAll();

    setWindowTitle(tr("(Untitled)"));
    file.clear();
    updateSource();
}

void tdMainWindow::openFile()
{
    if (!confirmSaveIfModified())
        return;

    loadFile(QFileDialog::getOpenFileName(this, tr("Open"), filePath(),
                                          "Markdown files (*.md *.markdown);;"
                                          "Text files (*.txt);;"
                                          "Any files (*.*)"), false);
    updateSource();
}

void tdMainWindow::saveFile()
{
    if (file.isNull())
        return saveFileAs();
    writeToFile(file);
}

void tdMainWindow::saveFileAs()
{
    QString name = QFileDialog::getSaveFileName(this, tr("Save File"),
                        filePath() + "/" + (file.isEmpty() ? "Untitled.md" : file));
    if (!name.isEmpty())
        writeToFile(name);
}

//void tdMainWindow::print()
//{
//    // @todo
//    QPrinter printer;
//    QPrintDialog printDialog(&printer, this);
//    printDialog.setOption(QPrintDialog::PrintPageRange, false);
//    if (printDialog.exec() == QDialog::Accepted) {
//        qDebug() << "...printing??";
//    }
//}

void tdMainWindow::exportPdf()
{
    QString fname;
    if (file.isEmpty()) {
        fname = QDir::homePath() + "/Untitled.pdf";
    } else {
        QFileInfo info(file);
        fname = info.path() + "/" + info.baseName() + ".pdf";
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(QFileDialog::getSaveFileName(this, tr("Save PDF as"),
                                                           fname, "*.pdf"));
    ui->view->print(&printer);

    QApplication::restoreOverrideCursor();
}

void tdMainWindow::exportHtml()
{
    QString fname;
    if (file.isEmpty()) {
        fname = QDir::homePath() + "/index.html";
    } else {
        QFileInfo info(file);
        fname = info.path() + "/" + info.baseName() + ".html";
    }
    QFile file(QFileDialog::getSaveFileName(this, tr("Save HTML"),
                                            fname, "*.html"));
    if (!file.open(QFile::WriteOnly))
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    updateSource();
    QString html = "<!DOCTYPE html>\n<html>\n<head>\n<title>Untitled</title>\n</head>\n<body>\n"
            + ui->source->toPlainText() + "</body>\n</html>";
    QTextStream stream(&file);
    stream << html;
    file.close();

    QApplication::restoreOverrideCursor();
}

void tdMainWindow::loadFile(QString filename, bool confirm)
{
    if (confirm && !confirmSaveIfModified())
        return;

    QFile f(filename);
    if (!f.open(QIODevice::ReadWrite | QIODevice::Text))
        return;

    file = filename;
    QFileInfo info(f);
    ui->editor->clear();
    renderer->refreshAll();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    ui->editor->document()->setPlainText(f.readAll());
    QApplication::restoreOverrideCursor();
    f.close();
    setWindowTitle(info.fileName());
    ui->editor->document()->setModified(false);

//    QString fileId = info.filePath() + " " + info.fileName();

    updateRecentFilesList();
}

void tdMainWindow::setEditorEnabled(bool enabled)
{
    if (enabled)
        ui->editor->show();
    else
        ui->editor->hide();
    //
    ui->wordWrapAction->setEnabled(enabled);
    ui->lineNumbersAction->setEnabled(enabled);
}

void tdMainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        loadFile(action->data().toString());
}

void tdMainWindow::clearRecentFiles()
{
    QSettings settings;
    QStringList files;
    settings.setValue("recentFileList", files);
    updateRecentFilesActions();
}

//void tdMainWindow::changeViewFont()
//{
//    ui->view->setFont(QFontDialog::getFont(0, ui->view->font()));
//    updateViewStyle();
//}

void tdMainWindow::writeToFile(QString name)
{
    QFile file(name);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    tdMainWindow::file = name;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QTextStream stream(&file);
    stream << ui->editor->toPlainText();
    QFileInfo info(file);
    file.close();
    QApplication::restoreOverrideCursor();
    setWindowTitle(info.fileName());
    ui->editor->document()->setModified(false);

    updateRecentFilesList();
}

bool tdMainWindow::confirmSaveIfModified()
{
    if (!ui->editor->document()->isModified())
        return true;

    QString name;
    if (file.isEmpty()) {
        name = "Untitled";
    } else {
        QFileInfo info(file);
        name = info.baseName();
    }

    QMessageBox msgBox;
    msgBox.setText(tr("Save changes to '%1'?").arg(name));
    msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes);
    msgBox.setDefaultButton(QMessageBox::Yes);
    msgBox.setIcon(QMessageBox::Warning);

    switch(msgBox.exec())
    {
    case QMessageBox::Yes:
        saveFile();
    case QMessageBox::No:
        return true;
    } // end switch
    return false;
}

QString tdMainWindow::filePath() const
{
    if (file.isEmpty())
        return QDir::homePath();
    QFileInfo info(file);
    return info.path();
}

void tdMainWindow::updateViewStyle()
{
    QWebElement de = ui->view->page()->mainFrame()->documentElement();
    QWebElement styleElement = de.findFirst("style");
    if (styleElement.isNull()) {
        QWebElement head = de.findFirst("head");
        head.appendInside("<style type=\"text/css\"></style>");
        styleElement = de.findFirst("style");
    }
    //assert(!styleElement.isNull());
    QFont font("sans-serif");
    QString css = "body { margin:0; padding:0 9px; font-family:" + font.family() + "; }";
    css.append(viewCss);
    styleElement.setPlainText(css);
}

void tdMainWindow::setToolBarActionsEnabled(bool enabled)
{
    QList<QAction *> actions = ui->toolBar->actions();
    QList<QAction *>::const_iterator i = actions.constBegin();
    for (; i != actions.constEnd(); ++i) {
        QAction *a = *i;
        if (ui->toolBar == a->parent())
            a->setEnabled(enabled);
    }
}

void tdMainWindow::updateRecentFilesList()
{
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(file);
    files.prepend(file);
    while (files.size() > tdMainWindowUi::MaxRecentFiles)
        files.removeLast();
     settings.setValue("recentFileList", files);
     updateRecentFilesActions();
}

void tdMainWindow::updateRecentFilesActions()
{
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();
    int n = files.count();
    for (int i = 0; i < n; ++i) {
        ui->recentFileActs[i]->setText(QFileInfo(files[i]).fileName());
        ui->recentFileActs[i]->setData(files[i]);
        ui->recentFileActs[i]->setVisible(true);
    }
    for (int j = n; j < tdMainWindowUi::MaxRecentFiles; ++j)
        ui->recentFileActs[j]->setVisible(false);
    ui->openRecentMenu->setEnabled(n > 0);
}
