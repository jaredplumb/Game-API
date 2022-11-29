#ifndef MY_XML_H_
#define MY_XML_H_

#include "GTypes.h"
#include <map>

/// This class is used to read a XML file.  XML files are actually pretty slow and should be used
/// sparingly.  Ideally a released game does not use them, however, there are situations that are
/// appropriate for XML files.  This class is intended to read them only, not modify or create them.
/// Although this class could be adjusted in the future by removing the const references and adding
/// functions to set data.
/// Implementation reference http://en.wikipedia.org/wiki/XML
class MyXML {
public:
	
	/// The tag <tag> of this xml element
	GString tag;
	
	/// The content <tag>content</tag> of this xml element
	GString content;
	
	/// A list of attributes <tag attribute="second"> for this xml element
	std::map<GString, GString> attributes;	// name/value
	
	/// Additional elements contained within this element
	std::multimap<GString, MyXML*> elements; // name of element/element
	
	/// The parent of this xml element, or NULL if this is the root element
	MyXML* parent;
	
	/// Default Constructor/Destructor
	MyXML ();
	~MyXML ();
	
	/// Load data from the path supplied (.xml)
	MyXML (const GString& resource);
	
	/// Load data from the path supplied (.xml)
	bool NewFromFile (const GString& path);
	
	/// Delete the private data of this class
	void Delete ();
	
	/// Returns the name of this xml element
	const GString& GetTag () const;
	
	/// Returns the content of this xml element
	const GString& GetContent () const;
	
	/// Returns the value for the given name.  NULL is returned when the attribute is not found
	const GString* GetAttribute (const GString& name) const;
	
	/// Return the nth element (index) with the given name
	const MyXML* GetElement (const GString& element, int index = 0) const;
	
	/// Returns the parent of this element or NULL if this is the root
	const MyXML* GetParent () const;
	
	/// Returns a large string with the contents of this xml as UTF8 text
	GString GetString () const;
};

#endif // MY_XML_H_
