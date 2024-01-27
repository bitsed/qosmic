/***************************************************************************
 *   Copyright (C) 2007-2016 by David Bitseff                              *
 *   bitsed@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/
#include <QApplication>
#include <QSettings>
#include <QLineEdit>
#include <QHeaderView>

#include "varstablewidget.h"
#include "logger.h"


VarsTableWidget::VarsTableWidget(QWidget* parent)
	: QTreeView(parent)
{
	step = 0.1;
	start_value = 0.0;
	vars_precision = 4;
	setEditTriggers(QAbstractItemView::DoubleClicked);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setSelectionBehavior(QAbstractItemView::SelectItems);
	header()->setSectionsMovable(false);
}

void VarsTableWidget::dataChanged(const QModelIndex& top, const QModelIndex& bottom)
{
	QTreeView::dataChanged(top, bottom);
	showHideNullRows();
}

void VarsTableWidget::showHideNullRows()
{
	int rows = model()->rowCount();
	QModelIndex idx(currentIndex());

	// only show/hide children of rootItem
	if (idx.parent().internalId() != model()->index(0,0).parent().internalId())
		idx = idx.parent();

	for (int n = 0 ; n < rows ; n++)
	{
		QModelIndex vidx = model()->index(n, 1);
		if (vidx.data(0).toDouble() == 0.0)
		{
			if (n == idx.row())
			{
				if (hasFocus())
					setRowHidden(n, vidx.parent(), false);
				else
					setRowHidden(n, vidx.parent(), true);
			}
			else
				setRowHidden(n, vidx.parent(), true);
		}
		else
			setRowHidden(n, vidx.parent(), false);

		setExpanded(model()->index(n, 0), true);
	}
}

void VarsTableWidget::restoreSettings()
{
	QSettings s;
	step = s.value(QString("varstablewidget/%1/step")
		.arg(objectName()), step).toDouble();

	vars_precision = s.value(QString("varstablewidget/%1/precision")
		.arg(objectName()), vars_precision).toInt();
}

void VarsTableWidget::keyPressEvent(QKeyEvent* e)
{
	switch (e->key())
	{
		case Qt::Key_BracketLeft:
			step *= 10.0;
			QSettings().setValue(QString("varstablewidget/%1/step").arg(objectName()), step);
			break;

		case Qt::Key_BracketRight:
			step *= 0.10;
			QSettings().setValue(QString("varstablewidget/%1/step").arg(objectName()), step);
			break;

		case Qt::Key_Return:
			break;

		case Qt::Key_Plus:
		case Qt::Key_Minus:
			if (e->modifiers() & Qt::ControlModifier)
			{
				setPrecision(precision() + (e->key() == Qt::Key_Plus ? 1 : -1));
				QSettings().setValue(QString("varstablewidget/%1/precision").arg(objectName()), precision());
				break;
			}
			// fall through
		case Qt::Key_Comma:
		case Qt::Key_Period:
		case Qt::Key_0:
		case Qt::Key_1:
		case Qt::Key_2:
		case Qt::Key_3:
		case Qt::Key_4:
		case Qt::Key_5:
		case Qt::Key_6:
		case Qt::Key_7:
		case Qt::Key_8:
		case Qt::Key_9:
		case Qt::Key_Space:
			{
				QModelIndex idx(currentIndex());
				QModelIndex vidx(idx.sibling(idx.row(), 1));
				setCurrentIndex(vidx);
				if (edit(vidx, QAbstractItemView::AllEditTriggers, e))
				{   // Display variation or variable text editor
					QLineEdit* editor(qobject_cast<QLineEdit*>(indexWidget(vidx)));
					if (e->key() != Qt::Key_Space)
					{   // Clear the editor value and add typed numeral character
						editor->setText(e->text());
					}
				}
				break;
			}

		case Qt::Key_Delete:
		case Qt::Key_Backspace:
			{
				QModelIndex idx(currentIndex());
				clearVariationValue(idx);
				break;
			}

		case Qt::Key_Right:
			{
				QModelIndex idx(currentIndex());
				QTreeView::keyPressEvent(e);
				if (idx.column() == 1)
					setCurrentIndex(idx);
				break;
			}

		case Qt::Key_Up:
		case Qt::Key_Down:
			if (e->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier ))
			{
				double nstep = step;
				if (e->modifiers() & Qt::ShiftModifier)
					nstep *= 0.10;
				else if (e->modifiers() & Qt::ControlModifier)
					nstep *= 10.0;

				if (e->key() == Qt::Key_Down)
					nstep *= -1.0;

				QModelIndex idx(currentIndex());
				QModelIndex vidx(idx.sibling(idx.row(), 1));
				double item_data = vidx.data(Qt::UserRole).toDouble();
				double inc_value = item_data + nstep;
				if (qFuzzyCompare(1 + inc_value, 1 + 0.0))
					inc_value = 0.0;
				setModelData(vidx, inc_value);
				e->accept();
				break;
			}
			// fall through

		default:
			QTreeView::keyPressEvent(e);
	}
}

void VarsTableWidget::clearVariationValue(QModelIndex& idx) {
	QModelIndex vidx(idx.sibling(idx.row(), 1));
	if (model()->data(vidx, Qt::UserRole).toDouble() == 0.0)
		return;
	setModelData(vidx, 0.0);
	emit undoStateSignal();
}

void VarsTableWidget::mousePressEvent(QMouseEvent* e)
{
	switch (e->button())
	{
		case Qt::LeftButton:
		{
			QModelIndex idx( indexAt(e->pos()) );
			if (idx.column() == 1)
			{
				start_item = idx;
				last_pos = e->localPos();
				start_value = start_item.data(Qt::UserRole).toDouble();
				e->accept();
			}
			else if (idx.column() == 2)
			{
				clearVariationValue(idx);
				e->accept();
				return;
			}
			else
				start_item = QModelIndex();
		}

		default:
			;
	}
	QTreeView::mousePressEvent(e);
}

void VarsTableWidget::mouseMoveEvent(QMouseEvent* e)
{
	if ((e->buttons() & Qt::LeftButton) && start_item.isValid())
	{
		double nstep = step;
		if (e->modifiers() & Qt::ShiftModifier)
			nstep *= 0.10;
		else if (e->modifiers() & Qt::ControlModifier)
			nstep *= 10.0;

		double dy = e->y() - last_pos.y();
		last_pos = e->localPos();
		if (dy == 0.0) return;
		if (dy > 0)
			nstep *= -1.0;

		double item_data = start_item.data(Qt::UserRole).toDouble();
		double inc_value = item_data + nstep;
		if (qFuzzyCompare(1 + inc_value, 1 + 0.0))
			inc_value = 0.0;
		setModelData(start_item, inc_value);
		e->accept();
	}
}

void VarsTableWidget::mouseReleaseEvent(QMouseEvent* e)
{
	if ((e->button() == Qt::LeftButton)
		&& start_item.isValid()
		&& start_value != start_item.data(Qt::UserRole).toDouble())
	{
		start_item = QModelIndex();
		e->accept();
		emit undoStateSignal();
	}
	QTreeView::mouseReleaseEvent(e);
}

int VarsTableWidget::precision()
{
	return vars_precision;
}

void VarsTableWidget::setPrecision(int n)
{
	if (vars_precision != n)
	{
		vars_precision = qMax(1, n);
		emit precisionChanged();
	}
}

void VarsTableWidget::commitData(QWidget* editor)
{
	QModelIndex idx(currentIndex());
	if (idx.column() == 1)
	{
		QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editor);
		if (lineEdit)
		{
			bool ok;
			double current_value(idx.data(Qt::UserRole).toDouble());
			double editor_value(QLocale().toDouble(lineEdit->text(), &ok));
			if (ok && current_value != editor_value)
			{
				setModelData(idx, editor_value);
				emit undoStateSignal();
			}
		}
	}
}


void VarsTableWidget::setModelData(QModelIndex& idx, double value)
{
	QModelIndex vidx(idx.sibling(idx.row(), 1));
	model()->setData(vidx, value);

	if (idx.parent().isValid())
		emit valueUpdated(idx.parent().row());
	else
		emit valueUpdated(idx.row());
}

//------------------------------------------------------------------------------

VarsTableItem::VarsTableItem(const QList<QVariant>& data, VarsTableItem* parent)
	: parentItem(parent), itemData(data)
{
	if (parentItem)
	{   // Store the variation or variable's unaltered name in varName.
		// All varName values have format "variation_name"
		//  or "variation_name_variable_name"
		// Variable names start with a variation's name + '_'.
		QString parentName(parentItem->varName + '_');
		QString myName(itemData.at(0).toString());
		varName = myName;  // UserRole value
		if (myName.startsWith(parentName))
		{   // Store the variable's DisaplayRole label.
			// Remove the variation name, and replace underscore
			itemData[0].setValue(
				myName.mid(parentName.size()).replace('_', ' '));
		}
		else
		{   // Store the variation's label
			itemData[0].setValue(myName.replace('_', ' '));
		}
	}
}

VarsTableItem::~VarsTableItem()
{
	qDeleteAll(childItems);
}

void VarsTableItem::appendChild(VarsTableItem* item)
{
	childItems.append(item);
}

VarsTableItem* VarsTableItem::child(int row)
{
	return childItems.value(row);
}

int VarsTableItem::childCount() const
{
	return childItems.count();
}

int VarsTableItem::row() const
{
	if (parentItem)
		return parentItem->childItems.indexOf(const_cast<VarsTableItem*>(this));

	return 0;
}

int VarsTableItem::columnCount() const
{
	return itemData.count();
}

QVariant VarsTableItem::data(int column, int role) const
{
	if (role == Qt::UserRole)
	{
		if (column == 0)
			return varName;
	}
	else if (column == 1)
	{
		return *(double*)(itemData.at(1).value<void*>());
	}
	return itemData.value(column);
}

bool VarsTableItem::setData(int column, const QVariant& value, int role)
{
	if (column == 1 && role != Qt::UserRole)
	{
		*(double*)(itemData.at(1).value<void*>()) = value.toDouble();
		return true;
	}
	else if (column < itemData.size())
	{
		itemData.replace(column, value);
		return true;
	}
	return false;
}

VarsTableItem* VarsTableItem::parent()
{
	return parentItem;
}


// -----------------------------------------------------------------------------

// DisplayRole labels stored in a VarsTableItem column 2
const QString VarsTableModel::RESET = QString("x");
const QString VarsTableModel::CLEAR = QString(" ");

// Default UserRole value stored in a VarsTableItem column 1
const double   VarsTableModel::ZEROD = 0.0;
const QVariant VarsTableModel::ZEROV =
	QVariant::fromValue((void*)&VarsTableModel::ZEROD);


VarsTableModel::VarsTableModel(QObject* parent) : QAbstractItemModel(parent),
	decimals(2)
{
	headerItems << tr("Variation") << tr("Value") << " ";

	QList<QVariant> itemData;
	itemData << "" << ZEROV << "";
	rootItem = new VarsTableItem(itemData);

	// variables map setup
	QMap<QString, QStringList*> variablesMap;
	QStringList& varList = Util::get_variable_names();
	QStringList::iterator variable;
	for ( variable = varList.begin() ; variable != varList.end() ; ++variable )
	{   // Create a map of variations that have adjustable variables
		// Entries in varList have format "variation_name.variable_name"
		QString variation( variable->left(variable->indexOf('.')) );
		logFine(QString("VarsTableModel::VarsTableModel : adding %1 variables map %2")
			.arg(variation).arg(*variable));
		if (!variablesMap.contains(variation))
		{   // Create a list of variables for a variation
			variablesMap.insert(variation, new QStringList());
		}
		// Append variable name "variation_name_variable_name" to the map
		variablesMap.value(variation)->append( variable->replace('.', '_') );
	}

	foreach (QString variation, Util::variation_names())
	{   // Contruct the table of variations and their possible variables
		// A varItem includes is a row entry in a table with three columns
		// [0] QString label, [1] double* ref, [2] reset 'x' label
		itemData.clear();
		itemData << variation << ZEROV << " ";  // null root item
		VarsTableItem* varItem = new VarsTableItem(itemData, rootItem);
		rootItem->appendChild(varItem);
		if (variablesMap.contains(variation))
		{   // This variation has variables
			foreach (QString variable, *(variablesMap.value(variation)))
			{   // Variable items are children of variation items
				itemData.clear();
				itemData << variable << ZEROV << " ";
				VarsTableItem* variableItem = new VarsTableItem(itemData, varItem);
				varItem->appendChild(variableItem);
			}
		}
		variationItems.insert(variation, varItem);
	}

	qDeleteAll(variablesMap);
}

VarsTableModel::~VarsTableModel()
{
	delete rootItem;
}

int VarsTableModel::columnCount(const QModelIndex& parent) const
{
	if (parent.isValid())
		return static_cast<VarsTableItem*>(parent.internalPointer())->columnCount();
	else
		return rootItem->columnCount();
}

int VarsTableModel::rowCount(const QModelIndex& parent) const
{
	VarsTableItem* parentItem;
	if (parent.column() > 0)
		return 0;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<VarsTableItem*>(parent.internalPointer());

	return parentItem->childCount();
}

QVariant VarsTableModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	switch (role)
	{
		case Qt::DisplayRole:
		case Qt::EditRole:
		{
			VarsTableItem* item = static_cast<VarsTableItem*>(index.internalPointer());
			if (index.column() == 1)
				return QLocale().toString(item->data(1).toDouble(), 'f', decimals);
			else
				return item->data(index.column());
		}
		case Qt::UserRole:
		{
			VarsTableItem* item = static_cast<VarsTableItem*>(index.internalPointer());
			return item->data(index.column());
		}
		case Qt::BackgroundRole:
		{
			VarsTableItem* item = static_cast<VarsTableItem*>(index.internalPointer());
			if (item->data(1).toDouble() != 0.0)
				return QApplication::palette().alternateBase();
			break;
		}
		case Qt::FontRole:
		{
			VarsTableItem* item = static_cast<VarsTableItem*>(index.internalPointer());
			if (item->data(1).toDouble() != 0.0)
			{
				QFont f = QApplication::font();
				f.setBold(true);
				return f;
			}
			break;
		}
		case Qt::TextAlignmentRole:
		{
			if (index.column() == 2)
				return Qt::AlignHCenter;
			break;
		}
	}
	return QVariant();
}

bool VarsTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (!index.isValid())
		return false;

	if (role != Qt::EditRole)
		return false;

	logFine(QString("VarsTableModel::setData : (%1,%2) %3")
		.arg(index.row()).arg(index.column()).arg(value.toString()));

	if (index.column() == 1)
	{
		VarsTableItem* item = static_cast<VarsTableItem*>(index.internalPointer());
		if (item->setData(1, value))
		{
			item->setData(2, value.toDouble() == 0.0 ?  CLEAR : RESET);
			emit dataChanged(index.siblingAtColumn(0), index.siblingAtColumn(2));
		}
		return true;
	}

	return false;
}

QModelIndex VarsTableModel::index(int row, int column, const QModelIndex& parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	VarsTableItem* parentItem;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<VarsTableItem*>(parent.internalPointer());

	VarsTableItem* childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex VarsTableModel::parent(const QModelIndex& index) const
{
	if (!index.isValid())
		return QModelIndex();

	VarsTableItem* childItem = static_cast<VarsTableItem*>(index.internalPointer());
	VarsTableItem* parentItem = childItem->parent();

	if (parentItem == rootItem)
		return QModelIndex();

	return createIndex(parentItem->row(), 0, parentItem);
}

Qt::ItemFlags VarsTableModel::flags(const QModelIndex& index) const
{
	if (!index.isValid())
		return Qt::NoItemFlags;

	if (index.column() == 1)
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;

	return Qt::ItemIsEnabled;
}

QVariant VarsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal)
			return headerItems[section];
		else
			return rootItem->child(section)->data(0);
	}
	return QVariant();
}

VarsTableItem* VarsTableModel::getVariation(int row) const
{
	if (row >= rowCount())
		return rootItem;

	return rootItem->child(row);
}

VarsTableItem* VarsTableModel::getItem(const QModelIndex& index) const
{
	if (!index.isValid())
		return rootItem;

	return static_cast<VarsTableItem*>(index.internalPointer());
}

void VarsTableModel::updateVarsTableItem(VarsTableItem* item, double* var_ptr)
{
	item->setData(1, QVariant::fromValue((void*)var_ptr), Qt::UserRole);
	item->setData(2, *var_ptr == 0.0 ? CLEAR : RESET);
	int children = item->childCount();
	if (children > 0)
	{
		for (int n = 0 ; n < children ; n++)
		{
			VarsTableItem* child = item->child(n);
			double* variable_ptr
				= Util::get_xform_variable_ref(xform,
					child->data(0, Qt::UserRole).toString());
			child->setData(1, QVariant::fromValue((void*)variable_ptr), Qt::UserRole);
			child->setData(2, *variable_ptr == 0.0 ? CLEAR : RESET);
		}
	}
}

void VarsTableModel::setModelData(flam3_xform* xf)
{
	xform = xf;
	double* var = xform->var;

	for (int n = 0 ; n < flam3_nvariations ; n++)
	{
		double* var_ptr = &var[n];
		QString name(flam3_variation_names[n]);
		VarsTableItem* item = variationItems.value( name );
		updateVarsTableItem(item, var_ptr);
	}
	emit dataChanged(index(0,0), index(flam3_nvariations - 1, 1));
}

int VarsTableModel::precision() const
{
	return decimals;
}

void VarsTableModel::setPrecision(int n)
{
	if (decimals != n)
	{
		decimals = qMax(1, n);
	}
}
