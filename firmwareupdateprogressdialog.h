// Copyright 2016 Razer, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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

  void setTitle(QString title);

  void setText(QString text);
  QString getText();

  void showOK();

private slots:
  void on_firmwareUpdateOKButton_clicked();

private:
  Ui::FirmwareUpdateProgressDialog *ui;
  QString m_text = QString("");
};

#endif // FIRMWAREUPDATEPROGRESSDIALOG_H
