#ifndef SSLIB_GLOBAL_H
#define SSLIB_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef SSLIB_LIB
# define SSLIB_EXPORT Q_DECL_EXPORT
#else
# define SSLIB_EXPORT Q_DECL_IMPORT
#endif

#endif // SSLIB_GLOBAL_H
