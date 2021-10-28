#include "getInputImageFilenameFilter.hh"

#include <QImageReader>
#include <QStringList>

const QString &
getInputImageFilenameFilter()
{
  static const QString filter = []{
    QStringList formats;
    for (const QByteArray &format : QImageReader::supportedImageFormats())
      formats.append(format);

    QStringList formatFilters;
    for (const QString &format : formats)
      formatFilters.append("*." + format);

    QString allFilter = "Images (" + formatFilters.join(" ") + ")";

    QStringList filters(allFilter);
    for (const QString &format : formats)
      filters.append(QString("%1 (*.%1)").arg(format));

    return filters.join(";;");
  }();

  return filter;
}
