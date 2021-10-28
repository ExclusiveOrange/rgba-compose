#include "generateInputImageFilenameFilter.hh"

#include <QImageReader>
#include <QStringList>

QString
generateInputImageFilenameFilter()
{
  QStringList formats;
  for (const QByteArray &format : QImageReader::supportedImageFormats())
    formats.append(format);

  QStringList formatFilters;
  for (const QString &format : formats)
    formatFilters.append("*." + format);

  QString allFilter = "Images (" + formatFilters.join(" ") + ")";

  QStringList filter(allFilter);
  for (const QString &format : formats)
    filter.append(QString("%1 (*.%1)").arg(format));

  return filter.join(";;");
}
