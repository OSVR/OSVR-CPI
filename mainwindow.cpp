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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "firmwareupdateprogressdialog.h"
#include "json/json.h"

#include <cstdio>
#include <string>
#include <fstream>
#include <iostream>

#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QtSerialPort>

// Constants ------------------------------------------------------------------

const QString MainWindow::RELATIVE_BIN_DIR = "/../OSVR-Core/bin/";
const QString MainWindow::DFU_PROGRAMMER_RELATIVE_DIR = "/dfu-prog-usb-1.2.2/";
const QString MainWindow::DFU_PROGRAMMER_NAME = "dfu-programmer.exe";
const QString MainWindow::GPU_TYPE_DETECTOR_NAME = "GPUTypeDetector.exe";
const QString MainWindow::ATMEL_DETECTOR_NAME = "AtmelDetector.exe";
const QString MainWindow::POST_FW_UPDATE_STR = "The new firmware version will now be checked. Please wait a moment...";
const QString MainWindow::FW_UPDATE_FAIL_STR = "Your HDK is currently in bootloader mode and is prepared to receive updated firmware. "
                                               "It will not function until it is reset or new firmware is loaded. "
                                               "It can generally be reset by unplugging and replugging it from the Belt Box module."
                                               "<br><br>"
                                               "Please see online documentation for further information.";

// Helper class for validating numerical input --------------------------------

class myValidator : public QDoubleValidator {
public:
  myValidator(double bottom, double top, int decimals, QObject *parent)
      : QDoubleValidator(bottom, top, decimals, parent) {}

  QValidator::State validate(QString &s, int &i) const {
    if (s.isEmpty() || s == "-") {
      return QValidator::Intermediate;
    }

    QLocale locale;

    QChar decimalPoint = locale.decimalPoint();
    int charsAfterPoint = s.length() - s.indexOf(decimalPoint) - 1;

    if (charsAfterPoint > decimals() && s.indexOf(decimalPoint) != -1) {
      return QValidator::Invalid;
    }

    bool ok;
    double d = locale.toDouble(s, &ok);

    if (ok && d >= bottom() && d <= top()) {
      return QValidator::Acceptable;
    } else {
      return QValidator::Invalid;
    }
  }
};

// Lifecycle ------------------------------------------------------------------

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  ui->standingHeight->setValidator(new myValidator(0, 300, 2, this));
  ui->seatedHeight->setValidator(new myValidator(0, 300, 2, this));
  ui->ipd->setValidator(new myValidator(0, 100, 2, 0));

  ui->dOsSpherical->setValidator(new myValidator(-100, 100, 2, this));
  ui->dOsCylindrical->setValidator(new myValidator(-100, 100, 2, this));
  ui->dOsAxis->setValidator(new myValidator(-100, 300, 2, this));
  ui->nOsAdd->setValidator(new myValidator(-100, 100, 2, this));

  ui->dOdSpherical->setValidator(new myValidator(-100, 100, 2, this));
  ui->dOdCylindrical->setValidator(new myValidator(-100, 100, 2, this));
  ui->dOdAxis->setValidator(new myValidator(-100, 300, 2, this));
  ui->nOdAdd->setValidator(new myValidator(-100, 100, 2, this));

  ui->tabWidget->setCurrentIndex(0);

  m_GPUType = ui->GPUType->currentText();

  // Hide user settings and display settings tab
  ui->tabWidget->removeTab(2);
  ui->tabWidget->removeTab(2);

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  QString programPath = "";
  programPath = env.value("PROGRAMDATA", "C:/ProgramData");
  m_osvrUserConfigFilename =
      QString(programPath + "/OSVR/osvr_user_settings.json");
  loadConfigFile(m_osvrUserConfigFilename);

  // detectGPUType();
}

MainWindow::~MainWindow() { delete ui; }

// Bottom buttons -------------------------------------------------------------

void MainWindow::on_helpButton_clicked() {
  QDesktopServices::openUrl(QUrl("http://www.osvr.org/hdk2help"));
}

void MainWindow::on_aboutButton_clicked() {
  QMessageBox::information(
      this, QString("HDK Firmware Utility"),
      QString("This application is a lightweight utility to help you configure "
              "your HDK's firmware. For more information, visit "
              "<a href=http://www.osvr.org>osvr.org</a>."),
      QMessageBox::Ok);
  // loadConfigFile(QString("start.json"));
}

