#include <Qt>
#include <QDebug>
#include <QString>
#include "tidy.h"

tdTidy::tdTidy(QObject *parent)
    : QObject(parent),
      m_tdoc(tidyCreate())
{
    tidyBufInit(&m_tidyout);
    tidyOptSetInt(m_tdoc, TidyIndentContent, 2);
    tidyOptSetInt(m_tdoc, TidyIndentSpaces, 3);
    tidyOptSetInt(m_tdoc, TidyBodyOnly, 1);
    tidyOptSetBool(m_tdoc, TidyPreserveEntities, yes);
    tidyOptSetBool(m_tdoc, TidyShowWarnings, no);
    tidyOptSetBool(m_tdoc, TidyForceOutput, yes);
    tidySetInCharEncoding(m_tdoc, "utf8");
}

tdTidy::~tdTidy()
{
    tidyBufFree(&m_tidyout);
    tidyRelease(m_tdoc);
}

QString tdTidy::tidy(QString input)
{
    input.replace(QChar(0x2013), "&ndash;")
         .replace(QChar(0x2014), "&mdash;")
         .replace(QChar(0x2026), "&hellip;")
         .replace(QChar(0x2122), "&trade;")
         .replace(QChar(0x201C), "&ldquo;")
         .replace(QChar(0x201D), "&rdquo;")
         .replace(QChar(0x2030), "&permil;");

    QByteArray ba = input.toUtf8();
    int rc = -1;
    tidyBufClear(&m_tidyout);
    rc = tidyParseString(m_tdoc, ba.constData());
    /* if (rc >= 0)
        rc = tidyCleanAndRepair(m_tdoc); */
    if (rc >= 0)
        rc = tidySaveBuffer(m_tdoc, &m_tidyout);
    if (rc >= 0) {
        if (rc > 0) return QString((const char *) m_tidyout.bp).replace("\n\n", "\n");
    } // else { /* error! */ }

    return QString();
}
