#include "ss2dwidget.h"

#include <QFile>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QGraphicsDropShadowEffect>
#include <QVector2D>
#include <QTextStream>
#include <QtDebug>

#include "sstask.h"
#include "ssconfigdlg.h"

double _vertex_radius = 7.0;
double _edge_width = 3.0;

// canvas
SSCanvas::SSCanvas(SS2DWidget* v)
: QGraphicsItem(0), m_view(v)
{
}

QRectF SSCanvas::boundingRect() const
{
	return QRectF(QPointF( - m_size.width()/2, - m_size.height()/2), m_size);
}
void SSCanvas::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget /* = 0 */)
{
	painter->setBackground(Qt::transparent);
	painter->eraseRect(boundingRect().adjusted(1, 1, -1, -1));
	painter->setPen(QPen(Qt::white, 1.0, Qt::DashLine));
	painter->drawRect(boundingRect());

	QPen _skeletonPen = QPen(m_view->property("-Skeleton Color").value<QColor>(), 1);
	QPen _wavefrontPen = QPen(m_view->property("-Wavefront Color").value<QColor>(), 2, Qt::DashLine);

	// draw skeletons
	if(m_view->property("-Show Skeleton").toBool()){
		painter->setPen(_skeletonPen);
		m_view->task()->drawSkeletons(painter);
	}
	// draw wavefronts
	if(m_view->property("-Show Wavefront").toBool()){
		painter->setPen(_wavefrontPen);
		m_view->task()->drawWaveFronts(painter);
	}

	
}


// vertex
SSVertex::SSVertex(SSCanvas* c)
: QGraphicsItem(c), m_canvas(c), m_from(0), m_to(0)
{
	setFlag(ItemIsMovable);
	setFlag(ItemIsSelectable);
	setFlag(ItemSendsGeometryChanges);
	setCacheMode(DeviceCoordinateCache);
	setZValue(100);	
}

QRectF SSVertex::boundingRect() const 
{
	return QRectF(- _vertex_radius, - _vertex_radius, 
		2 * _vertex_radius, 2 * _vertex_radius)
		.adjusted(-2, -2, 2, 2);
}

void SSVertex::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget /* = 0 */)
{
	static const QPen _pointPenCenter = QPen(Qt::white, 2.);
	static const QPen _selectedPointMarker = QPen(Qt::black, 1., Qt::DashLine);

	bool show = m_canvas->sswidget()->property("-Show Polygon Vertex").toBool();
	if(!show)
		return;

	QPen pen = QPen(m_canvas->sswidget()->property("-Polygon Vertex Color").value<QColor>(), 4.0);
	
	painter->setPen(_pointPenCenter);
	painter->drawEllipse(QPointF(), 2., 2.);
	painter->setPen(pen);
	painter->drawEllipse(QPointF(), 4., 4.);

	if(isSelected()){
		painter->setPen(_selectedPointMarker);
		painter->drawEllipse(QPointF(), 8.0, 8.0);
	}
}

bool SSVertex::validate(SSVertex* v, const QPointF& np )
{
	QList<SSEdge*> isPrevEdges;
	QList<SSEdge*> isNextEdges;

	if(!v->m_canvas->boundingRect().contains(np))
		return false;

	if(v->m_from){
		QLineF prevLine(v->m_from->prevV()->pos(), np);
		isPrevEdges = v->m_canvas->sswidget()->intersect(prevLine);

		for(int i = isPrevEdges.size() - 1; i >= 0; i--){
			isPrevEdges[i]->update();
			if(isPrevEdges[i]->nextTo(v->m_from))
				isPrevEdges.removeAt(i);
		}

		v->m_from->setHighlighted(!isPrevEdges.empty());
		v->m_from->update();
	}

	if(v->m_to){
		QLineF nextLine(np, v->m_to->nextV()->pos());
		isNextEdges = v->m_canvas->sswidget()->intersect(nextLine);

		for(int i = isNextEdges.size()- 1; i >= 0; i--){
			isNextEdges[i]->update();
			if(isNextEdges[i]->nextTo(v->m_to))
				isNextEdges.removeAt(i);
		}

		v->m_to->setHighlighted(!isNextEdges.empty());
		v->m_to->update();
	}	

	return isPrevEdges.empty() && isNextEdges.empty();
}

