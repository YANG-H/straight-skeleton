/**
 * ¿Õ¼äÈýÎ¬µã
 * @author wumabert, 20111215
 */
#ifndef TRIANGLE3D_H
#define  TRIANGLE3D_H

#include <QVector3D>

class Triangle3D {
public:
	QVector3D v1;
	QVector3D v2;
	QVector3D v3;

	Triangle3D();
	Triangle3D( QVector3D p1, QVector3D p2, QVector3D p3 );
	Triangle3D( const Triangle3D& t );
};

#endif