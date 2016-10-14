/*
 * Copyright 2016 OSVR and contributors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>

#include "osvruser.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private slots:
  void on_helpButton_clicked();
  void on_aboutButton_clicked();
  void on_exitButton_clicked();

  // User setting
  // functions-------------------------------------------------------------
  bool loadConfigFile(QString filename);
  void updateFormValues(void);
  void loadValuesFromForm(OSVRUser *oo);
  void saveConfigFile(QString filename);

  void on_resetButton_clicked();
  void on_saveButton_clicked();

  // HMD Tab-------------------------------------------------------------
  void on_checkFWButton_clicked();
  void on_updateFWButton_clicked();

  // SW Tab-------------------------------------------------------------
  // Recenter HMD
  // Direct mode toggles
  void on_recenterButton_clicked();
  void on_GPUType_currentIndexChanged(const QString &arg1);
  void on_directModeButton_clicked();
  void on_extendedModeButton_clicked();

  // HDK 1.x Tab-------------------------------------------------------------
  void on_enableDisplayButton_clicked();
  void on_toggleSBSButton_clicked();
  void on_screenPersistenceFull_clicked();
  void on_screenPersistenceMedium_clicked();
  void on_screenPersistenceLow_clicked();

private:
  Ui::MainWindow *ui;

  bool m_verbose = false;

  QString m_osvrUserConfigFilename;
  OSVRUser m_osvrUser;

  QString m_GPU_type =
      "NVIDIA"; // this should match the default value of the GPUType QComboBox

  QString m_relativeBinDir = "/../OSVR-Core/bin/";

  // Helper
  // functions-------------------------------------------------------------
  QString findSerialPort(int, int);
  QSerialPort *openSerialPort(QString);
  void writeSerialData(QSerialPort *thePort, const QByteArray &);

  void sendCommandNoResult(QByteArray);
  QString sendCommandWaitForResults(QByteArray);

  QString getFirmwareVersionsString();

  int launchAsyncProcess(QString path, QStringList args = QStringList(),
                         bool absolute_path = false);

  void atmel_erase();
  void atmel_load(QString);
  void atmel_launch();
};

#endif // MAINWINDOW_H