void MainWindow::on_exitButton_clicked() { QApplication::quit(); }

// User settings slots --------------------------------------------------------

void MainWindow::on_resetButton_clicked() { updateFormValues(); }

void MainWindow::on_saveButton_clicked() {
  loadValuesFromForm(&m_osvrUser);
  saveConfigFile(m_osvrUserConfigFilename);
}

// Firmware -------------------------------------------------------------------

void MainWindow::on_checkFWButton_clicked() {
  QString versions = getFirmwareVersionsString();
  if (versions == QString::null) {
    showFirmwareVersionError();
  } else {
    QMessageBox::information(this, "Firmware Versions", versions,
                             QMessageBox::Ok);
  }
}

void MainWindow::showFirmwareVersionError()
{
    QMessageBox::critical(
        this, QString("Error"),
        QString("Unable to read firmware version. Please ensure that your "
          "HDK is properly connected and that no other programs are connected "
          "to its serial port, then try again. If the problem persists, refer "
          "to the online documentation for further information."),
        QMessageBox::Ok);
}

/**
 * Reading the state of the lock pin from the FPGA fails on HDK 1.x units (which have an FPGA) and HDK 2 units that don't.
 * However, it fails in a different way on each, allowing us to tell the difference betwen HDK 1 firmware and HDK 2 firmware.
 * Note that this is a property of the firmware itself and not the underlying hardware, so if you manage to flash an HDK 1 with
 * firmware compiled to target an HDK 2, this will report E_FW_TARGET_HDK_2.
 */
MainWindow::FirmwareTarget MainWindow::getFirmwareTarget() {
    QString lockpin = sendCommandWaitForResults("#FL\n", true);

    if (lockpin == QString::null) {
        return E_FW_TARGET_UNKNOWN;
    }

    if (lockpin.contains("Bad command")) {
        return E_FW_TARGET_HDK_1X;
    }

    return E_FW_TARGET_HDK_2;
}

bool MainWindow::checkFirmwareTarget(FirmwareTarget current_fw, QString hexFile) {
    switch(current_fw) {
    case E_FW_TARGET_HDK_1X:
        if (hexFile.contains("HDK2_ONLY")) {
            return false;
        }
        break;

    case E_FW_TARGET_HDK_2:
        if (hexFile.contains("HDK1_ONLY")) {
            return false;
        }
        break;

    default:
        break;
    }

    return true;
}

