#include "sswidget.h"

#include <QPainter>
#include <QPen>
#include <QTimer>
#include <QElapsedTimer>
#include <QMouseEvent>
#include <QBitmap>
#include <QCursor>
#include <QtDebug>

#include "pslgsskeleton.h"
#include "pslggraph.h"
#include "sstask.h"

static const double _pointRadius = 6.0;
static const double _lineWidth = 3.0;

SSWidget::SSWidget(SSTask* t, QWidget *parent)
	: QWidget(parent), m_task(t)
{
	setMouseTracking(true);
	clear();
	setMode(PrepareAddingPolygon);

	setProperty("-Background Color", QColor(Qt::white));
	setProperty("-Polygon Edge Color", QColor(Qt::darkCyan));
	setProperty("-Polygon Vertex Color", QColor(Qt::red));
	setProperty("-Skeleton Color", QColor(Qt::darkCyan));
	setProperty("-Wavefront Color", QColor(Qt::red));
	
	setProperty("-Show Polygon Edge", true);
	setProperty("-Show Polygon Vertex", true);
	setProperty("-Show Skeleton", true);
	setProperty("-Show Wavefront", true);
}

SSWidget::~SSWidget()
{}

void SSWidget::reset(const QSize& size, const QPixmap& image)
{
	resize(size); 
	m_backgroundImage = image;
	m_fileInfo = QFileInfo(tr("untitled"));
	m_elapsed.start();

	emit changed(size, image);
}

QSize SSWidget::sizeHint() const {return QSize(800, 500);}

QPointF SSWidget::aligned(const QPointF& pos)
{
	m_focusedPoints.clear();
	for(int i = 0; i < m_polygons.size(); i++){
		QPolygonF& plg = m_polygons[i];
		for(int j = 0; j < plg.size(); j++){
			QPointF& p = plg[j];
			if(QVector2D(p - pos).lengthSquared() <= _pointRadius * _pointRadius){
				m_focusedPoints.append(PointIndex(i, j));
				return p;
			}
		}
	}
	return pos;
}

void SSWidget::select(const QRectF& area, int maxNum)
{
	m_focusedPoints.clear();
	for(int i = 0; i < m_polygons.size(); i++){
		QPolygonF& plg = m_polygons[i];
		for(int j = 0; j < plg.size(); j++){
			QPointF& p = plg[j];
			if(area.adjusted(-_pointRadius, -_pointRadius, 
				_pointRadius, _pointRadius).contains(p)){
				m_focusedPoints.append(PointIndex(i, j));
				if(maxNum > 0 && m_focusedPoints.size() >= maxNum)
					return;
			}
		}
	}
}

bool SSWidget::detectAll(const QPointF& pos, PointIndex& result)
{
	for(int i = 0; i < m_polygons.size(); i++){
		QPolygonF& plg = m_polygons[i];
		for(int j = 0; j < plg.size(); j++){
			QPointF& p = plg[j];
			if(QVector2D(p - pos).lengthSquared() <= _pointRadius * _pointRadius){
				result = PointIndex(i, j);
				return true;
			}
		}
	}
	return false;
}

bool SSWidget::detectSelected(const QPointF& pos, PointIndex& result)
{
	for(int i = 0; i < m_focusedPoints.size(); i++){
		PointIndex& index = m_focusedPoints[i];
		QPointF& p = m_polygons[index.first][index.second];
		if(QVector2D(p - pos).lengthSquared() <= _pointRadius * _pointRadius){
			result = index;
			return true;
		}
	}
	return false;
}

bool SSWidget::thenEdit(const QPointF& pos)
{
	if(detectSelected(pos))
		return true;
	m_focusedPoints.clear();
	PointIndex index;
	if(detectAll(pos, index)){
		m_focusedPoints.append(index);
		return true;
	}
	return false;
}

void SSWidget::cleanDuplicates()
{
	m_focusedPoints.clear();
	for(int i = m_polygons.size() - 1; i >= 0; i--){
		QPolygonF& plg = m_polygons[i];
		for(int j = plg.size() - 1; j >= 1 ; j--){
			if(plg[j] == plg[j - 1])
				plg.remove(j);
		}
		if(plg.size() <= 1){
			m_polygons.removeAt(i);
			continue;
		}
	}
}

