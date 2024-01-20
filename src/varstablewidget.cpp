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


const QString VarsTableModel::RESET = QString("x");
const QString VarsTableModel::CLEAR = QString(" ");


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
			emit valueUpdated(0);
			break;

		case Qt::Key_Plus:
			if (e->modifiers() & Qt::ControlModifier)
			{
				setPrecision(precision() + 1);
				QSettings().setValue(QString("varstablewidget/%1/precision").arg(objectName()), precision());
			}
			break;

		case Qt::Key_Minus:
			if (e->modifiers() & Qt::ControlModifier)
			{
				setPrecision(precision() - 1);
				QSettings().setValue(QString("varstablewidget/%1/precision").arg(objectName()), precision());
			}
			break;

		case Qt::Key_Delete:
		case Qt::Key_Backspace:
			{
				QModelIndex idx(currentIndex());
				clearVariationValue(idx);
				break;
			}

		case Qt::Key_Space:
			{
				QModelIndex idx(currentIndex());
				QModelIndex vidx(idx.sibling(idx.row(), 1));
				setCurrentIndex(vidx);
				edit(vidx);
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
				double item_data = vidx.data().toDouble();
				double inc_value = item_data + nstep;
				if (qFuzzyCompare(1 + inc_value, 1 + 0.0))
					inc_value = 0.0;
				setModelData(vidx, inc_value);
				e->accept();
				break;
			}

		default:
			QTreeView::keyPressEvent(e);
	}
}

void VarsTableWidget::clearVariationValue(QModelIndex& idx) {
	QModelIndex vidx(idx.sibling(idx.row(), 1));
	if (model()->data(vidx).toDouble() == 0.0)
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
				start_value = start_item.data().toDouble();
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

		double item_data = start_item.data().toDouble();
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
		&& start_value != start_item.data().toDouble())
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
			double current_value(idx.data().toDouble());
			double editor_value( lineEdit->text().toDouble(&ok) );
			if (ok && current_value != editor_value)
				setModelData(idx, editor_value);
		}
	}
}


void VarsTableWidget::setModelData(QModelIndex& idx, double value)
{
	QModelIndex vidx(idx.sibling(idx.row(), 1));
	QModelIndex ridx(idx.sibling(idx.row(), 2));
	model()->setData(vidx, QLocale().toString(value, 'f', vars_precision));
	model()->setData(ridx,
		value == 0.0 ? VarsTableModel::CLEAR : VarsTableModel::RESET);

	if (idx.parent().isValid())
		emit valueUpdated(idx.parent().row());
	else
		emit valueUpdated(idx.row());
}

//------------------------------------------------------------------------------

VarsTableItem::VarsTableItem(const QList<QVariant>& data, VarsTableItem* parent)
{
	parentItem = parent;
	itemData = data;
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

QVariant VarsTableItem::data(int column) const
{
	return itemData.value(column);
}

bool VarsTableItem::setData(int column, const QVariant& value)
{
	if (column < itemData.size())
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

VarsTableModel::VarsTableModel(QObject* parent) : QAbstractItemModel(parent),
	decimals(2)
{
	QList<QVariant> itemData;
	itemData << tr("Variation") << tr("Value") << " ";
	rootItem = new VarsTableItem(itemData);

	// variables map setup
	QMap<QString, QStringList*> variablesMap;
	foreach (QString variable, Util::get_variable_names())
	{
		int pos( variable.lastIndexOf(QChar('_')) );
		QString variation( variable.left(pos) );
		logFine(QString("VarsTableModel::VarsTableModel : adding %1 variables map %2")
			.arg(variation).arg(variable));
		if (!variablesMap.contains(variation))
		{
			QStringList* list = new QStringList();
			variablesMap.insert(variation, list);
		}
		variablesMap.value(variation)->append( variable );
	}

	foreach (QString variation, Util::variation_names())
	{
		itemData.clear();
		itemData << variation << "0.0" << " ";
		VarsTableItem* item = new VarsTableItem(itemData, rootItem);
		rootItem->appendChild(item);
		if (variablesMap.contains(variation))
			foreach (QString variable, *(variablesMap.value(variation)))
			{
				itemData.clear();
				itemData << variable << "0.0" << " ";
				VarsTableItem* variableItem = new VarsTableItem(itemData, item);
				item->appendChild(variableItem);
			}
		variationItems.insert(variation, item);
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
			if (item->parent() != rootItem && index.column() == 0)
			{
				// return only the unique part of a variation's variable name
				QString name( item->data(0).toString() );
				int pos( name.lastIndexOf(QChar('_')) + 1 );
				return name.mid(pos);
			}
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
			if (index.column() == 1 && QLocale().toDouble(item->data(1).toString()) != 0.0)
				return QApplication::palette().alternateBase();
			else
				return QApplication::palette().base();
		}
		case Qt::FontRole:
		{
			VarsTableItem* item = static_cast<VarsTableItem*>(index.internalPointer());
			if (index.column() == 1 && QLocale().toDouble(item->data(1).toString()) != 0.0)
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

	logFine(QString("VarsTableModel::setData : (%1,%2) %3").arg(index.row()).arg(index.column()).arg(value.toString()));
	if (index.column() > 0)
	{
		VarsTableItem* item = static_cast<VarsTableItem*>(index.internalPointer());
		if (item->setData(index.column(), value))
		{
			if (index.column() == 1)
			{
				if (item->parent() == rootItem)
					xform->var[index.row()] = value.toDouble();
				else
					Util::set_xform_variable(xform, item->data(0).toString(), value.toDouble());
			}

			emit dataChanged(index, index);
			return true;
		}
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
		return 0;

	if (index.column() == 1)
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;

	return Qt::ItemIsEnabled;
}

QVariant VarsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal)
			return rootItem->data(section);
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

void VarsTableModel::updateVarsTableItem(VarsTableItem* item, double value)
{
	QLocale l;
	item->setData(1, l.toString(value, 'f', decimals));
	item->setData(2, value == 0.0 ? CLEAR : RESET);
	int children = item->childCount();
	if (children > 0)
		for (int n = 0 ; n < children ; n++)
		{
			VarsTableItem* child = item->child(n);
			double value = Util::get_xform_variable(xform, child->data(0).toString());
			child->setData(1, l.toString(value, 'f', decimals));
			child->setData(2, value == 0.0 ? CLEAR : RESET);
		}
}

void VarsTableModel::setModelData(flam3_xform* xf)
{
	xform = xf;
	double* var = xform->var;

	for (int n = 0 ; n < flam3_nvariations ; n++)
	{
		double value = var[n];
		QString name(flam3_variation_names[n]);
		VarsTableItem* item = variationItems.value( name );
		updateVarsTableItem(item, value);
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
