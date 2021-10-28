#pragma once

#include <QDialog>

#include <optional>

namespace Ui {
  class GetImageSizeDialog;
}

class GetImageSizeDialog : public QDialog
{
  Q_OBJECT

public:
   GetImageSizeDialog(QString title, QString bodyText, QSize imageSize, QWidget *parent = nullptr);
  ~GetImageSizeDialog();

   // call this instead of .exec()
   std::optional<QSize>
   getImageSizeModal();

private:
  Ui::GetImageSizeDialog *ui;
};

