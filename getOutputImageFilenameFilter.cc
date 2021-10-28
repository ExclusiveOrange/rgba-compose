#include "getOutputImageFilenameFilter.hh"

#include <QImageWriter>
#include <QStringList>

const QString &
getOutputImageFilenameFilter()
{
  static const QString filter = []{
    QStringList formats;
    for (const QByteArray &format : QImageWriter::supportedImageFormats())
      formats.append(format);

    QStringList filters;
    for (const QString &format : formats)
      filters.append(QString("%1 (*.%1)").arg(format));

    return filters.join(";;");
  }();

  return filter;
}