QVariant SSVertex::itemChange(GraphicsItemChange change, const QVariant &value)
{
	switch (change) {
	case ItemPositionChange:
		// validate
		return validate(this, value.toPointF()) ? value : pos();
		break;
	case ItemPositionHasChanged:
		if(m_from) m_from->adjust();
		if(m_to) m_to->adjust();
		break;
	default:
		break;
	};

	return QGraphicsItem::itemChange(change, value);
}

// edge
SSEdge::SSEdge(SSVertex* from, SSVertex* to, SSCanvas* c)
	: QGraphicsItem(c), m_from(from), m_to(to), m_canvas(c), m_highlighted(false)
{
	setAcceptedMouseButtons(0);
	from->setNextE(this);
	to->setPrevE(this);
	adjust();
}

void SSEdge::adjust()
{
	if (!m_from || !m_to)
		return;

	QLineF line(mapFromItem(m_from, 0, 0), mapFromItem(m_to, 0, 0));
	qreal length = line.length();

	prepareGeometryChange();
	m_fromP = line.p1();
	m_toP = line.p2();
}


QRectF SSEdge::boundingRect() const
{
	if (!m_from || !m_to)
		return QRectF();

	double extra = _edge_width + 10;
	return QRectF(m_fromP, QSizeF(m_toP.x() - m_fromP.x(),
		m_toP.y() - m_fromP.y()))
		.normalized()
		.adjusted(-extra, -extra, extra, extra);
}

void SSEdge::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget /* = 0 */)
{
	bool show = m_canvas->sswidget()->property("-Show Polygon Edge").toBool();
	if(!show)
		return;
	QColor edgeColor = m_canvas->sswidget()->property("-Polygon Edge Color").value<QColor>();
	
	QPen pen = QPen(edgeColor, _edge_width, Qt::SolidLine, Qt::RoundCap);
	QPen highlightPen = QPen(edgeColor,	_edge_width, Qt::DashLine, Qt::RoundCap);

	if (!m_from || !m_to)
		return;

	QLineF line(m_fromP, m_toP);
	if (qFuzzyCompare(line.length(), qreal(0.)))
		return;

	// Draw the line itself
	painter->setPen(m_highlighted ? highlightPen : pen);
	painter->drawLine(line);
	
	QPointF p1 = m_toP + (m_fromP - m_toP) * qMin(1.0, 10.0 / line.length());
	QPointF temp = m_toP - p1;
	QPointF p2 = p1 + QPointF(- temp.y(),  temp.x());
	QPainterPath arrow(m_toP);
	arrow.lineTo(p2);
	arrow.lineTo(p1);
	arrow.lineTo(m_toP);

	painter->fillPath(arrow, edgeColor);
}

// widget
SS2DWidget::SS2DWidget(QWidget *parent)
	: QGraphicsView(parent), m_task(0), m_fileInfo(tr("untitled.plgx")), m_canvas(0), 
	m_penCursor(QPixmap(":/nstyle/Resources/black/pencil_icon&16.png"), 0, 16)
{
	//setProperty("-Background Color", QColor(Qt::transparent));
	setProperty("-Polygon Edge Color", QColor(Qt::yellow));
	setProperty("-Polygon Vertex Color", QColor(Qt::red));
	setProperty("-Skeleton Color", QColor(Qt::black));
	setProperty("-Wavefront Color", QColor(Qt::red));

	setProperty("-Show Polygon Edge", true);
	setProperty("-Show Polygon Vertex", true);
	setProperty("-Show Skeleton", true);
	setProperty("-Show Wavefront", true);

	m_scene = new QGraphicsScene(this);
	setScene(m_scene);
	setBackgroundBrush(Qt::gray);
	setRenderHints(QPainter::Antialiasing);
	setInteractive(true);

	create(QSize(450, 650));
}

SS2DWidget::~SS2DWidget()
{

}

