#include <qglobal.h>

#ifdef XDG_STATIC
# define XDG_API
#else
# if defined(XDG_LIBRARY)
#  define XDG_API Q_DECL_EXPORT
# else
#  define XDG_API Q_DECL_IMPORT
# endif
#endif