// FW Update button
void MainWindow::on_updateFWButton_clicked() {
  QMessageBox::information(this,
              "HDK Driver Update",
              "Before you can update your HDK's firmware, you must install additional drivers. "
              "<br><br>"
              "To install these drivers, click <a href=\"file:///" +
              QCoreApplication::applicationDirPath() + DFU_PROGRAMMER_RELATIVE_DIR +
              "\">here</a> and run <b>HDK_Bootloader_Drivers.exe</b>."
              "<br><br>"
              "To proceed, click OK. You will be prompted to select the new firmware.");

  // Ask user for the HEX file to update with
  QString hexFile;

  hexFile = QFileDialog::getOpenFileName(this,
                                         "Select Firmware Update File",
                                         QCoreApplication::applicationDirPath(),
                                         tr("Firmware Hex Files (*.hex)"));

  // User pressed cancel
  if (hexFile == "") {
    return;
  }

  // find the OSVR HDK and get current FW version
  QString firmware_versions = getFirmwareVersionsString();
  QMessageBox::StandardButton reply;

  if (firmware_versions != QString::null) {
    FirmwareTarget current_fw = getFirmwareTarget();
    if (checkFirmwareTarget(current_fw, hexFile)) {
        reply = QMessageBox::question(
            this, tr("Ready To Update Firmware Versions"),
            "<b>Current Firmware Versions:</b><br>" + firmware_versions +
                "<br><br><b>Selected Firmware Update File:</b><br>" +
                hexFile +
                "<br><br>Do you wish to proceed with the firmware update?",
            QMessageBox::Yes | QMessageBox::No);
    } else {
        QString current_hw_guess = (current_fw == E_FW_TARGET_HDK_1X ? "1.x" : "2"),
                update_hw_guess = (current_fw == E_FW_TARGET_HDK_1X ? "2" : "1.x");

        reply = QMessageBox::critical(
            this, tr("Selected Firmware Does Not Match!"),
            "The firmware you have selected <b>does not match</b> the firmware currently running on your HDK. "
            "Updating your HDK to use the selected firmware would most likely render your HDK <b>inoperable</b>. "
            "The update will not proceed."
            "<br><br>"
            "Your HDK's current firmware is built for HDK " +
                    current_hw_guess +
            " hardware, and the selected firmware is intended for HDK " +
                    update_hw_guess +
            " hardware.<br><br>"
            "<b>Current Firmware Versions:</b><br>" + firmware_versions +
            "<br><br><b>Firmware Hex File Selected For Update:</b><br>" +
            hexFile,
            QMessageBox::Ok);
        return;
    }
  } else {
      reply = QMessageBox::question(
          this, tr("Ready To Update Firmware Versions"),
          "<b>Warning:</b> Your current firmware version cannot be detected. "
              "This will not prevent you from updating your firmware. See "
              "online documentation for details."
              "<br><br>Please ensure that your HDK is connected according to "
              "the manual before proceeding."
              "<br><br><b>Firmware Hex File Selected For Update:</b><br>" +
              hexFile +
              "<br><br>Do you wish to proceed with the firmware update?",
          QMessageBox::Yes | QMessageBox::No);
  }

  if (reply == QMessageBox::No) {
    return;
  }

  int result;

  // Set bootloader mode
  FirmwareUpdateProgressDialog dialog;
  dialog.show();
  dialog.setText("Setting bootloader mode...");
  sendCommandNoResult("#?b1948\n");

  // Wait for Atmel device to appear
  launchProcess(ATMEL_DETECTOR_NAME, E_PM_RELATIVE, QStringList(), E_LM_SYNCHRONOUS, &result);
  switch (result) {
    case 0:
      dialog.setText(dialog.getText() + "<b>done</b>.<br>");
      break;

    case 1:
      dialog.close();
      QMessageBox::critical(this, QString("Unable To Update Firmware"),
                            QString("Unable to detect an HDK in bootloader mode. "
                                    "Please ensure that you have installed <b>HDK_Bootloader_Drivers.exe</b> from "
                                    "<a href=\"file:///" +
                                    QCoreApplication::applicationDirPath() + DFU_PROGRAMMER_RELATIVE_DIR +
                                    "\">here</a> and that your HDK is connected according to the manual, then try again."));
      return;
  }

  // Erase firmware
  dialog.setText(dialog.getText() + "Erasing existing firmware...");
  result = atmel_erase();
  if (result == DFU_PROGRAMMER_MISSING) {
      dialog.close();
      QMessageBox::critical(this, QString("Unable To Update Firmware"),
                            QString("The dfu-programmer utility cannot be found. Please reinstall."
                                    "<br><br>") + FW_UPDATE_FAIL_STR);
      return;
  } else if (result) {
      dialog.setText(dialog.getText() + "<b>failed!</b><br><br>" + FW_UPDATE_FAIL_STR);
      dialog.showOK();
      dialog.exec();
      return;
  }
  dialog.setText(dialog.getText() + "<b>done</b>.<br>Loading new firmware...");

  // Load new firmware
  if (atmel_load(hexFile)) {
      dialog.setText(dialog.getText() + "<b>failed!</b><br><br>" + FW_UPDATE_FAIL_STR);
      dialog.showOK();
      dialog.exec();
      return;
  }
  dialog.setText(dialog.getText() +
                 "<b>done</b>.<br>Launching new firmware...");

  // Launch new firmware
  if (atmel_launch()) {
      dialog.setText(dialog.getText() + "<b>failed!</b><br><br>" + FW_UPDATE_FAIL_STR);
      dialog.showOK();
      dialog.exec();
      return;
  }

  // Success!
  dialog.setText(dialog.getText() +
                 "<b>done</b>.<br><br>Firmware update complete."
                 "<br><br>" +
                 POST_FW_UPDATE_STR);
  dialog.setTitle(QString("Firmware Update Complete"));

  QThread::sleep(3);

  QString text = dialog.getText();
  text.chop(POST_FW_UPDATE_STR.length());

  // Verify FW version
  firmware_versions = getFirmwareVersionsString();
  if (firmware_versions == QString::null) {
    dialog.setText(text + "<b>Unable to detect new firmware version. Please refer to online documentation for further information.</b>");
  } else {
    dialog.setText(text + "<b>New firmware versions:</b><br>" + firmware_versions);
  }

  dialog.showOK();
  dialog.exec();
}

