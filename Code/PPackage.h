#ifndef _P_PACKAGE_H_
#define _P_PACKAGE_H_

#include "GTypes.h"
#include "GSystem.h"
#include "PFile.h"
#include "PArchive.h"

#define		P_PACKAGE_AUTOLOAD(n)		_P_PACKAGE_UNIQUE(n,__COUNTER__)
#define		_P_PACKAGE_UNIQUE(n,u)		_P_PACKAGE_STATIC(n,u)
#define		_P_PACKAGE_STATIC(n,u)		static PPackage _P_PACKAGE_ ## u ## _NAME(n, true);

class PPackage {
public:
	
	PPackage ();
	~PPackage ();
	
	/// This constructor calls OpenForRead
	PPackage (const GString& path);
	
	/// This constructor sets the default WD then calls OpenForRead
	PPackage (const GString& path, bool setDefaultWD);
	
	/// Opens a package for reading and adds itself to the global list of packages
	bool OpenForRead (const GString& path);
	
	/// Opens a package for writing
	bool OpenForWrite (const GString& path);
	
	/// Closes this package
	bool Close ();
	
	/// Write a resoruce to this package
	bool Write (const GString& resource, const void* data, uint64 size);
	
	/// Returns the size of a resourse found in the global list of packages
	static uint64 GetSize (const GString& resource);
	
	/// Reads data from a resource found in the global list of packages
	static bool Read (const GString& resource, void* data, uint64 size);
	
private:
	PFile						_file;
	uint64						_footer;
	std::map<GString, uint64>	_resources;
};

#endif // _P_PACKAGE_H_
