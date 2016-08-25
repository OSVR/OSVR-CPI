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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void saveConfigFile(QString filename);
    bool loadConfigFile(QString filename);
    void updateFormValues(void);
    void loadValuesFromForm(OSVRUser *oo);

    void on_resetButton_clicked();
    void on_saveButton_clicked();
    void on_exitButton_clicked();
    void on_aboutButton_clicked();
    void on_resetYawButton_clicked();

    void on_disableButton_clicked();
    void on_enableButton_clicked();

    void on_updateFWButton_clicked();
    void on_checkFWButton_clicked();

    QString findSerialPort(int,int);
    QSerialPort *openSerialPort(QString);
    void writeSerialData(QSerialPort *thePort, const QByteArray &);

    void sendCommandNoResult(QByteArray);
    QString sendCommandWaitForResults(QByteArray);

    void atmel_erase();
    void atmel_load(QString);
    void atmel_launch();


    void on_enableDisplayButton_clicked();

    void on_persistanceSlider_valueChanged(int value);
    void on_rotationVectorSlider_sliderReleased();
    
    void on_disableButton_2_clicked();

private:
    Ui::MainWindow *ui;

    bool m_verbose=false;
    QString m_osvrUserConfigFilename;
    OSVRUser m_osvrUser;
};

#endif // MAINWINDOW_H