void MainWindow::on_cameraFWUpdateButton_clicked()
{
    launchProcess("OSVR IR Camera Firmware update(5SF006N2_v0007).exe");
}

// Display --------------------------------------------------------------------

void MainWindow::on_recenterButton_clicked() {
    launchProcess("osvr_reset_yaw_RZ.exe");
}

void MainWindow::on_GPUType_currentIndexChanged(const QString &gpu_type) {
  m_GPUType = gpu_type;
}

void MainWindow::on_directModeButton_clicked() {
  QString exe;

  if (m_GPUType.contains("NVIDIA")) {
    exe = "EnableOSVRDirectMode.exe";
  } else if (m_GPUType.contains("AMD")) {
    exe = "EnableOSVRDirectModeAMD.exe";
  } else {
    QMessageBox::critical(
        this, "Unable To Set Direct Mode",
        "You must select a graphics card type of NVIDIA or AMD.",
        QMessageBox::Ok);
    return;
  }

  LaunchResult launch_result = launchProcess(RELATIVE_BIN_DIR + "/" + exe);
  switch (launch_result) {
  case E_LR_MISSING:
      QMessageBox::critical(
          this, "Unable To Set Direct Mode",
          "Unable to locate the executable which is used to toggle direct " \
                  "mode (<b>" + exe + "</b>). Please reinstall.",
          QMessageBox::Ok);
    break;

  case E_LR_UNABLE_TO_START:
  case E_LR_UNABLE_TO_WAIT:
    QMessageBox::critical(this, "Unable To Set Direct Mode",
                          "Unable to set direct mode. Try running <b>" + exe +
                              "</b> manually.",
                          QMessageBox::Ok);
    break;

  default:
      break;
  }
}

void MainWindow::on_extendedModeButton_clicked() {
    QString exe;

    if (m_GPUType.contains("NVIDIA")) {
      exe = "DisableOSVRDirectMode.exe";
    } else if (m_GPUType.contains("AMD")) {
      exe = "DisableOSVRDirectModeAMD.exe";
    } else {
      QMessageBox::critical(
          this, "Unable To Set Extended Mode",
          "You must select a graphics card type of NVIDIA or AMD.",
          QMessageBox::Ok);
      return;
    }

    LaunchResult launch_result = launchProcess(RELATIVE_BIN_DIR + "/" + exe);
    switch (launch_result) {
    case E_LR_MISSING:
        QMessageBox::critical(
            this, "Unable To Set Extended Mode",
            "Unable to locate the executable which is used to toggle extended " \
                    "mode (<b>" + exe + "</b>). Please reinstall.",
            QMessageBox::Ok);
      break;

    case E_LR_UNABLE_TO_START:
    case E_LR_UNABLE_TO_WAIT:
      QMessageBox::critical(this, "Unable To Set Extended Mode",
                            "Unable to set extended mode. Try running <b>" + exe +
                                "</b> manually.",
                            QMessageBox::Ok);
      break;

    default:
        break;
    }
}

// HDK 1.x --------------------------------------------------------------------

void MainWindow::on_enableDisplayButton_clicked() {
  sendCommandNoResult("#Hi\n");
}

void MainWindow::on_toggleSBSButton_clicked() {
  // Toggle side by side mode for 1.x HMDs
  sendCommandNoResult("#f1s\n");
}

void MainWindow::on_screenPersistenceFull_clicked() {
  sendCommandNoResult("#sp03C50\n");
}

void MainWindow::on_screenPersistenceMedium_clicked() {
  sendCommandNoResult("#sp03C32\n");
}

void MainWindow::on_screenPersistenceLow_clicked() {
  sendCommandNoResult("#sp03C0A\n");
}

