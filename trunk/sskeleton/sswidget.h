#ifndef SSWIDGET_H
#define SSWIDGET_H

#include <QWidget>
#include <QBrush>
#include <QFileInfo>
#include <QPoint>
#include <QVector2D>
#include <QPolygonF>
#include <QProgressBar>
#include <QElapsedTimer>

#include "MAlgorithm.h"

class PSLGSSkeleton;
class SSTask;

class SSWidget : public QWidget
{
	Q_OBJECT
	Q_PROPERTY(SSMode mode READ mode WRITE setMode)
	Q_PROPERTY(SSContents contents READ contents WRITE setContents)
	Q_ENUMS(SSMode)
	Q_FLAGS(SSContents)

public:
	enum SSMode {
		PrepareAddingPolygon, // prepare to add a new polygon 
		AddPoint, // continue adding points to current polygon
		SetNewPoint, // locating the new point
		SelectPoints, // select points
		EditPoints // edit selected points
	};
	enum SSContent {
		SelectedMarkers = 0x01,
		SelectArea      = 0x02,
		OverridePointMarker = 0x04
	};
	Q_DECLARE_FLAGS(SSContents, SSContent)
	typedef QPair<int, int> PointIndex;

signals:
	void changed(const QSize&, const QPixmap&);
	void taskFinished();

	// for state machine
	void polygonAdded();
	void pointAdded();
	void polygonClosed();
	void pointsSelected();
	void pointsLost();

public:
	SSWidget(SSTask* t, QWidget *parent = 0);
	~SSWidget();
	
	inline void setMode(SSMode s){m_mode = s;}
	inline SSMode mode() const {return m_mode;}
	inline void setContents(SSContents c) {m_displayContents = c;}
	inline SSContents contents() const {return m_displayContents;}

	void reset(const QSize& size, const QPixmap& image);

	inline void setFileInfo(const QFileInfo& fi) {m_fileInfo = fi;}
	inline void setPolygons(const QList<QPolygonF>& p) {m_polygons = p;}
	inline void setBackgroundImage(const QPixmap& image) {m_backgroundImage = image;}

	inline QFileInfo fileInfo() const {return m_fileInfo;}
	inline bool fileExists() const {return m_fileInfo.exists();}

	bool open();
	bool save();

	QSize sizeHint() const;

	inline void beginAddingPolygon(){setMode(PrepareAddingPolygon);}
	inline void beginEditingPolygon(){setMode(SelectPoints);}

	void motorGraph();
	void startExtractingSkeleton();
	void clear();

protected:
	QPointF aligned(const QPointF& pos);
	void select(const QRectF& area, int maxNum = -1);
	
	bool detectAll(const QPointF& pos, PointIndex& result = PointIndex());
	bool detectSelected(const QPointF& pos, PointIndex&  result = PointIndex());
	bool thenEdit(const QPointF& pos);
	void cleanDuplicates();

	void paintEvent(QPaintEvent *);
	void mousePressEvent(QMouseEvent *);
	void mouseMoveEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void mouseDoubleClickEvent(QMouseEvent *);

private:
	std::vector<Motorcycle> m_motors;
	std::vector<QLineF> m_segments;

	// file data
	QList<QPolygonF> m_polygons;
	SSTask* m_task;

	QPixmap m_backgroundImage;
	QFileInfo m_fileInfo;

	QList<PointIndex> m_focusedPoints; // polygon index; point index
	PointIndex m_hoveredPoint;
	//QList<PointIndex> m_selectedPoints;
	QRectF m_selectRect;
	QPointF m_lastPos;
	
	// flags
	SSMode m_mode;
	SSContents m_displayContents;

	QElapsedTimer m_elapsed;
};

#endif // SSWIDGET_H
