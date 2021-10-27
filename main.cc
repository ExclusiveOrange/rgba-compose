#include "RgbaComposer.hh"

#include <QApplication>

int main(int argc, char *argv[])
{
  QCoreApplication::setOrganizationName("ExclusiveOrange");
  QCoreApplication::setApplicationName("RGBA Composer");
  QApplication a(argc, argv);
  RgbaComposer w;
  w.show();
  return a.exec();
}
