/*
 *   Copyright 2013, Jorge Martinez Vargas ( jormarv5@fiv.upv.es )
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 * 
 *      Author: Jorge Martinez Vargas ( jormarv5@fiv.upv.es )
 */

#include <QtCore/QMap>
#include <QtCore/QVariant>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QLabel>
#include <iostream>

#include "settingswindow.h"

SettingsWindow::SettingsWindow(QWidget *parent, QMap<QString, QVariant> *prevset) : QDialog(parent) {
    main_window = parent;
    if(prevset==0){ // No previous settings, default ones loaded (this shouldn't happen anyways)
        autosave = false;
        overwrite = true;
	dist_maxima_UP_toBaseline = 20;
	dist_maxima_DOWN_toBaseline = 10;
    }else{
        autosave = (*prevset)["autosave"].toBool();
        overwrite = (*prevset)["overwrite"].toBool();
	dist_maxima_UP_toBaseline = (*prevset)["up_edge"].toInt();
	dist_maxima_DOWN_toBaseline = (*prevset)["down_edge"].toInt();
    }

    /* GUI creation */
    setAttribute(Qt::WA_DeleteOnClose);

    settings_layout = new QVBoxLayout;
    overwrite_checkbox = new QCheckBox("Overwrite xml file when saving");
    autosave_checkbox = new QCheckBox("Autosave changes on image change/exit");       
    
    settings_layout->addWidget(autosave_checkbox);
    settings_layout->addWidget(overwrite_checkbox);
    
    QLabel *integerLabel_up = new QLabel(tr("Up baseline border "  "%1 and %2:").arg(5).arg(40));
    QSpinBox *integerSpinBox_up = new QSpinBox;
    integerSpinBox_up->setRange(5, 40);
    integerSpinBox_up->setSingleStep(1);
    integerSpinBox_up->setValue(dist_maxima_UP_toBaseline);
    settings_layout->addWidget(integerLabel_up);
    settings_layout->addWidget(integerSpinBox_up);

    QLabel *integerLabel_down = new QLabel(tr("Down baseline border "  "%1 and %2:").arg(5).arg(40));
    QSpinBox *integerSpinBox_down = new QSpinBox;
    integerSpinBox_down->setRange(5, 40);
    integerSpinBox_down->setSingleStep(1);
    integerSpinBox_down->setValue(dist_maxima_DOWN_toBaseline);
    settings_layout->addWidget(integerLabel_down);
    settings_layout->addWidget(integerSpinBox_down);

    
    commands_layout = new QHBoxLayout;
    apply_button = new QPushButton("Apply");
    accept_button = new QPushButton("Accept");
    cancel_button = new QPushButton("Cancel");
    commands_widget = new QWidget();
    
    commands_layout->addWidget(apply_button);
    commands_layout->addWidget(accept_button);
    commands_layout->addWidget(cancel_button);
    commands_widget->setLayout(commands_layout);
    settings_layout->addWidget(commands_widget);
    this->setLayout(settings_layout);

    if(autosave)
        autosave_checkbox->setChecked(true);
    else
        autosave_checkbox->setChecked(false);
    if(overwrite)
        overwrite_checkbox->setChecked(true);
    else
        overwrite_checkbox->setChecked(false);

    connect(autosave_checkbox,SIGNAL(toggled(bool)),this,SLOT(updateAutosave(bool)));
    connect(overwrite_checkbox,SIGNAL(toggled(bool)),this,SLOT(updateOverwrite(bool)));
    connect(integerSpinBox_up,SIGNAL(valueChanged(int)),this,SLOT(updateDist_maxima_UP_toBaseline(int)));
    connect(integerSpinBox_down,SIGNAL(valueChanged(int)),this,SLOT(updateDist_maxima_DOWN_toBaseline(int)));
    
    connect(apply_button,SIGNAL(clicked()),this,SLOT(applySlot()));
    connect(accept_button,SIGNAL(clicked()),this,SLOT(acceptSlot()));
    connect(cancel_button,SIGNAL(clicked()),this,SLOT(close()));
    
    show();
}

/**********************************************************************************
                                  SLOTS
***********************************************************************************/

void SettingsWindow::applySlot(){
    QMap<QString, QVariant> settings;
    settings["autosave"] = autosave;
    settings["overwrite"] = overwrite;
    settings["up_edge"] = dist_maxima_UP_toBaseline;
    settings["down_edge"] = dist_maxima_DOWN_toBaseline;
    updateSettings(settings);
}

void SettingsWindow::acceptSlot(){
  QMap<QString, QVariant> settings;
  settings["autosave"] = autosave;
  settings["overwrite"] = overwrite;
  settings["up_edge"] = dist_maxima_UP_toBaseline;
  settings["down_edge"] = dist_maxima_DOWN_toBaseline;
  updateSettings(settings);
  //applySlot();
  close();
}

void SettingsWindow::updateAutosave(bool s){
    autosave = s;
}
void SettingsWindow::updateDist_maxima_UP_toBaseline(int s){
  dist_maxima_UP_toBaseline = s;
}
void SettingsWindow::updateDist_maxima_DOWN_toBaseline(int s){
  dist_maxima_DOWN_toBaseline = s;
}

void SettingsWindow::updateOverwrite(bool s){
    overwrite = s;
}

void SettingsWindow::closeEvent(QCloseEvent *event){
    disconnect(this,SIGNAL(updateSettings(QMap<QString,QVariant>&)),main_window,SLOT(updateSettings(QMap<QString,QVariant>&)));   
}

