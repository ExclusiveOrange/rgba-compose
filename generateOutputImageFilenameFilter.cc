#include "generateOutputImageFilenameFilter.hh"

#include <QImageWriter>
#include <QStringList>

QString
generateOutputImageFilenameFilter()
{
  QStringList formats;
  for (const QByteArray &format : QImageWriter::supportedImageFormats())
    formats.append(format);

  QStringList filter;
  for (const QString &format : formats)
    filter.append(QString("%1 (*.%1)").arg(format));

  return filter.join(";;");
}
