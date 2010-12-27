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

#include <osgDB/FileNameUtils>
#include <osgEarthUtil/WMS>
#include "Common"
#include "WMSEditDialog"

WMSEditDialog::WMSEditDialog(const Godzi::WMS::WMSDataSource* source) : _activeUrl("")
{
	initUi();
	updateUi(source);
}

Godzi::WMS::WMSDataSource* WMSEditDialog::getSource()
{
	return (Godzi::WMS::WMSDataSource*)_source->clone();
}

void WMSEditDialog::initUi()
{
	_ui.setupUi(this);
	_ui.messageLabel->setStyleSheet("QLabel { color : red; }");

	QObject::connect(_ui.queryButton, SIGNAL(clicked()), this, SLOT(doQuery()));
	QObject::connect(this, SIGNAL(accepted()), this, SLOT(onDialogClose()));
	QObject::connect(_ui.locationLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(toggleQueryEnabled()));
	QObject::connect(_ui.formatCheckBox, SIGNAL(toggled(bool)), this, SLOT(toggleOptions()));
	QObject::connect(_ui.selectAllCheckbox, SIGNAL(clicked(bool)), this, SLOT(selectAllClicked(bool)));
	QObject::connect(_ui.layersListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onLayerItemClicked(QListWidgetItem*)));
}

void WMSEditDialog::updateUi(bool canSelectLayers)
{
	bool active = _source.valid() && !_source->getLocation().empty() && !_source->error();

	_ui.locationLineEdit->setText(_source.valid() ? QString(_source->getLocation().c_str()) : "");

	_ui.nameLabel->setEnabled(active);

	_ui.nameLineEdit->setEnabled(active);
	_ui.nameLineEdit->setText(active && _source->name().isSet() ? QString(_source->name()->c_str()) : "");

	_ui.formatCheckBox->setEnabled(active);

	_ui.formatComboBox->clear();

	_ui.layersListWidget->setEnabled(active && canSelectLayers);
	_ui.layersListWidget->clear();

	_ui.okButton->setEnabled(active);

	_ui.selectAllCheckbox->setChecked(true);
	_ui.selectAllCheckbox->setEnabled(active && canSelectLayers);

	if (active)
	{
		std::vector<std::string> formats = _source->getAvailableFormats();
		for (std::vector<std::string>::iterator it = formats.begin(); it != formats.end(); ++it)
			_ui.formatComboBox->addItem(QString( (*it).c_str() ) );

		osgEarth::Drivers::WMSOptions opt = (osgEarth::Drivers::WMSOptions)_source->getOptions();

		if (opt.format().isSet())
		{
			_ui.formatCheckBox->setChecked(true);
			int fIndex = _ui.formatComboBox->findText(QString(opt.format()->c_str()), Qt::MatchExactly);
			if (fIndex >= 0)
			{
				_ui.formatComboBox->setCurrentIndex(fIndex);
			}
			else
			{
				//TODO?
			}
		}
		else
		{
			_ui.formatCheckBox->setChecked(false);
		}

		Godzi::DataObjectSpecVector layerSpecs;
		if (_source->getDataObjectSpecs(layerSpecs) )
		{
			for(Godzi::DataObjectSpecVector::const_iterator i = layerSpecs.begin(); i != layerSpecs.end(); ++i)
			{
				QListWidgetItem* item = new QListWidgetItem(QString(i->getText().c_str()));

				GodziDesktop::DataSourceObjectPair data;
				data._source = _source.get();
				data._spec = *i;

				item->setData(Qt::UserRole, QVariant::fromValue(data));

				if (canSelectLayers)
					item->setCheckState(Qt::Checked);

				_ui.layersListWidget->addItem(item);
			}
		}
	}

	_ui.formatComboBox->setEnabled(active && _ui.formatCheckBox->isChecked());

	if (_source.valid())
		_ui.messageLabel->setText(QString(_source->error() ? _source->errorMsg().c_str() : ""));
}

void WMSEditDialog::onDialogClose()
{
	_source = new Godzi::WMS::WMSDataSource(createSourceOptions());
	_source->setFullUrl(_activeUrl);
	_ui.nameLineEdit->text().isEmpty() ? _source->name().unset() : _source->name() = _ui.nameLineEdit->text().toUtf8().data();
}

osgEarth::Drivers::WMSOptions WMSEditDialog::createSourceOptions()
{
	osgEarth::Drivers::WMSOptions opt;

	std::string url = _ui.locationLineEdit->text().toUtf8().data();
	opt.url() = url.substr(0, url.find("?"));

	if (_ui.formatCheckBox->isChecked())
		opt.format() = _ui.formatComboBox->currentText().toUtf8().data();

	//if (_ui->srsCheckBox->isChecked())
	//	opt.srs() = _ui->srsLineEdit->text().toStdString();

	std::vector<int> selectedLayerIds;
	for (int i = 0; i < _ui.layersListWidget->count(); i++)
	{
		QListWidgetItem* item = _ui.layersListWidget->item(i);

		if (item->checkState() == Qt::Checked)
		{
			QVariant v = item->data(Qt::UserRole);
			if ( !v.isNull() )
			{
				GodziDesktop::DataSourceObjectPair data = v.value<GodziDesktop::DataSourceObjectPair>();
				selectedLayerIds.push_back(data._spec.getObjectUID());
			}
		}
	}

	std::vector<std::string> selectedLayers;
	for (std::vector<int>::iterator it = selectedLayerIds.begin(); it != selectedLayerIds.end(); ++it)
		selectedLayers.push_back(_source->getLayerName(*it));

	opt.layers() = Godzi::vectorToCSV(selectedLayers);

	return opt;
}

void WMSEditDialog::doQuery()
{
	_activeUrl = _ui.locationLineEdit->text().toUtf8().data();
	_source = new Godzi::WMS::WMSDataSource(_activeUrl);

	std::string lower = osgDB::convertToLowerCase(_activeUrl);
	updateUi(lower.find("layers=", 0) == std::string::npos);
}

void WMSEditDialog::toggleQueryEnabled()
{
	_ui.queryButton->setEnabled(_ui.locationLineEdit->text().length() > 0);
}

void WMSEditDialog::toggleOptions()
{
	_ui.formatComboBox->setEnabled(_ui.formatCheckBox->isChecked());
}

void WMSEditDialog::selectAllClicked(bool checked)
{
	for (int i = 0; i < _ui.layersListWidget->count(); i++)
		_ui.layersListWidget->item(i)->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
}

void WMSEditDialog::onLayerItemClicked(QListWidgetItem* item)
{
	if (_ui.selectAllCheckbox->isEnabled() && item->checkState() != Qt::Checked)
		_ui.selectAllCheckbox->setChecked(false);
}