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
#include "WMSEditDialog"

std::string extractBetween(const std::string& str, const std::string &lhs, const std::string &rhs)
{
    std::string result;
		std::string::size_type start = str.find(lhs);
		if (start != std::string::npos)
    {
        start += lhs.length();
        std::string::size_type count = str.size() - start;
        std::string::size_type end = str.find(rhs, start); 
        if (end != std::string::npos) count = end-start;
        result = str.substr(start, count);
    }
    return result;
}

WMSEditDialog::WMSEditDialog(const Godzi::WMSSource* source)
{
	_source = source ? (Godzi::WMSSource*)source->clone() : new Godzi::WMSSource(osgEarth::Drivers::WMSOptions());
	_active = _source->getLocation().length() > 0;

	initUi();
	updateUi();
}

Godzi::WMSSource* WMSEditDialog::getSource()
{
	Godzi::WMSSource* outSource = (Godzi::WMSSource*)_source->clone();
	outSource->setAvailableLayers(_source->getActiveLayers());

	return outSource;
}

void WMSEditDialog::initUi()
{
	_ui.setupUi(this);
	_ui.messageLabel->setStyleSheet("QLabel { color : red; }");

	QObject::connect(_ui.queryButton, SIGNAL(clicked()), this, SLOT(doQuery()));
	QObject::connect(this, SIGNAL(accepted()), this, SLOT(onDialogClose()));
	QObject::connect(_ui.locationLineEdit, SIGNAL(textEdited(const QString&)), this, SLOT(toggleQueryEnabled()));
	QObject::connect(_ui.formatCheckBox, SIGNAL(toggled(bool)), this, SLOT(toggleOptions()));
}

void WMSEditDialog::updateUi()
{
	_ui.locationLineEdit->setText(QString::fromStdString(_source->getLocation()));

	_ui.formatCheckBox->setEnabled(_active);
	_ui.formatComboBox->setEnabled(_active && _ui.formatCheckBox->isChecked());

	_ui.formatComboBox->clear();
	for (std::vector<std::string>::iterator it = _availableFormats.begin(); it != _availableFormats.end(); ++it)
		_ui.formatComboBox->addItem(QString::fromStdString(*it));

	osgEarth::Drivers::WMSOptions opt = (osgEarth::Drivers::WMSOptions)_source->getOptions();

	if (opt.format().isSet())
	{
		_ui.formatCheckBox->setChecked(true);
		int fIndex = _ui.formatComboBox->findText(QString::fromStdString(opt.format().get()), Qt::MatchExactly);
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

	_ui.layersListWidget->setEnabled(_active);
	_ui.layersListWidget->clear();

	std::vector<std::string> layers = _source->getAvailableLayers();
	std::vector<std::string> active = _source->getActiveLayers();
	for (int i=0; i < layers.size(); i++)
	{
		QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(_source->layerDisplayName(layers[i])));
		item->setData(Qt::UserRole, QString::fromStdString(layers[i]));
		item->setCheckState(std::find(active.begin(), active.end(), layers[i]) == active.end() ? Qt::Unchecked : Qt::Checked);
		_ui.layersListWidget->addItem(item);
	}

	if (_active)
	{
		_ui.messageLabel->setText("");
	}
	else
	{
		_ui.messageLabel->setText(QString::fromStdString(_source->errorMsg()));
	}

	_ui.okButton->setEnabled(_active);
}

void WMSEditDialog::onDialogClose()
{
	updateSourceOptions(false);
}

