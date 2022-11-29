#ifndef _GPACKAGE_H_
#define _GPACKAGE_H_

#include "GTypes.h"
#include "GSystem.h"
#include "GResource.h"
#include <map>
#include <list>

#define		GPACKAGE_AUTOLOAD(n)		_GPACKAGE_UNIQUE(n,__COUNTER__)
#define		_GPACKAGE_UNIQUE(n,u)		_GPACKAGE_STATIC(n,u)
#define		_GPACKAGE_STATIC(n,u)		static GPackage _GPACKAGE_ ## u ## _NAME(n, true);

class GPackage {
public:
	GPackage ();
	~GPackage ();
	
	/// This constructor calls OpenForRead
	GPackage (const GString& path);
	
	/// This constructor sets the default WD then calls OpenForRead
	GPackage (const GString& path, bool setDefaultWD);
	
	/// Opens a package for reading and adds itself to the global list of packages
	bool OpenForRead (const GString& path);
	
	/// Opens a package for writing
	bool OpenForWrite (const GString& path);
	
	/// Closes this package
	bool Close ();
	
	/// Write a resoruce to this package
	bool Write (const GString& resource, const void* data, int64_t size);
	
	/// Returns the size of a resourse found in the global list of packages
	static int64_t GetSize (const GString& resource);
	
	/// Reads data from a resource found in the global list of packages
	static bool Read (const GString& resource, void* data, int64_t size);
	
private:
	GResource							_file;
	int64_t								_footer;
	std::map<GString, int64_t>			_resources;
	static constexpr uint8_t			VERSION = 5;
	static constexpr const char			IDENTIFIER[] = "PACKAGE";
	static inline std::list<GPackage*>	PACKAGES;
	static inline GPackage*				CACHED_PACKAGE = nullptr;
	static inline GString				CACHED_RESOURCE;
	static inline int64_t				CACHED_SIZE = 0;
};

#endif // _GPACKAGE_H_
