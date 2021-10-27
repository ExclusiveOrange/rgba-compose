#pragma once

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class RgbaComposer; }
QT_END_NAMESPACE

class RgbaComposer : public QMainWindow
{
  Q_OBJECT

public:
  RgbaComposer(QWidget *parent = nullptr);
  ~RgbaComposer();

private:
  Ui::RgbaComposer *ui;
  struct Private;
  Private *p;

  void setupUi();
};