void SS2DWidget::fromPolygons( const QList<QPolygonF>& polygons, double scale /*= 1.0*/, bool adjustCanvasSize /*= false*/ )
{
	m_heads.clear();
    m_tails.clear();

	double xmax = - 1000000, ymax = -1000000, 
		xmin = 1000000, ymin = 1000000;
	for(int i  =0; i < polygons.size(); i++){
		for(int j = 0; j < polygons[i].size(); j++){
			double x = polygons[i][j].x();
			double y = polygons[i][j].y();

			if(x < xmin)
				xmin = x;
			if(y < ymin)
				ymin = y;
			if(x > xmax)
				xmax = x;
			if(y > ymax)
				ymax = y;
		}
	}

	QRectF boundingRect(xmin, ymin, xmax-xmin, ymax-ymin);
	if(adjustCanvasSize){
		QSizeF sizeF((xmax - xmin) * scale + 5, (ymax - ymin) * scale + 5);
		m_canvas->setSize(sizeF);
		emit canvasSizeChanged(sizeF);
	}

	foreach(QPolygonF polygon, polygons){
		if(polygon.size() <= 2)
			continue;

		SSVertex* firstV = new SSVertex(m_canvas);
		firstV->setPos((polygon.at(0) - boundingRect.center()) * scale);
		m_scene->addItem(firstV);
		m_heads.append(firstV);

		SSVertex* pre = firstV;
		for(int i = 1; i < polygon.size() - 1; i++){
			SSVertex* cur = new SSVertex(m_canvas);
			cur->setPos((polygon.at(i) - boundingRect.center()) * scale);
			SSEdge* e = new SSEdge(pre, cur, m_canvas);
			pre = cur;
			m_scene->addItem(cur);
			m_scene->addItem(e);
		}
		SSVertex* lastV = firstV;
		if(!polygon.isClosed()){
			lastV = new SSVertex(m_canvas);
			lastV->setPos((polygon.last() - boundingRect.center()) * scale);
			m_scene->addItem(lastV);
		}
		SSEdge* e = new SSEdge(pre, lastV, m_canvas);
		m_scene->addItem(e);
	}	
}

QList<QPolygonF> SS2DWidget::toPolygons()
{
	QList<QPolygonF> results;
	for(int i = 0; i < m_heads.size(); i++){
		QPolygonF polygon;
		SSVertex* head = m_heads.at(i);
		if(!head->nextE() || head->nextE()->nextV() == head 
			|| !head->prevE()) // ignore unclosed polygons
			continue;

		polygon.append(head->pos());
		SSVertex* cur = head;
		while(cur->nextE()){
			cur = cur->nextE()->nextV();
			if(polygon.last() != cur->pos())
				polygon.append(cur->pos());
			if(cur == head)
				break;
		}

		results << polygon;
	}
	return results;
}


void SS2DWidget::create( const QSizeF& s )
{
	if(m_canvas){
		m_scene->removeItem(m_canvas);
        m_heads.clear();
        m_tails.clear();
		delete m_canvas;
		m_canvas = 0;
	}
	m_canvas = new SSCanvas(this);

	m_canvas->setSize(s);
	emit canvasSizeChanged(s);
	m_scene->addItem(m_canvas);

	m_fileInfo = QFileInfo(tr("untitled.plgx"));
	setWindowFilePath(m_fileInfo.absoluteFilePath());
}

