#ifndef ETHANMLIB_GLOBAL_H
#define ETHANMLIB_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef ETHANMLIB_LIB
# define ETHANMLIB_EXPORT Q_DECL_EXPORT
#else
# define ETHANMLIB_EXPORT Q_DECL_IMPORT
#endif

#endif // ETHANMLIB_GLOBAL_H