void SSWidget::paintEvent(QPaintEvent * e)
{
	static const double _pointPenWidth = 4.0;
	static const double _selectedPointRadius = 8.0;

	//static const QBrush _defultBackgroundBrush = QBrush(QColor(250, 240, 245));
	static const QBrush _selectAreaBrush = QBrush(QColor(100, 100, 200, 80));

	QPen _skeletonPen = QPen(property("-Skeleton Color").value<QColor>(), 1);
	QPen _wavefrontPen = QPen(property("-Wavefront Color").value<QColor>(), 2, Qt::DashLine);
	QPen _polygonPen = QPen(property("-Polygon Edge Color").value<QColor>(), _lineWidth, Qt::SolidLine, Qt::RoundCap);
	static const QPen _pointPenCenter = QPen(Qt::white, 2);
	QPen _pointPen = QPen(property("-Polygon Vertex Color").value<QColor>(), _pointPenWidth);
	static const QPen _selectedPointMarker = QPen(Qt::black, 1, Qt::DashLine);
	static const QPen _selectAreaBoundPen = QPen(Qt::blue, 1);

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	if(m_backgroundImage.isNull())
		painter.setBackground(property("-Background Color").value<QColor>());
	else
		painter.setBackground(m_backgroundImage);
	painter.eraseRect(rect());

	// draw skeletons
	if(property("-Show Skeleton").toBool()){
		painter.setPen(_skeletonPen);
		m_task->drawSkeletons(& painter);
	}
	// draw wavefronts
	if(property("-Show Wavefront").toBool()){
		painter.setPen(_wavefrontPen);
		m_task->drawWaveFronts(& painter);
	}

	bool showEdges = property("-Show Polygon Edge").toBool();
	bool showVertex = property("-Show Polygon Vertex").toBool();
	// draw polygons
	for(int i = 0; i < m_polygons.size(); i++){
		painter.setPen(_polygonPen);
		if(showEdges)
			painter.drawPolyline(m_polygons[i]);
		if(showVertex)
		for(int j = 0; j < m_polygons[i].size(); j++){
			painter.setPen(_pointPenCenter);
			painter.drawEllipse(m_polygons[i][j], 2, 2);
			painter.setPen(_pointPen);
			double radius = _pointRadius - _pointPenWidth / 2.0;
			painter.drawEllipse(m_polygons[i][j], radius, radius);
		}
	}

	// draw selected points
	painter.setPen(_selectedPointMarker);
	for(int i = 0; i < m_focusedPoints.size(); i++){
		PointIndex& index = m_focusedPoints[i];		
		painter.drawEllipse(m_polygons[index.first][index.second], 
			_selectedPointRadius, _selectedPointRadius);
	}

	// draw select rect
	if(m_displayContents & SelectArea){
		painter.fillRect(m_selectRect, _selectAreaBrush);
		painter.setPen(_selectAreaBoundPen);
		painter.drawRect(m_selectRect);
	}

	// draw motorcycles:
	/*painter.setPen(QPen(Qt::white, 2));
	for(int i = 0; i  < m_segments.size(); i++)
		if(m_segments.at(i).length() > 2)
			painter.drawLine(m_segments.at(i));

	for(int i = 0; i < m_motors.size(); i++){
		painter.setPen(QPen(Qt::yellow, 2));
		painter.drawEllipse(m_motors.at(i).position, 3, 3);
		painter.drawEllipse(m_motors.at(i).position + m_motors.at(i).speed.toPointF(), 1, 1);
	}*/
}

