#ifndef SSPROPMODEL_H
#define SSPROPMODEL_H

#include <QObject>
#include <QAbstractItemModel>
#include <QDockWidget>

class SSPropModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	SSPropModel(QObject* obj, QObject *parent = 0);
	~SSPropModel();

	QVariant data(const QModelIndex &index, int role) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role);
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column,
		const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;

signals:
	void propChanged();

private:
	QObject* m_object;
};

class SSPropDock : public QDockWidget
{
public:
	SSPropDock(QObject* obj, QWidget* parent = 0);
	~SSPropDock();
};


#endif // SSPROPMODEL_H