// Serial ---------------------------------------------------------------------

QString MainWindow::findSerialPort(int VID, int PID) {
  QString outputString = "";

  QString description;
  QString manufacturer;
  QString serialNumber;
  QString portName;
  bool deviceFound = false;

  portName = "Not found";
  foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
    if (info.vendorIdentifier() == VID && info.productIdentifier() == PID) {
      description = info.description();
      manufacturer = info.manufacturer();
      serialNumber = info.serialNumber();
      outputString += "\n";
      outputString +=
          tr("Port: ") + info.portName() + "\n" +
          tr("Location: ") + info.systemLocation() + "\n" +
          tr("Description: ") + info.description() + "\n" +
          tr("Manufacturer: ") + info.manufacturer() + "\n" +
          tr("Serial number: ") + info.serialNumber() + "\n" +
          tr("Vendor Identifier: ") +
          (info.hasVendorIdentifier()
               ? QString::number(info.vendorIdentifier(), 16)
               : QString()) +
          "\n" + tr("Product Identifier: ") +
          (info.hasProductIdentifier()
               ? QString::number(info.productIdentifier(), 16)
               : QString()) +
          "\n" + tr("Busy: ") +
          (info.isBusy() ? tr("Yes") : tr("No")) + "\n";
      deviceFound = true;
      portName = info.portName();
    }
  }

  if (DEBUG_VERBOSE) {
    if (deviceFound) {
      QMessageBox::information(this, "Device Located",
                               "COM port: " + outputString);
    } else {
      QMessageBox::warning(this, "Unable To Locate Device",
                           "Unable to find device. Ensure it is connected as "
                           "shown in the manual.");
    }
  }

  return portName;
}

QSerialPort *MainWindow::openSerialPort(QString portName) {
  QSerialPort *thePort = new QSerialPort(this);
  thePort->setPortName(portName);

  bool init_error = false;
  if (!thePort->setBaudRate(QSerialPort::Baud57600)) {
    init_error = true;
  }
  if (thePort->error() != QSerialPort::NoError) {
    init_error = true;
  }

  if (!init_error && thePort->open(QIODevice::ReadWrite)) {
    if (thePort->baudRate() != QSerialPort::Baud57600) {
      if (DEBUG_VERBOSE) {
        QMessageBox::warning(this, tr("Warning While Opening Port"),
                                      "When opening the serial port " + portName +
                                      ", the requested Baud setting of 57600 was not set.\n");
      }

      return NULL;
    }

    return thePort;
  } else {
    if (DEBUG_VERBOSE) {
      QMessageBox::critical(this, tr("Unable To Open Port"),
                                     "Unable to open serial port " + portName +
                                     ".\nPlease ensure the device is connected as "
                                     "shown in the manual and try again.\n");
    }
    return NULL;
  }
}

void MainWindow::writeSerialData(QSerialPort *thePort, const QByteArray &data) {
  if (thePort->write(data) == -1) {
    if (DEBUG_VERBOSE) {
      QMessageBox::critical(this, tr("Error Writing To Serial Port"),
                            "Unable to write to serial port.\nPlease ensure "
                            "the device is connected as shown in the manual "
                            "and try again.\n");
    }
  }
  thePort->flush();
  thePort->waitForBytesWritten(5000);
  QThread::sleep(1);
}

QString MainWindow::sendCommandWaitForResults(QByteArray theCommand, bool silent) {
  QByteArray theResult = "";
  QSerialPort *thePort;
  QString portName;

  // find the OSVR HDK and get current FW version
  portName = findSerialPort(SERIAL_PORT_VID, SERIAL_PORT_PID);
  if (portName == "Not found") {
      if (!silent) {
          QMessageBox::critical(this,
                                QString("Unable To Send Command"),
                                QString("Unable to locate serial port. Please disconnect and reconnect your HDK."));
      }

      return QString::null;
  }

  portKnock(portName);

  for (int i = 0; i < SERIAL_PORT_RETRIES; i++) {
      thePort = openSerialPort(portName);

      if (thePort) {
        writeSerialData(thePort, theCommand);

        if (thePort->waitForReadyRead(SERIAL_READ_MS)) {
          theResult = thePort->readAll();
          thePort->close();
          return theResult;
        }
        else {
            thePort->close();
            continue;
        }
      }

      QThread::msleep(SERIAL_PORT_RETRY_MS);
  }

  if (DEBUG_VERBOSE) {
    QMessageBox::critical(this,
                        QString("Unable To Send Command"),
                        QString("Unable to send command '<b>'" + theCommand + "</b>. Please try again."));
  }

  return QString::null;
}