void SSWidget::mousePressEvent(QMouseEvent * e)
{
	static int idprovider = 0;
	if(e->buttons() & Qt::LeftButton){
		switch (m_mode){
		case PrepareAddingPolygon:
			m_polygons.push_back(QPolygonF(QVector<QPointF>() << aligned(e->posF())));
			//emit polygonAdded();
			break;
		case AddPoint:
			if(m_polygons.empty())
				m_polygons.push_back(QPolygonF(QVector<QPointF>() << aligned(e->posF())));
			else
				m_polygons.last() << aligned(e->posF());
			break;

		case SelectPoints:

		case EditPoints:
			if(thenEdit(e->posF())){
				m_lastPos = e->posF();
				m_mode = EditPoints;
				setCursor(Qt::ClosedHandCursor);
			}else{
				m_selectRect.setTopLeft(e->posF());
				m_selectRect.setBottomRight(e->posF());
				m_displayContents |= SelectArea;
				m_mode = SelectPoints;
			}			
			break;
		default:
			break;
		}
	}
	update();
}

void SSWidget::mouseMoveEvent(QMouseEvent * e)
{	
	QPointF alignedP;
	switch (m_mode){
	case PrepareAddingPolygon:
	case AddPoint:
		alignedP = aligned(e->posF());
		if(e->buttons() & Qt::LeftButton) 
			// move the last point if dragging
			m_polygons.last().last() = alignedP;
		update();
		break;

	case SelectPoints:
		if(e->buttons() & Qt::LeftButton){
			m_selectRect.setBottomRight(e->posF());
			select(m_selectRect);
			update();
		}
		break;
	case EditPoints:
		if(e->buttons() & Qt::LeftButton){
			for(int i = 0; i < m_focusedPoints.size(); i++){
				PointIndex& index = m_focusedPoints[i];
				m_polygons[index.first][index.second] += (e->posF() - m_lastPos);
			}
			m_lastPos = e->posF();
			update();
		}
	default:
		break;
	}
}

void SSWidget::mouseReleaseEvent(QMouseEvent * e)
{
	switch (m_mode){
	case PrepareAddingPolygon:
		m_mode = AddPoint;
		break;
	case AddPoint:
		if(!m_polygons.empty()){
			if(m_polygons.last().isClosed() && m_polygons.last().size() >= 3)
				m_mode = PrepareAddingPolygon;
		}
		break;
	
	case SelectPoints:
		m_selectRect = QRectF();
		m_displayContents &= (~SelectArea);
		update();
		break;
	case EditPoints: setCursor(Qt::CrossCursor);break;
	default: break;
	}
}

void SSWidget::mouseDoubleClickEvent(QMouseEvent * e)
{
	switch (m_mode){
	case PrepareAddingPolygon:
		break;
	case AddPoint:
		if(!m_polygons.empty()){
			m_polygons.last().append(m_polygons.last().first());
			// do not change mode to AddPolygon here, do not add point here either,
			// this shall be done by mousePressEvent & mouseReleaseEvent
			update();
		}
		break;
	case SelectPoints: break;
	case EditPoints: break;
	default: break;
	}
}

bool SSWidget::open()
{
	clear();
	QFile file(m_fileInfo.absoluteFilePath());
	if(file.open(QIODevice::ReadOnly)){
		QSize canvasSize;
		QDataStream in(& file);
		in >> m_backgroundImage >> m_polygons >> canvasSize;
		resize(canvasSize);
		emit changed(canvasSize, m_backgroundImage);
		update();
		return true;
	}	
	return false;
}

bool SSWidget::save()
{
	QFile file(m_fileInfo.absoluteFilePath());
	if(file.open(QIODevice::WriteOnly)){
		QDataStream out(& file);
		out << m_backgroundImage << m_polygons << size();
		update();
		return true;
	}
	return false;
}


void SSWidget::motorGraph()
{
	GetMotorcycleGraphBF(m_motors, m_segments);
	update();
}

void SSWidget::startExtractingSkeleton()
{
	cleanDuplicates();
	if(m_polygons.empty() || m_task->isRunning())
		return;
	m_task->startTask(m_polygons);
}

void SSWidget::clear()
{
	m_polygons.clear();
	m_focusedPoints.clear();
	m_selectRect = QRect();
	m_motors.clear();
	m_segments.clear();
	//m_skeletons.clear();
	//m_wavefronts.clear();
	m_task->terminate();
	m_task->cleanSkeleton();

	//setMode(AddPoint ? AddPolygon : m_mode);
	m_displayContents = 0;
	update();
}