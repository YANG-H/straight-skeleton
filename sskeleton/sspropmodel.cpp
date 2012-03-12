#include "sspropmodel.h"

#include <QIcon>
#include <QBoxLayout>
#include <QTableView>
#include <QTreeView>
#include <QScrollArea>

SSPropModel::SSPropModel(QObject* obj, QObject *parent)
	: QAbstractItemModel(parent), m_object(obj)
{
	connect(this, SIGNAL(propChanged()), obj, SLOT(update()));
	connect(this, SIGNAL(propChanged()), obj, SLOT(updateCanvas()));
}

SSPropModel::~SSPropModel()
{

}

QVariant SSPropModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid())
		return QVariant();
	QList<QByteArray> names = m_object->dynamicPropertyNames();
	if(role == Qt::DisplayRole || role == Qt::EditRole){
		int col = index.column();
		int row = index.row();
		if(!names.at(row).startsWith("-"))
			return tr("#System Property#");
		return col == 0 ? names.at(row).mid(1) : m_object->property(names.at(row));
	}
	else if(role == Qt::DecorationRole && index.column() == 0){
		if(names.at(index.row()).startsWith("-")){
			return QIcon(":/SSMainWind/Resources/gear.png");
		}
	}else if(role == Qt::BackgroundRole){
		if(!names.at(index.row()).startsWith("-")){
			return QBrush(Qt::transparent);
		}
	}else if(role == Qt::ForegroundRole){
		if(!names.at(index.row()).startsWith("-")){
			return QBrush(Qt::white);
		}
		if(index.column() == 1){
			QVariant& p = m_object->property(names.at(index.row()));
			if(p.type() == QVariant::Color)
				return QBrush(p.value<QColor>());
		}
	}
	return QVariant();
}

bool SSPropModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if(role == Qt::EditRole && index.isValid() && index.column() == 1){
		QList<QByteArray> names = m_object->dynamicPropertyNames();
		m_object->setProperty(names.at(index.row()), value);
		emit dataChanged(index, index);
		emit propChanged();
		return true;
	}		
	return false;
}

Qt::ItemFlags SSPropModel::flags(const QModelIndex &index) const
{
	if(!index.isValid())
		return 0;
	QList<QByteArray> names = m_object->dynamicPropertyNames();
	if(index.column() == 0)
		return (Qt::ItemIsEnabled | Qt::ItemIsSelectable); 
	else if(!names.at(index.row()).startsWith("-"))
		return 0;
	return (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
}

QVariant SSPropModel::headerData(int section, Qt::Orientation orientation, 
	int role /* = Qt::DisplayRole */) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return section == 0 ? tr("Name") : tr("Value");
	return QVariant();
}

QModelIndex SSPropModel::index(int row, int column, const QModelIndex &parent /* = QModelIndex */) const
{
	if(!hasIndex(row, column, parent))
		return QModelIndex();
	if(parent.isValid())
		return QModelIndex();
	return createIndex(row, column);
}

QModelIndex SSPropModel::parent(const QModelIndex &index) const
{
	return QModelIndex();
}

int SSPropModel::rowCount(const QModelIndex &parent) const
{
	if(!parent.isValid())
		return m_object->isWidgetType() ? m_object->dynamicPropertyNames().size() - 1 : 
		// there's a default q_XXX property in all widgets!
			m_object->dynamicPropertyNames().size();
	return 0;
}

int SSPropModel::columnCount(const QModelIndex &parent /* = QModelIndex */) const
{
	return 2;
}

SSPropDock::SSPropDock(QObject* o, QWidget* parent /* = 0 */) 
	: QDockWidget(tr("Properties"), parent)
{
	QTreeView* vw = new QTreeView;
	vw->setModel(new SSPropModel(o, this));
	//vw->setAlternatingRowColors(true);
	vw->setAutoScroll(true);
	vw->setIndentation(0);
	//vw->setHeaderHidden(true);
	setWidget(vw);
}

SSPropDock::~SSPropDock(){}
