#ifndef SS2DWIDGET_H
#define SS2DWIDGET_H

#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QFileInfo>
#include <QPoint>
#include <QVector2D>
#include <QPolygonF>
#include <QProgressBar>

#include "MAlgorithm.h"

class SSTask;
class SS2DWidget;
class SSCanvas;
class SSVertex;
class SSEdge;

class SSCanvas : public QGraphicsItem
{
public:
	SSCanvas(SS2DWidget* v);
	~SSCanvas(){}

	inline void setSize(const QSizeF& s){prepareGeometryChange();m_size = s;}
	inline QSizeF size() const {return m_size;}
	inline SS2DWidget* sswidget() const {return m_view;}

	QRectF boundingRect() const;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);

private:
	SS2DWidget* m_view;
	QSizeF m_size;
};

class SSVertex : public QGraphicsItem
{
public:
	SSVertex(SSCanvas* c);
	~SSVertex(){}

	QRectF boundingRect() const;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);

	inline void setPrevE(SSEdge* e){m_from = e;}
	inline void setNextE(SSEdge* e){m_to = e;}
	inline bool isEndVertex() const {return m_from == 0 || m_to == 0;}
	
	inline SSEdge* prevE() const {return m_from;}
	inline SSEdge* nextE() const {return m_to;}

	static bool validate(SSVertex* v, const QPointF& np);

private:
	SSEdge* m_from;
	SSEdge* m_to;
	SSCanvas* m_canvas;
};

class SSEdge : public QGraphicsItem
{
public:
	SSEdge(SSVertex* v1, SSVertex* v2, SSCanvas* c);
	~SSEdge(){}

	QRectF boundingRect() const;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);
	void adjust();

	inline SSVertex* prevV() const {return m_from;}
	inline SSVertex* nextV() const {return m_to;}

	inline void setHighlighted(bool on){m_highlighted = on;}
	inline QLineF lineF() const{return QLineF(mapFromItem(m_from, 0, 0), mapFromItem(m_to, 0, 0));}
	inline bool nextTo(SSEdge* e) const {return m_from == e->m_from || m_to == e->m_to 
		|| m_from == e->m_to || m_to == e->m_from;}

private:
	SSVertex* m_from;
	SSVertex* m_to;
	QPointF m_fromP, m_toP;
	SSCanvas* m_canvas;

	bool m_highlighted;
};

class SS2DWidget : public QGraphicsView
{
	Q_OBJECT
	Q_ENUMS(State)
	Q_PROPERTY(State insertState READ insertState WRITE setInsertState)
	//Q_PROPERTY(QCursor, canvasCursor READ canvasCursor WRITE setCanvasCursor)

public:
	enum State {NoInsert, InsertFirst, InsertMore};

	SS2DWidget(QWidget *parent = 0);
	~SS2DWidget();

	inline void bind(SSTask* t){m_task = t;}

	inline void setInsertState(State s){
		m_insertState = s;
		if(s == InsertFirst || s == InsertMore)
			m_canvas->setCursor(m_penCursor);
		else
			m_canvas->unsetCursor();
	}
	inline State insertState() const {return m_insertState;}

	void create(const QSizeF& s);
	bool load();
	bool save();

	inline SSTask* task() const {return m_task;}

	inline void setFileInfo(const QFileInfo& fi) {m_fileInfo = fi; setWindowFilePath(fi.absoluteFilePath());}
	inline QFileInfo fileInfo() const {return m_fileInfo;}
	inline bool fileExists() const {return m_fileInfo.exists();}

	QList<QPolygonF> toPolygons();
	void fromPolygons(const QList<QPolygonF>& polygons, double scale = 1.0, bool adjustCanvasSize = false);

	QList<SSEdge*> intersect( const QLineF& line );

signals:
	void canvasSizeChanged(const QSizeF& s);
	void canvasBackgroundChanged(const QPixmap& p);

	void firstInserted();
	void polygonClosed();
	void polygonUnclosed();

public slots:
	void updateCanvas(){m_canvas->update();}
	void cancelInsertingPolygons();

protected:
	void mousePressEvent(QMouseEvent *);
	void mouseMoveEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void mouseDoubleClickEvent(QMouseEvent *);
	void wheelEvent(QWheelEvent *event);

	SSVertex* insertSeeds(const QPointF& p1, const QPointF& p2, const QPointF& p3);
	SSVertex* insertAfter(const QPointF& p, SSVertex* v);
	void remove(SSVertex* v);

private:
	SSTask* m_task;
	QFileInfo m_fileInfo;

	QList<SSVertex*> m_heads;
	QList<SSVertex*> m_tails;

	QGraphicsScene* m_scene;
	SSCanvas* m_canvas;

	State m_insertState;
	QCursor m_penCursor;
};

#endif // SS2DWIDGET_H
