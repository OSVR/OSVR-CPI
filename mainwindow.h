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

  enum FirmwareTarget { E_FW_TARGET_UNKNOWN = 0, E_FW_TARGET_HDK_1X = 1, E_FW_TARGET_HDK_2 = 2 };
  FirmwareTarget getFirmwareTarget();

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

  void on_cameraFWUpdateButton_clicked();

private:
  Ui::MainWindow *ui;

  static const QString RELATIVE_BIN_DIR,
                       DFU_PROGRAMMER_RELATIVE_DIR,
                       DFU_PROGRAMMER_NAME,
                       GPU_TYPE_DETECTOR_NAME,
                       ATMEL_DETECTOR_NAME,
                       POST_FW_UPDATE_STR,
                       FW_UPDATE_FAIL_STR;

  static const bool DEBUG_VERBOSE = false;
  static const int DFU_PROGRAMMER_MISSING = -646675,
                   DFU_PROGRAMMER_TIMEOUT_MS = 15000,
                   PORT_KNOCK_SLEEP_MS = 200,
                   SERIAL_PORT_RETRIES = 3,
                   SERIAL_PORT_RETRY_MS = 100,
                   SERIAL_READ_MS = 1000,
                   SERIAL_PORT_VID = 0x1532,
                   SERIAL_PORT_PID = 0x0B00;

  QString m_osvrUserConfigFilename, m_GPUType;
  OSVRUser m_osvrUser;

  /* Serial */
  QString findSerialPort(int, int);
  QSerialPort *openSerialPort(QString);
  void writeSerialData(QSerialPort *thePort, const QByteArray &);

  void sendCommandNoResult(QByteArray command);
  QString sendCommandWaitForResults(QByteArray command, bool silent = false);
  void portKnock(QString portName);

  /* Firmware */
  QString getFirmwareVersionsString();
  void showFirmwareVersionError();
  bool checkFirmwareTarget(FirmwareTarget current_fw, QString hexFile);

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
                    LaunchMode launch_mode = E_LM_ASYNCHRONOUS,
                    int* exit_code = NULL, // exit_code valid iff launch_mode==E_LM_SYNCHRONOUS
                    int timeout_ms = -1); // -1 = no timeout

  /* ATMEL */
  int atmel_erase();
  int atmel_load(QString);
  int atmel_launch();

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
