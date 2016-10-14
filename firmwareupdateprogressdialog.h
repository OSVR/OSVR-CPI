#ifndef FIRMWAREUPDATEPROGRESSDIALOG_H
#define FIRMWAREUPDATEPROGRESSDIALOG_H

#include <QDialog>

namespace Ui {
class FirmwareUpdateProgressDialog;
}

class FirmwareUpdateProgressDialog : public QDialog {
  Q_OBJECT

public:
  explicit FirmwareUpdateProgressDialog(QWidget *parent = 0);
  ~FirmwareUpdateProgressDialog();

  void setText(QString text);
  QString getText();

private:
  Ui::FirmwareUpdateProgressDialog *ui;
  QString m_text = QString("");
};

#endif // FIRMWAREUPDATEPROGRESSDIALOG_H