void MainWindow::sendCommandNoResult(QByteArray theCommand) {
  QSerialPort *thePort;
  QString portName;

  // find the OSVR HDK and get current FW version
  portName = findSerialPort(SERIAL_PORT_VID, SERIAL_PORT_PID);
  if (portName == "Not found") {
      QMessageBox::critical(this,
                            QString("Unable To Send Command"),
                            QString("Unable to locate serial port. Please disconnect and reconnect your HDK."));
      return;
  }

  for (int i = 0; i < SERIAL_PORT_RETRIES; i++) {
      thePort = openSerialPort(portName);

      if (thePort) {
        writeSerialData(thePort, theCommand);
        thePort->close();
        return;
      }

      QThread::msleep(SERIAL_PORT_RETRY_MS);
  }

  QMessageBox::critical(this,
                        QString("Unable To Send Command"),
                        QString("Unable to send command '<b>'" + theCommand + "</b>. Please try again."));
}

void MainWindow::portKnock(QString portName) {
    QStringList args;
    args << "-serial" << portName;
    args << portName;
    launchProcess("putty.exe", E_PM_RELATIVE, args, E_LM_KNOCK);
    QThread::msleep(PORT_KNOCK_SLEEP_MS);   // sleep for long enough to ensure that serial port has been released by PuTTY
}

QString MainWindow::getFirmwareVersionsString() {
  QString response = sendCommandWaitForResults("#?v\n");
  QString result = "<u>HMD Main Board:</u> ";

  if (response == QString::null) {
      return QString::null;
  }

  response = response.replace("\r", "");
  QStringList split = response.split("\n", QString::SplitBehavior::SkipEmptyParts);

  /*
   * split should look like one of these (or a newer version):
   *
   * 0: '#?v'
   * 1: 'Version 1.98  Nov  8 2016'
   * 2: 'Tracker:1.10.1.472'
   *
   * Note format change with 1.99 and beyond:
   * 0: '#?v'
   * 1: 'Version 1.99 (RELEASE) Nov 28 2016'
   * 2: 'Tracker:1.10.1.472'
   */
  if (split.length() != 3) {
      return QString::null;
  }

  QStringList fw_version_split = split.at(1).split(" ", QString::SplitBehavior::SkipEmptyParts);
  /*
   * fw_version_split should look like one of these (or a newer version):
   *
   * 0: 'Version'
   * 1: '1.98'
   * 2: 'Nov'
   * 3: '8'
   * 4: '2016'
   *
   * Note format change with 1.99 and beyond:
   * 0: 'Version'
   * 1: '1.99'
   * 2: '(RELEASE)'
   * 3: 'Nov'
   * 4: '28'
   * 5: '2016'
   */

  switch (fw_version_split.length())
  {
  case 5:
      result += fw_version_split.at(1) + " (" +
              fw_version_split.at(2) + " " +
              fw_version_split.at(3) + ", " +
              fw_version_split.at(4) + ")<br>";
      break;

  case 6:
      result += fw_version_split.at(1) + " " +
              fw_version_split.at(2) + " (" +
              fw_version_split.at(3) + " " +
              fw_version_split.at(4) + ", " +
              fw_version_split.at(5) + ")<br>";
      break;

  default:
      return QString::null;
  }

  QStringList tracker_version_split = split.at(2).split(":");
  /*
   * tracker_version_split should look like this (or a newer version):
   * 0='Tracker'
   * 1='1.10.1.472'
   */
  if (tracker_version_split.length() != 2) {
      return QString::null;
  }

  result += "<u>IMU Sensor Hub:</u> " + tracker_version_split.at(1);

  return result;
}

// Supplementary executables --------------------------------------------------