void WMSEditDialog::updateSourceOptions(bool urlChanged)
{
	osgEarth::Drivers::WMSOptions opt;
	std::vector<std::string> activeLayers;
	std::string urlStr = _source->fullUrl().isSet() ? _source->fullUrl().get() : _source->getLocation();

	opt.url() = urlStr.substr(0, urlStr.find("?"));

	if (urlStr.find("?") != std::string::npos)
		parseWMSOptions(urlStr, opt);

	if (!urlChanged)
	{
		if (_ui.formatCheckBox->isChecked())
			opt.format() = _ui.formatComboBox->currentText().toStdString();

		//if (_ui->srsCheckBox->isChecked())
		//	opt.srs() = _ui->srsLineEdit->text().toStdString();

		for (int i = 0; i < _ui.layersListWidget->count(); i++)
		{
			QListWidgetItem* item = _ui.layersListWidget->item(i);

			if (item->checkState() == Qt::Checked)
				activeLayers.push_back(item->data(Qt::UserRole).toString().toStdString());
		}
	}

	_source->setOptions(opt);
	_source->setActiveLayers(activeLayers);
}

void WMSEditDialog::parseWMSOptions(const std::string& url, osgEarth::Drivers::WMSOptions& opt)
{
	std::string lower = osgDB::convertToLowerCase( url );

	if (lower.find("layers=", 0) != std::string::npos)
		opt.layers() = extractBetween(lower, "layers=", "&");

	if (lower.find("styles=", 0) != std::string::npos)
		opt.style() = extractBetween(lower, "styles=", "&");

	if (lower.find("srs=", 0) != std::string::npos)
		opt.srs() = extractBetween(lower, "srs=", "&");

	if (lower.find("format=image/", 0) != std::string::npos)
		opt.format() = extractBetween(lower, "format=image/", "&");
}

void WMSEditDialog::doQuery()
{
	std::string oldUrl = _source->fullUrl().isSet() ? _source->fullUrl().get() : "";
	_source->fullUrl() = _ui.locationLineEdit->text().toStdString();

	updateSourceOptions(oldUrl != _source->fullUrl().get());

	std::string url = _ui.locationLineEdit->text().toStdString();

	if (url.length() == 0)
		return;

	char sep = url.find_first_of('?') == std::string::npos? '?' : '&';
	std::string capUrl = url + sep + "SERVICE=WMS" + "&REQUEST=GetCapabilities";

	//Try to read the WMS capabilities
	osg::ref_ptr<osgEarthUtil::WMSCapabilities> capabilities = osgEarthUtil::WMSCapabilitiesReader::read(capUrl, 0L);
	if (capabilities.valid())
	{
		_active = true;

		//NOTE: Currently this flattens any layer heirarchy into a single list of layers
		std::vector<std::string> layerList;
		std::map<std::string, std::string> displayNames;
		getLayerNames(capabilities->getLayers(), layerList, displayNames);
		
		_source->setAvailableLayers(layerList);
		_source->setLayerDisplayNames(displayNames);

		_availableFormats.clear();
		osgEarthUtil::WMSCapabilities::FormatList formats = capabilities->getFormats();
		for (osgEarthUtil::WMSCapabilities::FormatList::const_iterator it = formats.begin(); it != formats.end(); ++it)
		{
			std::string format = *it;

			int pos = format.find("image/");
			if (pos != std::string::npos && int(pos) == 0)
				format.erase(0, 6);

			_availableFormats.push_back(format);
		}
	}
	else
	{
		_active = false;
		_source->setError(true);
		_source->setErrorMsg("Could not get WMS capabilities.");
	}

	updateUi();
}

void WMSEditDialog::toggleQueryEnabled()
{
	_ui.queryButton->setEnabled(_ui.locationLineEdit->text().length() > 0);
}

void WMSEditDialog::toggleOptions()
{
	_ui.formatComboBox->setEnabled(_ui.formatCheckBox->isChecked());
}

void WMSEditDialog::getLayerNames(osgEarthUtil::WMSLayer::LayerList& layers, std::vector<std::string>& names, std::map<std::string, std::string>& displayNames)
{
	for (int i=0; i < layers.size(); i++)
	{
		if (layers[i]->getName().size() > 0)
		{
			names.push_back(layers[i]->getName());

			if (layers[i]->getTitle().size() > 0)
				displayNames[layers[i]->getName()] = layers[i]->getTitle() + " (" + layers[i]->getName() + ")";
		}

		getLayerNames(layers[i]->getLayers(), names, displayNames);
	}
}