#include "GXML.h"
#if PLATFORM_MACOSX || PLATFORM_IOS

@interface _XMLParser : NSXMLParser<NSXMLParserDelegate> {
	GXML* _xml;		// Pointer to main xml object
	GXML* _element;	// Pointer to the current element
}
@end

@implementation _XMLParser

- (void) setxml: (GXML*)xml {
	_xml = xml;
}

- (void) parserDidStartDocument: (NSXMLParser*)parser {
	_element = nil;
}

- (void) parser: (NSXMLParser*)parser didStartElement: (NSString*)elementName namespaceURI: (NSString*)namespaceURI qualifiedName: (NSString*)qName attributes: (NSDictionary*)attributeDict {
	if(_element) {
		GXML* element = new GXML;
		element->parent = _element;
		_element->elements.insert(std::make_pair([[elementName lowercaseString] UTF8String], element));
		_element = element;
	} else {
		_element = _xml;
	}
	
	// set the tag name
	_element->tag.New([[elementName lowercaseString] UTF8String]);
	
	// add the attributes
	NSEnumerator* enumerator = [attributeDict keyEnumerator];
	for(id key = [enumerator nextObject]; key; key = [enumerator nextObject])
		_element->attributes.insert(std::make_pair([[key lowercaseString] UTF8String], [[attributeDict objectForKey:key] UTF8String]));
}

- (void) parser: (NSXMLParser*)parser didEndElement: (NSString*)elementName namespaceURI: (NSString*)namespaceURI qualifiedName: (NSString*)qName {
	if(_element)
		_element = _element->parent;
}

- (void) parser: (NSXMLParser*)parser foundCharacters: (NSString*)string {
	if(_element)
		_element->content += [string UTF8String];
}

@end

GXML::GXML ()
:	parent(NULL)
{
}

GXML::~GXML () {
	Delete();
}

GXML::GXML (const GString& resource) {
	NewFromFile(resource);
}

bool GXML::NewFromFile (const GString& path) {
	Delete();
	
	_XMLParser* parser = [[_XMLParser alloc] initWithContentsOfURL:[NSURL fileURLWithPath:[NSString stringWithUTF8String:path] isDirectory:NO]];
	if(parser == nil)
		return false;
	
	[parser setxml:this];
	[parser setDelegate:parser];
	[parser parse];
	[parser setDelegate:nil];
	[parser setxml:nil];
	parser = nil;
	
	if(tag.IsEmpty())
		return false;
	
	return true;
}

void GXML::Delete () {
	for(std::multimap<GString, GXML*>::iterator i = elements.begin(); i != elements.end(); i++)
		if(i->second) {
			delete i->second;
			i->second = NULL;
		}
	tag.Delete();
	content.Delete();
	attributes.clear();
	elements.clear();
	parent = NULL;
}

const GString& GXML::GetTag () const {
	return tag;
}

const GString& GXML::GetContent () const {
	return content;
}

const GString* GXML::GetAttribute (const GString& name) const {
	std::map<GString, GString>::const_iterator find = attributes.find(GString(name).ToLower());
	if(find != attributes.end())
		return &find->second;
	return NULL;
}

const GXML* GXML::GetElement (const GString& element, int_t index) const {
	std::pair<std::multimap<GString, GXML*>::const_iterator, std::multimap<GString, GXML*>::const_iterator> find = elements.equal_range(GString(element).ToLower());
	for(std::multimap<GString, GXML*>::const_iterator i = find.first; i != find.second; i++)
		if(index-- <= 0)
			return i->second;
	return NULL;
}

const GXML* GXML::GetParent () const {
	return parent;
}

static GString _GetString (const GXML* xml, const GString& tab) {
	
	if(xml == NULL)
		return NULL;
	
	// Add tag
	GString string;
	string.Format("%s<%s", (const char*)tab, (const char*)xml->GetTag());
	
	// Add attributes
	for(std::map<GString, GString>::const_iterator i = xml->attributes.begin(); i != xml->attributes.end(); i++)
		string += GString().Format(" %s=\"%s\"", (const char*)i->first, (const char*)i->second);
	string += ">\n";
	
	// Add elements recursively
	for(std::multimap<GString, GXML*>::const_iterator i = xml->elements.begin(); i != xml->elements.end(); i++)
		string += _GetString(i->second, tab + "\t");
	
	// Add content (if it exists)
	if(!xml->content.IsEmpty())
		string += GString(xml->GetContent()).TrimSpaces();
	
	// End the tag
	string += GString().Format("%s</%s>\n", (const char*)tab, (const char*)xml->GetTag());
	
	return string;
}

GString GXML::GetString () const {
	return GString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n") + _GetString(this, "");
}

#endif // PLATFORM_MACOSX || PLATFORM_IOS