bool SS2DWidget::load()
{
	QFile file(m_fileInfo.absoluteFilePath());
	QPixmap backgroundImage;
	QList<QPolygonF> polygons;
	QSize canvasSize;

	if(m_fileInfo.suffix().toLower() == tr("plg")){	// old plg file	
		if(file.open(QIODevice::ReadOnly)){
			QSize canvasSize;
			QDataStream in(& file);
			in >> backgroundImage >> polygons >> canvasSize;

			if(m_canvas){
				m_scene->removeItem(m_canvas);
				delete m_canvas;
				m_canvas = 0;
			}
			m_canvas = new SSCanvas(this);

			m_canvas->setSize(canvasSize);
			m_scene->addItem(m_canvas);

			fromPolygons(polygons, 1.0, true);
			return true;
		}
	}
	else if(m_fileInfo.suffix().toLower() == tr("poly")){
		if(file.open(QIODevice::ReadOnly)){
			QTextStream ts(& file);
			QList<QPolygonF> polygons;

			int polygonNum = 0;
			ts >> polygonNum;

			for(int i = 0; i < polygonNum; i++){
				int polygonSize = 0;
				ts >> polygonSize;

				QPolygonF polygon;
				for(int j = 0; j < polygonSize; j++){
					double x, y;
					ts >> x >> y;
					polygon << QPointF(x, y);
				}
				polygon << polygon.first();
				polygons << polygon;
			}

			if(m_canvas){
				m_scene->removeItem(m_canvas);
				delete m_canvas;
				m_canvas = 0;
			}
			m_canvas = new SSCanvas(this);			

			m_scene->addItem(m_canvas);
			fromPolygons(polygons, 5000, true);
			return true;
		}
	}
	else if(m_fileInfo.suffix().toLower() == tr("plgx")){
		if(file.open(QIODevice::ReadOnly)){
			QSize canvasSize;
			QDataStream in(& file);
			in >> polygons >> canvasSize;

			if(m_canvas){
				m_scene->removeItem(m_canvas);
				delete m_canvas;
				m_canvas = 0;
			}
			m_canvas = new SSCanvas(this);

			m_canvas->setSize(canvasSize);
			emit canvasSizeChanged(canvasSize);
			m_scene->addItem(m_canvas);

			fromPolygons(polygons, 1.0, true);
			return true;
		}
	}

	return false;
}

bool SS2DWidget::save()
{
	if(m_fileInfo.suffix().toLower() == tr("plgx")){
		QFile file(m_fileInfo.absoluteFilePath());
		if(file.open(QIODevice::WriteOnly)){
			QDataStream out(& file);
			out << toPolygons() << m_canvas->size();
		}
		return true;
	}else if(m_fileInfo.suffix().toLower() == tr("plg")){
		QFile file(m_fileInfo.absoluteFilePath());
		QPixmap backgroundImage;
		if(file.open(QIODevice::WriteOnly)){
			QDataStream out(& file);
			out << backgroundImage << toPolygons() << m_canvas->size();
		}
		return true;
	}
	return false;
}

void SS2DWidget::mousePressEvent(QMouseEvent *e)
{
	if(m_insertState == InsertFirst && (e->buttons() & Qt::LeftButton)){
		SSVertex* v = insertAfter(mapToScene(e->pos()), 0);
		if(v){
			m_heads << v;
			m_tails << v;
			emit firstInserted();
		}
		// insert another
		v = insertAfter(mapToScene(e->pos()), m_heads.last());
		if(v){
			m_tails.last() = v;
		}
	}else if(m_insertState == InsertMore && (e->buttons() & Qt::LeftButton)){
		SSVertex* v = insertAfter(mapToScene(e->pos()), m_tails.last());
		if(v){
			m_tails.last() = v;
		}
	}else if(m_insertState == InsertMore && (e->buttons() & Qt::RightButton)){
		auto isEdges = intersect(QLineF(m_heads.last()->pos(), m_tails.last()->pos()));
		isEdges.removeOne(m_heads.last()->nextE());
		isEdges.removeOne(m_tails.last()->prevE());
		if(isEdges.empty() && m_heads.last() != m_tails.last()){
			SSEdge* edge = new SSEdge(m_tails.last(), m_heads.last(), m_canvas);
			m_scene->addItem(edge);
			emit polygonClosed();
		}
	}//else if(m_insertState == InsertMore && (e->buttons() & Qt::MidButton))
	//	emit polygonUnclosed();
	
	QGraphicsView::mousePressEvent(e);
}

void SS2DWidget::mouseMoveEvent(QMouseEvent *e)
{
	if(m_insertState == InsertMore){
		if(!m_tails.empty()){
			m_tails.last()->setPos(mapToScene(e->pos()));
		}
	}
	QGraphicsView::mouseMoveEvent(e);
}

void SS2DWidget::mouseReleaseEvent(QMouseEvent *e)
{
	QGraphicsView::mouseReleaseEvent(e);
}

