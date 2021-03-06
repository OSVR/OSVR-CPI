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

#include "firmwareupdateprogressdialog.h"
#include "ui_firmwareupdateprogressdialog.h"

FirmwareUpdateProgressDialog::FirmwareUpdateProgressDialog(QWidget *parent)
    : QDialog(parent, Qt::WindowStaysOnTopHint | Qt::WindowSystemMenuHint |
                          Qt::WindowTitleHint),
      ui(new Ui::FirmwareUpdateProgressDialog) {
  ui->setupUi(this);

  ui->firmwareUpdateOKButton->setVisible(false);
}

FirmwareUpdateProgressDialog::~FirmwareUpdateProgressDialog() { delete ui; }

void FirmwareUpdateProgressDialog::setTitle(QString title) {
  setWindowTitle(title);
}

void FirmwareUpdateProgressDialog::setText(QString text) {
  m_text = text;
  ui->label->setText(text);
  QApplication::processEvents();
}

QString FirmwareUpdateProgressDialog::getText() { return m_text; }

void FirmwareUpdateProgressDialog::showOK() {
    ui->firmwareUpdateOKButton->setVisible(true);
    activateWindow();
}

void FirmwareUpdateProgressDialog::on_firmwareUpdateOKButton_clicked() { close(); }
