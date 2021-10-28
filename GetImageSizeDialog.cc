#include "GetImageSizeDialog.hh"
#include "ui_GetImageSizeDialog.h"

GetImageSizeDialog::GetImageSizeDialog(QString title, QString bodyText, QSize imageSize, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::GetImageSizeDialog)
{
  ui->setupUi(this);
  setWindowTitle(title);
  ui->bodyText->setText(bodyText);
  ui->width->setValue(imageSize.width());
  ui->height->setValue(imageSize.height());
  adjustSize();
  setMinimumSize(size());
  setMaximumSize(size());
}

GetImageSizeDialog::~GetImageSizeDialog()
{
  delete ui;
}

std::optional<QSize>
GetImageSizeDialog::getImageSizeModal()
{
  if (Accepted == exec())
    return QSize{ui->width->value(), ui->height->value()};

  return std::nullopt;
}