void SS2DWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
	QGraphicsView::mouseDoubleClickEvent(e);
}

void SS2DWidget::wheelEvent(QWheelEvent *event)
{
	static double sc = 1.0;
	double s = exp(event->delta() / 1000.0);
	sc *= s;
	if(event->modifiers() & Qt::ControlModifier)
		scale(s, s);
	else
		QGraphicsView::wheelEvent(event);
}

QList<SSEdge*> SS2DWidget::intersect( const QLineF& line )
{
	QList<QGraphicsItem*> nearItems = m_scene->items(QRectF(line.p1(), line.p2()).normalized().adjusted(-1, -1, 1, 1), 
		Qt::IntersectsItemBoundingRect);
	QList<SSEdge*> edges;
	foreach(QGraphicsItem* i, nearItems){
		SSEdge* e = dynamic_cast<SSEdge*>(i);
		if(!e)
			continue;
		if(e->lineF().intersect(line, 0) == QLineF::BoundedIntersection)
			edges << e;
	}
	return edges;
}

SSVertex* SS2DWidget::insertSeeds(const QPointF& p1, const QPointF& p2, const QPointF& p3)
{
	if(p1 == p2 || p2 == p2 || p1 == p3)
		return 0;
	SSVertex* v1 = new SSVertex(m_canvas);
	SSVertex* v2 = new SSVertex(m_canvas);
	SSVertex* v3 = new SSVertex(m_canvas);
	v1->setPos(p1);
	v2->setPos(p2);
	v3->setPos(p3);

	SSEdge* e1 = new SSEdge(v1, v2, m_canvas);
	SSEdge* e2 = new SSEdge(v2, v3, m_canvas);
	SSEdge* e3 = new SSEdge(v3, v1, m_canvas);

	m_scene->addItem(v1);
	m_scene->addItem(v2);
	m_scene->addItem(v3);
	m_scene->addItem(e1);
	m_scene->addItem(e2);
	m_scene->addItem(e3);
	m_heads << v1;

	return v1;
}

SSVertex* SS2DWidget::insertAfter(const QPointF& p, SSVertex* v)
{
	if(!v)// first vertex
	{
		QLineF line(p, p);
		if(!intersect(line).empty())
			return 0;

		SSVertex* vertex = new SSVertex(m_canvas);
		vertex->setPos(p);
		m_scene->addItem(vertex);

		return vertex;
	}

	SSEdge* brokenEdge = v->nextE();

	if(brokenEdge) // v is not an end vertex 
	{
		SSVertex* v2 = brokenEdge->nextV();

		QLineF line1(v->pos(), p);
		QLineF line2(p, v2->pos());

		auto es1 = intersect(line1);
		auto es2 = intersect(line2);
		es1.removeOne(brokenEdge);
		es2.removeOne(brokenEdge);

		if(es1.empty() && es2.empty()) // no intersection
		{
			SSVertex* vertex = new SSVertex(m_canvas);
			vertex->setPos(p);
			SSEdge* e1 = new SSEdge(v, vertex, m_canvas);		
			SSEdge* e2 = new SSEdge(vertex, v2, m_canvas);
			m_scene->removeItem(brokenEdge);
			m_scene->addItem(vertex);
			m_scene->addItem(e1);
			m_scene->addItem(e2);

			return v2;
		}
		return 0;
	}

	// v is an end vertex
	QLineF line1(v->pos(), p);
	auto es1 = intersect(line1);
	//es1.removeOne(brokenEdge);

	if(es1.empty())
	{
		SSVertex* vertex = new SSVertex(m_canvas);
		vertex->setPos(p);
		SSEdge* e1 = new SSEdge(v, vertex, m_canvas);
		m_scene->addItem(vertex);
		m_scene->addItem(e1);
		return vertex;
	}
	return 0;
}

void SS2DWidget::remove(SSVertex* v)
{
	SSEdge* next = v->nextE();
	SSEdge* prev = v->prevE();
	if(next){
		
	}
}

void SS2DWidget::cancelInsertingPolygons()
{

}


