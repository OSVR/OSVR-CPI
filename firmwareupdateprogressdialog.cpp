#include "firmwareupdateprogressdialog.h"
#include "ui_firmwareupdateprogressdialog.h"

FirmwareUpdateProgressDialog::FirmwareUpdateProgressDialog(QWidget *parent)
    : QDialog(parent, Qt::WindowStaysOnTopHint | Qt::WindowSystemMenuHint |
                          Qt::WindowTitleHint),
      ui(new Ui::FirmwareUpdateProgressDialog) {
  ui->setupUi(this);
}

FirmwareUpdateProgressDialog::~FirmwareUpdateProgressDialog() { delete ui; }

void FirmwareUpdateProgressDialog::setText(QString text) {
  m_text = text;
  ui->label->setText(text);
  QApplication::processEvents();
}

QString FirmwareUpdateProgressDialog::getText() { return m_text; }
