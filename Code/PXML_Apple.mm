#include "PXML.h"
#if PLATFORM_MACOSX

@interface _XMLParser : NSXMLParser<NSXMLParserDelegate> {
	PXML* _xml;		// Pointer to main xml object
	PXML* _element;	// Pointer to the current element
}
@end

@implementation _XMLParser

- (void) setxml: (PXML*)xml {
	_xml = xml;
}

- (void) parserDidStartDocument: (NSXMLParser*)parser {
	_element = nil;
}

- (void) parser: (NSXMLParser*)parser didStartElement: (NSString*)elementName namespaceURI: (NSString*)namespaceURI qualifiedName: (NSString*)qName attributes: (NSDictionary*)attributeDict {
	if(_element) {
		PXML* element = new PXML;
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

PXML::PXML ()
:	parent(NULL)
{
}

PXML::~PXML () {
	Delete();
}

PXML::PXML (const PString& resource) {
	NewFromFile(resource);
}

bool PXML::NewFromFile (const PString& path) {
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

void PXML::Delete () {
	for(std::multimap<PString, PXML*>::iterator i = elements.begin(); i != elements.end(); i++)
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

const PString& PXML::GetTag () const {
	return tag;
}

const PString& PXML::GetContent () const {
	return content;
}

const PString* PXML::GetAttribute (const PString& name) const {
	std::map<PString, PString>::const_iterator find = attributes.find(PString(name).ToLower());
	if(find != attributes.end())
		return &find->second;
	return NULL;
}

const PXML* PXML::GetElement (const PString& element, int_t index) const {
	std::pair<std::multimap<PString, PXML*>::const_iterator, std::multimap<PString, PXML*>::const_iterator> find = elements.equal_range(PString(element).ToLower());
	for(std::multimap<PString, PXML*>::const_iterator i = find.first; i != find.second; i++)
		if(index-- <= 0)
			return i->second;
	return NULL;
}

const PXML* PXML::GetParent () const {
	return parent;
}

static PString _GetString (const PXML* xml, const PString& tab) {
	
	if(xml == NULL)
		return NULL;
	
	// Add tag
	PString string;
	string.Format("%s<%s", (const char*)tab, (const char*)xml->GetTag());
	
	// Add attributes
	for(std::map<PString, PString>::const_iterator i = xml->attributes.begin(); i != xml->attributes.end(); i++)
		string += PString().Format(" %s=\"%s\"", (const char*)i->first, (const char*)i->second);
	string += ">\n";
	
	// Add elements recursively
	for(std::multimap<PString, PXML*>::const_iterator i = xml->elements.begin(); i != xml->elements.end(); i++)
		string += _GetString(i->second, tab + "\t");
	
	// Add content (if it exists)
	if(!xml->content.IsEmpty())
		string += PString(xml->GetContent()).TrimSpaces();
	
	// End the tag
	string += PString().Format("%s</%s>\n", (const char*)tab, (const char*)xml->GetTag());
	
	return string;
}

PString PXML::GetString () const {
	return PString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n") + _GetString(this, "");
}

#endif // PLATFORM_MACOSX
