#include "Triangle3D.h"

Triangle3D::Triangle3D() {

}

Triangle3D::Triangle3D( QVector3D p1, QVector3D p2, QVector3D p3 ) : v1(p1), v2(p2), v3(p3) {

}

Triangle3D::Triangle3D( const Triangle3D& t ) : v1(t.v1), v2(t.v2), v3(t.v3) {

}