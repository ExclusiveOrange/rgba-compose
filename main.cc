#include "RgbaComposer.hh"

#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  RgbaComposer w;
  w.show();
  return a.exec();
}
