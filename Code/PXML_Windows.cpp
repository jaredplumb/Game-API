#include "PXML.h"
#if PLATFORM_WINDOWS

static bool _ReadXML(CComPtr<IXmlReader>& reader, PXML* xml) {

	if (xml == NULL)
		return false;

	XmlNodeType nodeType;
	if (reader->Read(&nodeType) != S_OK)
		return true;

	HRESULT result;
	const WCHAR* name;
	const WCHAR* value;

	switch (nodeType) {
	case XmlNodeType_Element:
		// Get the element name
		if (FAILED(reader->GetLocalName(&name, NULL))) {
			//ERROR("Could not read xml element name!");
			return false;
		}

		// If the tag already exists, then this is a new element and a child needs to be spawned
		if (!xml->tag.IsEmpty()) {
			PXML* element = new PXML;
			element->parent = xml;
			xml->elements.insert(std::make_pair(PString().Format("%S", name), element));
			xml = element;
		}

		// Set the tag name
		xml->tag.Format("%S", name);

		// Add the attributes
		result = reader->MoveToFirstAttribute();
		if (result == S_FALSE) {
			// Do nothing
		}
		else if (FAILED(result)) {
			//ERROR("Could not read xml attributes!");
			return false;
		}
		else {
			do {
				if (FAILED(reader->GetLocalName(&name, NULL))) {
					//ERROR("Could not read XML attribute name!");
					return false;
				}

				if (FAILED(reader->GetValue(&value, NULL))) {
					//ERROR("Could not read XML attribute value!");
					return false;
				};

				// Add the specific attribute
				xml->attributes.insert(std::make_pair(PString().Format("%S", name), PString().Format("%S", value)));

			} while (reader->MoveToNextAttribute() == S_OK);
		}

		// Move to the end of the element
		if (FAILED(reader->MoveToElement())) {
			//ERROR("Could not finish reading the xml element!");
			return false;
		}

		// This element ends here, so advance back up to the parent
		if (reader->IsEmptyElement() && xml->parent)
			xml = xml->parent;

		break;
	case XmlNodeType_EndElement:
		// This is triggered from the end of an element, so go back to the parent
		if (xml->parent)
			xml = xml->parent;
		break;
	case XmlNodeType_Text:
		// Read the text value (or content value)
		if (FAILED(reader->GetValue(&value, NULL))) {
			//ERROR("Could not read XML text value!");
			return false;
		};

		// Add the value to the content
		xml->content += PString().Format("%S", value);

		break;
	default:
		break;
	};

	return _ReadXML(reader, xml);
}

PXML::PXML()
: parent(NULL)
{
}

PXML::~PXML() {
	Delete();
}

PXML::PXML(const PString& resource) {
	NewFromFile(resource);
}

bool PXML::NewFromFile(const PString& path) {
	
	Delete();

	CComPtr<IStream> fileStream;
	if (FAILED(SHCreateStreamOnFileA(path, STGM_READ, &fileStream))) {
		//ERROR("Could not open xml file \"%s\"!\n", (const char*)path);
		return false;
	}

	CComPtr<IXmlReader> reader;
	if (FAILED(CreateXmlReader(__uuidof(IXmlReader), (void**)&reader, NULL))) {
		//ERROR("Could not create xml reader!\n");
		return false;
	}

	if (FAILED(reader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit))) {
		//ERROR("Could not xml reader property!\n");
		return false;
	}

	if (FAILED(reader->SetInput(fileStream))) {
		//ERROR("Could not set file stream as xml input!\n");
		return false;
	}

	return _ReadXML(reader, this);
}

void PXML::Delete() {
	for (std::multimap<PString, PXML*>::iterator i = elements.begin(); i != elements.end(); i++)
	if (i->second) {
		delete i->second;
		i->second = NULL;
	}
	tag.Delete();
	content.Delete();
	attributes.clear();
	elements.clear();
	parent = NULL;
}

const PString& PXML::GetTag() const {
	return tag;
}

const PString& PXML::GetContent() const {
	return content;
}

const PString* PXML::GetAttribute(const PString& name) const {
	std::map<PString, PString>::const_iterator find = attributes.find(PString(name).ToLower());
	if (find != attributes.end())
		return &find->second;
	return NULL;
}

const PXML* PXML::GetElement(const PString& element, int_t index) const {
	std::pair<std::multimap<PString, PXML*>::const_iterator, std::multimap<PString, PXML*>::const_iterator> find = elements.equal_range(PString(element).ToLower());
	for (std::multimap<PString, PXML*>::const_iterator i = find.first; i != find.second; i++)
	if (index-- <= 0)
		return i->second;
	return NULL;
}

const PXML* PXML::GetParent() const {
	return parent;
}

static PString _GetString(const PXML* xml, const PString& tab) {

	if (xml == NULL)
		return NULL;

	// Add tag
	PString string;
	string.Format("%s<%s", (const char*)tab, (const char*)xml->GetTag());

	// Add attributes
	for (std::map<PString, PString>::const_iterator i = xml->attributes.begin(); i != xml->attributes.end(); i++)
		string += PString().Format(" %s=\"%s\"", (const char*)i->first, (const char*)i->second);
	string += ">\n";

	// Add elements recursively
	for (std::multimap<PString, PXML*>::const_iterator i = xml->elements.begin(); i != xml->elements.end(); i++)
		string += _GetString(i->second, tab + "\t");

	// Add content (if it exists)
	if (!xml->content.IsEmpty())
		string += PString(xml->GetContent()).TrimSpaces();

	// End the tag
	string += PString().Format("%s</%s>\n", (const char*)tab, (const char*)xml->GetTag());

	return string;
}

PString PXML::GetString() const {
	return PString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n") + _GetString(this, "");
}

#endif // PLATFORM_WINDOWS