MainWindow::LaunchResult MainWindow::launchProcess(QString path,
                                                   PathMode path_mode, // = E_RELATIVE_PATH
                                                   QStringList args, // = QStringList()
                                                   LaunchMode launch_mode, // E_LM_ASYNCHRONOUS
                                                   int* exit_code, // Valid iff launch_mode==E_LM_SYNCHRONOUS,
                                                   int timeout_ms) { // default: no timeout
  if (path_mode == E_PM_RELATIVE) {
    path = QCoreApplication::applicationDirPath() + "/" + path;
  }

  QFileInfo exe(path);
  if (!exe.exists()) {
    return E_LR_MISSING;
  }

  QProcess* process;

  switch (launch_mode) {
  case E_LM_SYNCHRONOUS:
      process = new QProcess(this);
      connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));
      process->start(path, args);

      if (!process->waitForStarted()) {
          return E_LR_UNABLE_TO_START;
      }

      if (!process->waitForFinished(timeout_ms)) {
          return E_LR_UNABLE_TO_WAIT;
      }

      if (exit_code) {
        *exit_code = process->exitCode();
      }

      return E_LR_SUCCESS;

  case E_LM_KNOCK:
      process = new QProcess(this);
      connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));
      process->start(path, args);

      if (!process->waitForStarted()) {
          return E_LR_UNABLE_TO_START;
      }

      // Sleep long enough to ensure PuTTY is able to open the serial port
      QThread::msleep(200);

      process->kill();
      return E_LR_SUCCESS;

  case E_LM_ASYNCHRONOUS:
      return QProcess::startDetached(path, args) ? E_LR_SUCCESS : E_LR_UNABLE_TO_START;

  default:
      // Compiler appeasement
      return E_LR_SUCCESS;
  }
}

// ATMEL ----------------------------------------------------------------------

int MainWindow::atmel_erase() {
  QStringList args;
  args << "atxmega256a3bu"
       << "erase";
  int return_code;
  if (launchProcess(DFU_PROGRAMMER_NAME, E_PM_RELATIVE, args, E_LM_SYNCHRONOUS, &return_code, DFU_PROGRAMMER_TIMEOUT_MS) == E_LR_MISSING) {
      return DFU_PROGRAMMER_MISSING;
  }
  return return_code;
}

int MainWindow::atmel_load(QString fwFile) {
  QStringList args;
  args << "atxmega256a3bu"
       << "flash" << fwFile;
  int return_code;
  launchProcess(DFU_PROGRAMMER_NAME, E_PM_RELATIVE, args, E_LM_SYNCHRONOUS, &return_code, DFU_PROGRAMMER_TIMEOUT_MS);
  return return_code;
}

int MainWindow::atmel_launch() {
  QStringList args;
  args << "atxmega256a3bu"
       << "launch";
  int return_code;
  launchProcess(DFU_PROGRAMMER_NAME, E_PM_RELATIVE, args, E_LM_SYNCHRONOUS, &return_code, DFU_PROGRAMMER_TIMEOUT_MS);
  QThread::sleep(3);
  return return_code;
}

// GPU Detection --------------------------------------------------------------

void MainWindow::detectGPUType() {
    QProcess *process = new QProcess(this);
    connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));
    process->start(GPU_TYPE_DETECTOR_NAME);

    if (!process->waitForStarted()) {
        return;
    }
    if (!process->waitForFinished()) {
        return;
    }

    switch((GPUType)process->exitCode()) {
    case E_GPUTYPE_NVIDIA:
        ui->GPUType->setCurrentIndex(ui->GPUType->findText("NVIDIA", Qt::MatchContains));
        return;

    case E_GPUTYPE_AMD:
        ui->GPUType->setCurrentIndex(ui->GPUType->findText("AMD", Qt::MatchContains));
        return;

    case E_GPUTYPE_UNKNOWN:
    case E_GPUTYPE_ERROR:
        break;
    }
}

// User settings --------------------------------------------------------------

bool MainWindow::loadConfigFile(QString filename) {
  char *cstr;
  std::string fname = filename.toStdString();
  cstr = new char[fname.size() + 1];
  strcpy(cstr, fname.c_str());

  std::ifstream file_id;
  file_id.open(cstr);

  Json::Reader reader;
  Json::Value value;
  if (!reader.parse(file_id, value)) {
    // qWarning("Couldn't open save file; creating file.");
    // new file just has default values
    saveConfigFile(filename);
  } else {
    m_osvrUser.read(value);
  }
  updateFormValues();
  return true;
}

