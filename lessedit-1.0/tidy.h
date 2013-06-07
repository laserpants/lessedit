#ifndef TIDY_H
#define TIDY_H

#include <QObject>
#include <tidy/tidy.h>
#include <tidy/buffio.h>

class tdTidy : public QObject
{
    Q_OBJECT

public:
    explicit tdTidy(QObject *parent = 0);
    ~tdTidy();

    QString tidy(QString input);

private:
    TidyDoc      const m_tdoc;
    TidyBuffer         m_tidyout;
};

#endif // TIDY_H
