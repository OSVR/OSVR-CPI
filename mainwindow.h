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
#include <QProcess>

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
  /* Bottom buttons */
  void on_helpButton_clicked();
  void on_aboutButton_clicked();
  void on_exitButton_clicked();

  /* User settings */
  void on_resetButton_clicked();
  void on_saveButton_clicked();

  /* Firmware */
  void on_checkFWButton_clicked();
  void on_updateFWButton_clicked();

  /* Display */
  void on_recenterButton_clicked();
  void on_GPUType_currentIndexChanged(const QString &arg1);
  void on_directModeButton_clicked();
  void on_extendedModeButton_clicked();

  /* HDK 1.x */
  void on_enableDisplayButton_clicked();
  void on_toggleSBSButton_clicked();
  void on_screenPersistenceFull_clicked();
  void on_screenPersistenceMedium_clicked();
  void on_screenPersistenceLow_clicked();

private:
  Ui::MainWindow *ui;

  static const QString RELATIVE_BIN_DIR;
  static const bool DEBUG_VERBOSE;

  QString m_osvrUserConfigFilename, m_GPUType;
  OSVRUser m_osvrUser;

  /* Serial */
  QString findSerialPort(int, int);
  QSerialPort *openSerialPort(QString);
  void writeSerialData(QSerialPort *thePort, const QByteArray &);

  void sendCommandNoResult(QByteArray);
  QString sendCommandWaitForResults(QByteArray);
  void portKnock(QString portName);

  QString getFirmwareVersionsString();
  void showFirmwareVersionError();

  /* Supplementary executables */
  enum PathMode { E_PM_ABSOLUTE, E_PM_RELATIVE };
  enum LaunchMode { E_LM_SYNCHRONOUS, E_LM_ASYNCHRONOUS, E_LM_KNOCK };
  enum LaunchResult { E_LR_MISSING,
                      E_LR_UNABLE_TO_START,
                      E_LR_UNABLE_TO_WAIT,
                      E_LR_SUCCESS };
  LaunchResult launchProcess(QString path,
                    PathMode path_mode = E_PM_RELATIVE,
                    QStringList args = QStringList(),
                    LaunchMode launch_mode = E_LM_ASYNCHRONOUS);

  /* ATMEL */
  void atmel_erase();
  void atmel_load(QString);
  void atmel_launch();

  /* GPU Detection */
  void detectGPUType();
  enum GPUType { E_GPUTYPE_ERROR = -1, E_GPUTYPE_NVIDIA = 1, E_GPUTYPE_AMD = 2, E_GPUTYPE_UNKNOWN = -2 };

  /* User settings */
  bool loadConfigFile(QString filename);
  void updateFormValues(void);
  void loadValuesFromForm(OSVRUser *user);
  void saveConfigFile(QString filename);
};

#endif // MAINWINDOW_H
