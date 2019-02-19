#ifndef _P_XML_FILE_H_
#define _P_XML_FILE_H_

#include "PPlatform.h"
#include "PString.h"

/// This class is used to read a XML file.  XML files are actually pretty slow and should be used
/// sparingly.  Ideally a released game does not use them, however, there are situations that are
/// appropriate for XML files.  This class is intended to read them only, not modify or create them.
/// Although this class could be adjusted in the future by removing the const references and adding
/// functions to set data.
/// Implementation reference http://en.wikipedia.org/wiki/XML
class PXML {
public:
	
	/// The tag <tag> of this xml element
	PString tag;
	
	/// The content <tag>content</tag> of this xml element
	PString content;
	
	/// A list of attributes <tag attribute="second"> for this xml element
	std::map<PString, PString> attributes;	// name/value
	
	/// Additional elements contained within this element
	std::multimap<PString, PXML*> elements; // name of element/element
	
	/// The parent of this xml element, or NULL if this is the root element
	PXML* parent;
	
	/// Default Constructor/Destructor
	PXML ();
	~PXML ();
	
	/// Load data from the path supplied (.xml)
	PXML (const PString& resource);
	
	/// Load data from the path supplied (.xml)
	bool NewFromFile (const PString& path);
	
	/// Delete the private data of this class
	void Delete ();
	
	/// Returns the name of this xml element
	const PString& GetTag () const;
	
	/// Returns the content of this xml element
	const PString& GetContent () const;
	
	/// Returns the value for the given name.  NULL is returned when the attribute is not found
	const PString* GetAttribute (const PString& name) const;
	
	/// Return the nth element (index) with the given name
	const PXML* GetElement (const PString& element, int_t index = 0) const;
	
	/// Returns the parent of this element or NULL if this is the root
	const PXML* GetParent () const;
	
	/// Returns a large string with the contents of this xml as UTF8 text
	PString GetString () const;
};

#endif // _P_XML_FILE_H_
