#include <qglobal.h>

#if defined(XDG_LIBRARY)
# define XDG_API Q_DECL_EXPORT
#else
# define XDG_API Q_DECL_IMPORT
#endif
