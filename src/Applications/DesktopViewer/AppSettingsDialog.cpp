/* --*-c++-*-- */
/**
 * Godzi
 * Copyright 2010 Pelican Mapping
 * http://pelicanmapping.com
 * http://github.com/gwaldron/godzi
 *
 * Godzi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include <QFileDialog>
#include <QMessageBox>
#include "GodziApp"
#include "AppSettingsDialog"

AppSettingsDialog::AppSettingsDialog(const GodziApp* app)
: _app(app), _cacheEnabled(true)
{
  initUi();
}

void AppSettingsDialog::initUi()
{
	_ui.setupUi(this);

  if (_app.valid())
  {
    _ui.skyModelCheckBox->setChecked(_app->getSkyEnabled());
    _ui.sunComboBox->setCurrentIndex(_app->getSunMode() < _ui.sunComboBox->count() ? _app->getSunMode() : 0);

    double sunLat, sunLon;
    _app->getSunPosition(sunLat, sunLon);
    _ui.sunLatBox->setText(QString::number(sunLat));
    _ui.sunLonBox->setText(QString::number(sunLon));

    _cacheEnabled = _app->getCacheEnabled();
    _ui.cacheEnabledCheckBox->setChecked(_cacheEnabled);

    std::string cachePath = _app->getCachePath();
	  if (!cachePath.empty())
	  {
		  std::string tempStr(cachePath);
      _ui.cachePathLabel->setText( QString(tempStr.c_str()) );
	  }

    if (_app->getCacheMaxSize().isSet())
    {
      unsigned int max = _app->getCacheMaxSize().get();
      _ui.cacheMaxSpinBox->setValue(max > _ui.cacheMaxSpinBox->maximum() ? _ui.cacheMaxSpinBox->maximum() : (max < _ui.cacheMaxSpinBox->minimum() ? _ui.cacheMaxSpinBox->minimum() : max));
    }
  }
  else
  {
    _ui.okButton->setEnabled(false);
  }

  _ui.messageLabel->setStyleSheet("color: red");

  QObject::connect(_ui.okButton, SIGNAL(clicked()), this, SLOT(validateAndAccept()));
  QObject::connect(_ui.skyModelCheckBox, SIGNAL(clicked()), this, SLOT(updateSkyStates()));
  QObject::connect(_ui.sunComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSkyStates()));
	QObject::connect(_ui.cacheBrowseButton, SIGNAL(clicked()), this, SLOT(showBrowse()));
	QObject::connect(_ui.cacheEnabledCheckBox, SIGNAL(toggled(bool)), this, SLOT(updateCacheStates()));
  QObject::connect(_ui.clearCacheButton, SIGNAL(clicked()), this, SLOT(clearCache()));
  QObject::connect(_ui.sunLatBox, SIGNAL(textChanged(const QString&)), this, SLOT(onNumericTextChanged()));
  QObject::connect(_ui.sunLonBox, SIGNAL(textChanged(const QString&)), this, SLOT(onNumericTextChanged()));

  updateSkyStates();
	updateCacheStates();
}

void AppSettingsDialog::validateAndAccept()
{
  std::string errMsg = "";
  if (validateNumericInput(errMsg) && validateCacheSettings(errMsg))
    accept();
  else
    _ui.messageLabel->setText(QString(errMsg.c_str()));
}

void AppSettingsDialog::updateSkyStates()
{
  bool skyEnabled = _ui.skyModelCheckBox->isChecked();
  _ui.sunLabel->setEnabled(skyEnabled);
  _ui.sunComboBox->setEnabled(skyEnabled);

  //TODO: Fix following line after other options have been implemented
  bool sunFixed = true; // _ui.sunComboBox->currentIndex() == GodziApp::FixedPosition;
  _ui.sunLatLabel->setEnabled(skyEnabled && sunFixed);
  _ui.sunLatBox->setEnabled(skyEnabled && sunFixed);
  _ui.sunLonLabel->setEnabled(skyEnabled && sunFixed);
  _ui.sunLonBox->setEnabled(skyEnabled && sunFixed);
}

void AppSettingsDialog::onNumericTextChanged()
{
  std::string errMsg;
  validateNumericInput(errMsg);
}

bool AppSettingsDialog::validateNumericInput(std::string& errMsg)
{
  //Sun position latitude
  bool sunLatOk = true;
  if (_ui.sunLatBox->isEnabled())
  {
    _ui.sunLatBox->text().toDouble(&sunLatOk);
    if (sunLatOk)
    {
      _ui.sunLatBox->setStyleSheet("");
    }
    else
    {
      _ui.sunLatBox->setStyleSheet("color: red");
      errMsg = errMsg + "Sun latitude is invalid. ";
    }
  }

  //Sun position longitude
  bool sunLonOk = true;
  if (_ui.sunLonBox->isEnabled())
  {
    _ui.sunLonBox->text().toDouble(&sunLonOk);
    if (sunLonOk)
    {
      _ui.sunLonBox->setStyleSheet("");
    }
    else
    {
      _ui.sunLonBox->setStyleSheet("color: red");
      errMsg = errMsg + "Sun longitude is invalid. ";
    }
  }

  return sunLatOk && sunLonOk;
}

bool AppSettingsDialog::validateCacheSettings(std::string& errMsg)
{
  if (_cacheEnabled && !_ui.cacheEnabledCheckBox->isChecked())
    QMessageBox::information(this, "Notice", "Disabling the cache will not take effect until Godzi has been restarted.", QMessageBox::Ok, QMessageBox::Ok);

  return true;
}

void AppSettingsDialog::updateCacheStates()
{
	bool cacheEnabled = _ui.cacheEnabledCheckBox->isChecked();
	_ui.cachePathLabel->setEnabled(cacheEnabled);
  _ui.cachePathLabelLabel->setEnabled(cacheEnabled);
	_ui.cacheBrowseButton->setEnabled(cacheEnabled);
	_ui.cacheMaxLabel->setEnabled(cacheEnabled);
	_ui.cacheMaxSpinBox->setEnabled(cacheEnabled);
}

void AppSettingsDialog::showBrowse()
{
  QString cachePath = QFileDialog::getSaveFileName(this, tr("Save File"),
    _ui.cachePathLabel->text().length() > 0 ? _ui.cachePathLabel->text() : QDir::homePath() + QDir::separator() + "Godzi",
    tr("All Files (*.*)"));

	if (!cachePath.isNull())
		_ui.cachePathLabel->setText(cachePath);
}

void AppSettingsDialog::clearCache()
{
  QMessageBox msg;
  msg.setText("Are you sure you want to clear the cache?");
  msg.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
  msg.setDefaultButton(QMessageBox::Cancel);
  msg.setIcon(QMessageBox::Question);
  msg.setWindowTitle("Clearing the cache");

  if (msg.exec() == QMessageBox::Yes)
  {
    if (_app.valid())
      _app->clearCache();
  }
}

void AppSettingsDialog::getSunPosition(double& out_lat, double& out_lon)
{
  out_lat = _ui.sunLatBox->text().toDouble();
  out_lon = _ui.sunLonBox->text().toDouble();
}