void MainWindow::updateFormValues() {
  if ("Male" == m_osvrUser.gender()) {
    ui->gender->setCurrentIndex(0);
  } else {
    ui->gender->setCurrentIndex(1);
  }
  ui->standingHeight->setText(QString::number(m_osvrUser.standingEyeHeight()));
  ui->seatedHeight->setText(QString::number(m_osvrUser.seatedEyeHeight()));
  ui->ipd->setText(QString::number(m_osvrUser.pupilDistance(OS) +
                                   m_osvrUser.pupilDistance(OD)));

  ui->dOsSpherical->setText(QString::number(m_osvrUser.spherical(OS)));
  ui->dOsCylindrical->setText(QString::number(m_osvrUser.cylindrical(OS)));
  ui->dOsAxis->setText(QString::number(m_osvrUser.axis(OS)));
  ui->nOsAdd->setText(QString::number(m_osvrUser.addNear(OS)));
  ui->OSdominant->setChecked(m_osvrUser.dominant(OS));

  ui->dOdSpherical->setText(QString::number(m_osvrUser.spherical(OD)));
  ui->dOdCylindrical->setText(QString::number(m_osvrUser.cylindrical(OD)));
  ui->dOdAxis->setText(QString::number(m_osvrUser.axis(OD)));
  ui->nOdAdd->setText(QString::number(m_osvrUser.addNear(OD)));
  ui->ODdominant->setChecked(m_osvrUser.dominant(OD));
}

void MainWindow::loadValuesFromForm(OSVRUser *user) {
  double ipd;

  user->setGender(ui->gender->currentText().toStdString());

  // IPD settings
  if (!ui->ipd->text().isEmpty()) {
    ipd = ui->ipd->text().toDouble();
    user->setPupilDistance(OS, ipd / 2);
    user->setPupilDistance(OD, ipd / 2);
  }

  // Left eye settings
  if (!ui->dOsSpherical->text().isEmpty()) {
    user->setSpherical(OS, ui->dOsSpherical->text().toDouble());
  }
  if (!ui->dOsCylindrical->text().isEmpty()) {
    user->setCylindrical(OS, ui->dOsCylindrical->text().toDouble());
  }
  if (!ui->dOsAxis->text().isEmpty()) {
    user->setAxis(OS, ui->dOsAxis->text().toDouble());
  }
  if (!ui->nOsAdd->text().isEmpty()) {
    user->setAddNear(OS, ui->nOsAdd->text().toDouble());
  }

  // right eye settings
  if (!ui->dOdSpherical->text().isEmpty()) {
    user->setSpherical(OD, ui->dOdSpherical->text().toDouble());
  }
  if (!ui->dOdCylindrical->text().isEmpty()) {
    user->setCylindrical(OD, ui->dOdCylindrical->text().toDouble());
  }
  if (!ui->dOdAxis->text().isEmpty()) {
    user->setAxis(OD, ui->dOdAxis->text().toDouble());
  }
  if (!ui->nOdAdd->text().isEmpty()) {
    user->setAddNear(OD, ui->nOdAdd->text().toDouble());
  }

  if (ui->ODdominant->isChecked()) {
    user->setDominant(OD);
  } else {
    user->setDominant(OS);
  }

  // anthropometric settings
  if (!ui->standingHeight->text().isEmpty()) {
    user->setStandingEyeHeight(ui->standingHeight->text().toDouble());
  }
  if (!ui->seatedHeight->text().isEmpty()) {
    user->setSeatedEyeHeight(ui->seatedHeight->text().toDouble());
  }
}

void MainWindow::saveConfigFile(QString filename) {
  char *cstr;
  std::string fname = filename.toStdString();
  cstr = new char[fname.size() + 1];
  strcpy(cstr, fname.c_str());

  Json::Value json;
  m_osvrUser.write(json);

  std::ofstream out_file;
  out_file.open(cstr);
  Json::StyledWriter styledWriter;
  out_file << styledWriter.write(json);
  out_file.close();
}

