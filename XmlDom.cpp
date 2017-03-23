//============================================================================
// Name        : TvGuideFetch
// Author      : John Saunders
// Copyright   : Copyright (c) 2009 John Saunders
// Description : Fetch Australian XMLTV guide data from oztivo
//============================================================================

#include <cstring>

#include "XmlDom.hpp"

//////////////////////////////////////////////////////////////////////////////
// XmlObj
//////////////////////////////////////////////////////////////////////////////
XmlObj::XmlObj()
:   m_next(0)
{
}

XmlObj::~XmlObj()
{
}

XmlObj::XmlObj(const XmlObj &copy)
:   m_value(copy.m_value),
    m_next(0)
{
}

XmlObj &XmlObj::operator=(const XmlObj &copy)
{
    if (this != &copy)
    {
        m_value = copy.m_value;
    }

    return *this;
}

//////////////////////////////////////////////////////////////////////////////
// XmlCom
//////////////////////////////////////////////////////////////////////////////
XmlCom::XmlCom()
{
}

XmlCom::~XmlCom()
{
}

//////////////////////////////////////////////////////////////////////////////
// XmlChars
//////////////////////////////////////////////////////////////////////////////
XmlChars::XmlChars()
{
}

XmlChars::~XmlChars()
{
}

//////////////////////////////////////////////////////////////////////////////
// XmlAttr
//////////////////////////////////////////////////////////////////////////////
XmlAttr::XmlAttr(const std::string& name, const std::string& value)
:   m_name(name),
    m_value(value),
    m_next(0)
{
}

XmlAttr::~XmlAttr()
{
}

XmlAttr::XmlAttr(const XmlAttr &copy)
:   m_name(copy.m_name),
    m_value(copy.m_value),
    m_next(0)
{
}

XmlAttr &XmlAttr::operator=(const XmlAttr &copy)
{
    if (this != &copy)
    {
        m_name = copy.m_name;
        m_value = copy.m_value;
    }

    return *this;
}

//============================================================================

XmlNode::XmlNode(const std::string& name, XmlNode *parent)
:   m_name(name),
    m_parent(parent),
    m_firstAttribute(0),
    m_lastAttribute(0),
    m_firstChild(0),
    m_lastChild(0)
{
}

XmlNode::~XmlNode()
{
}

XmlNode::XmlNode(const XmlNode &copy)
:   m_parent(copy.m_parent),
    m_firstAttribute(copy.m_firstAttribute),
    m_lastAttribute(copy.m_lastAttribute),
    m_firstChild(copy.m_firstChild),
    m_lastChild(copy.m_lastChild)
{
}

XmlNode &XmlNode::operator=(const XmlNode &copy)
{
    if (this != &copy)
    {
        m_parent = copy.m_parent;
        m_firstAttribute = copy.m_firstAttribute;
        m_lastAttribute = copy.m_lastAttribute;
        m_firstChild = copy.m_firstChild;
        m_lastChild = copy.m_lastChild;
    }

    return *this;
}

void XmlNode::addAttribute(XmlAttr *attribute)
{
    attribute->setNext(0);
    if (m_lastAttribute)
    {
        m_lastAttribute->setNext(attribute);
    }
    else
    {
        m_firstAttribute = attribute;
    }
    m_lastAttribute = attribute;
}

void XmlNode::addChild(XmlObj *child)
{
    child->setNext(0);
    if (m_lastChild)
    {
        m_lastChild->setNext(child);
    }
    else
    {
        m_firstChild = child;
    }
    m_lastChild = child;
}

XmlAttr *XmlNode::findAttribute(const std::string& name)
{
    XmlAttr *attr = m_firstAttribute;
    while (attr)
    {
        if (attr->getName() == name)
        {
            return attr;
        }
        attr = attr->getNext();
    }

    return 0;
}

XmlNode *XmlNode::findChildNode(const std::string& name)
{
    XmlObj *child = m_firstChild;
    while (child)
    {
        XmlNode *node = dynamic_cast<XmlNode *>(child);
        if (node && (node->getName() == name))
        {
            return node;
        }
        child = child->getNext();
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////////
// XmlNav
//////////////////////////////////////////////////////////////////////////////
XmlNav::XmlNav(XmlDoc &doc)
:   m_doc(&doc),
    m_currentNode(0),
    m_currentAttr(0),
    m_currentChars(0)
{
    gotoRootNode();
}

XmlNav::~XmlNav()
{
}

XmlNav::XmlNav(const XmlNav &copy)
:   m_doc(copy.m_doc),
    m_currentNode(copy.m_currentNode),
    m_currentAttr(copy.m_currentAttr),
    m_currentChars(copy.m_currentChars)
{
}

XmlNav &XmlNav::operator=(const XmlNav &copy)
{
    if (this != &copy)
    {
        m_doc = copy.m_doc;
        m_currentNode = copy.m_currentNode;
        m_currentAttr = copy.m_currentAttr;
        m_currentChars = copy.m_currentChars;
    }

    return *this;
}

// node navigation
bool XmlNav::gotoNode(const std::string& path)
{
    std::string thePath(path);
    std::string name;

    // Handle the root element first.
    if (getPathElement(thePath, name))
    {
        if ((gotoRootNode() == false) || (getNodeName() != name))
        {
            return false;
        }
    }

    while (getPathElement(thePath, name))
    {
        if (gotoChildNode(name) == false)
        {
            return false;
        }
    }

    return true;
}

bool XmlNav::getPathElement(std::string& path, std::string& name) const
{
    if ((path.size() > 0) && (path[0] == '/'))
    {
        size_t pos = path.find('/', 1);
        if (pos == std::string::npos)
        {
            name = path.substr(1);
            path.clear();
        }
        else
        {
            name = path.substr(1, pos - 1);
            path = path.substr(pos);
        }

        return true;
    }

    return false;
}

bool XmlNav::gotoRootNode()
{
    m_currentNode = m_doc->m_rootNode;
    return m_currentNode != 0;
}

bool XmlNav::gotoParentNode()
{
    if (m_currentNode && m_currentNode->getParent())
    {
        m_currentNode = m_currentNode->getParent();
        return true;
    }

    return false;
}

bool XmlNav::gotoChildNode(const std::string& name)
{
    if (m_currentNode)
    {
        XmlNode *child = m_currentNode->findChildNode(name);
        if (child)
        {
            m_currentNode = child;
            return true;
        }
    }

    return false;
}

bool XmlNav::gotoFirstChildNode()
{
    XmlObj *child = m_currentNode->getFirstChild();

    while (child)
    {
        XmlNode *node = dynamic_cast<XmlNode *>(child);
        if (node)
        {
            m_currentNode = node;
            return true;
        }
        child = child->getNext();
    }

    return false;
}

bool XmlNav::gotoNextSiblingNode()
{
    XmlObj *sibling = m_currentNode->getNext();

    while (sibling)
    {
        XmlNode *node = dynamic_cast<XmlNode *>(sibling);
        if (node)
        {
            m_currentNode = node;
            return true;
        }
        sibling = sibling->getNext();
    }

    return false;
}

unsigned short XmlNav::getNodeLevel() const
{
    unsigned short level = 0;
    XmlNode *parent = m_currentNode;

    while (parent)
    {
        ++level;
        parent = parent->getParent();
    }

    return level;
}

std::string XmlNav::getNodeName() const
{
    if (m_currentNode)
    {
        return m_currentNode->getName();
    }

    return "";
}

// attribute navigation within the current node
bool XmlNav::gotoAttr(const std::string& id)
{
    if (m_currentNode)
    {
        XmlAttr *attr = m_currentNode->findAttribute(id);
        if (attr)
        {
            m_currentAttr = attr;
            return true;
        }
    }

    return false;
}

bool XmlNav::gotoFirstAttr()
{
    XmlAttr *attr = m_currentNode->getFirstAttribute();

    if (attr)
    {
        m_currentAttr = attr;
        return true;
    }

    return false;
}

bool XmlNav::gotoNextAttr()
{
    XmlAttr *attr = m_currentAttr->getNext();

    if (attr)
    {
        m_currentAttr = attr;
        return true;
    }

    return false;
}

std::string XmlNav::getAttrName() const
{
    if (m_currentAttr)
    {
        return m_currentAttr->getName();
    }

    return "";
}

std::string XmlNav::getAttrValue() const
{
    if (m_currentAttr)
    {
        return m_currentAttr->getValue();
    }

    return "";
}

bool XmlNav::setAttrValue(const std::string& val)
{
    if (m_currentAttr)
    {
        m_currentAttr->setValue(val);
    }

    return false;
}

// char data navigation within the current node
std::string XmlNav::getAllChars() const
{
    if (m_currentNode)
    {
        return m_currentNode->getValue();
    }

    return "";
}

bool XmlNav::setAllChars(const std::string& chars)
{
    if (m_currentNode)
    {
        m_currentNode->setValue(chars);
        return true;
    }

    return false;
}

bool XmlNav::gotoFirstChars()
{
    XmlObj *child = m_currentNode->getFirstChild();

    while (child)
    {
        XmlChars *chars = dynamic_cast<XmlChars *>(child);
        if (chars)
        {
            m_currentChars = chars;
            return true;
        }
        child = child->getNext();
    }

    return false;
}

bool XmlNav::gotoNextChars()
{
    XmlObj *sibling = m_currentChars->getNext();

    while (sibling)
    {
        XmlChars *chars = dynamic_cast<XmlChars *>(sibling);
        if (chars)
        {
            m_currentChars = chars;
            return true;
        }
        sibling = sibling->getNext();
    }

    return false;
}

std::string XmlNav::getChars() const
{
    if (m_currentChars)
    {
        return m_currentChars->getValue();
    }

    return "";
}

bool XmlNav::setChars(const std::string& chars)
{
    if (m_currentChars)
    {
        m_currentChars->setValue(chars);
        return true;
    }

    return false;
}

std::ostream &operator<<(std::ostream &s, const XmlNav &nav)
{
    XmlNav myNav(nav);
    std::string indent;

    indent.append((myNav.getNodeLevel() - 1) * 2, ' ');

    // Display the node start tag including attributes.
    s << indent << "<" << XmlDoc::quoteString(myNav.getNodeName());
    bool attrSuccess = myNav.gotoFirstAttr();
    while (attrSuccess)
    {
        s << " " << XmlDoc::quoteString(myNav.getAttrName());
        s << "=\"" << XmlDoc::quoteString(myNav.getAttrValue()) << "\"";
        attrSuccess = myNav.gotoNextAttr();
    }
    s << ">";

    // Display all of the children nodes.
    bool childSuccess = myNav.gotoFirstChildNode();
    if (childSuccess)
    {
        s << std::endl;
        while (childSuccess)
        {
            s << myNav << std::endl;
            childSuccess = myNav.gotoNextSiblingNode();
        }
        myNav.gotoParentNode();
        s << indent;
    }
    else
    {
        // Display all of the enclosed characters.
        s << XmlDoc::quoteString(myNav.getAllChars());
    }

    // Display the closing tag.
    s << "</" << XmlDoc::quoteString(myNav.getNodeName()) << ">";

    return s;
}

//////////////////////////////////////////////////////////////////////////////
// XmlDoc
//////////////////////////////////////////////////////////////////////////////

XmlDoc::XmlDoc()
:   m_parser(0),
    m_errorLineNumber(0),
    m_errorMessage(""),
    m_currentNode(0),
    m_rootNode(0)
{
}

XmlDoc::~XmlDoc()
{
}

XmlDoc::XmlDoc(const XmlDoc &copy)
:   m_parser(copy.m_parser),
    m_errorLineNumber(copy.m_errorLineNumber),
    m_errorMessage(copy.m_errorMessage),
    m_currentNode(copy.m_currentNode),
    m_rootNode(copy.m_rootNode)
{
}

XmlDoc &XmlDoc::operator=(const XmlDoc &copy)
{
    if (this != &copy)
    {
        m_parser = copy.m_parser;
        m_errorLineNumber = copy.m_errorLineNumber;
        m_errorMessage = copy.m_errorMessage;
        m_currentNode = copy.m_currentNode;
        m_rootNode = copy.m_rootNode;
    }

    return *this;
}

bool XmlDoc::parse(std::string s, bool frugalMemoryUse)
{
    if (!initialise(frugalMemoryUse))
    {
        return 0;
    }

    // Parse the entire string in one call.
    if (XML_Parse(m_parser, s.c_str(), s.size(), 1) == XML_STATUS_ERROR)
    {
        m_errorLineNumber = XML_GetCurrentLineNumber(m_parser);
        m_errorMessage = XML_ErrorString(XML_GetErrorCode(m_parser));
        cleanup();

        return false;
    }

    return true;
}

bool XmlDoc::parse(std::istream &s, bool frugalMemoryUse)
{
    if (!initialise(frugalMemoryUse))
    {
        return 0;
    }

    // Parse the stream a buffer at a time.
    char buffer[4096];
    while (s.good())
    {
        s.read(buffer, sizeof(buffer));
        if (s.bad()) break;

        if (XML_Parse(m_parser, buffer, s.gcount(), (s.eof() ? 1 : 0)) == XML_STATUS_ERROR)
        {
            m_errorLineNumber = XML_GetCurrentLineNumber(m_parser);
            m_errorMessage = XML_ErrorString(XML_GetErrorCode(m_parser));
            cleanup();

            return false;
        }
    }

    return true;
}

std::string XmlDoc::quoteString(const std::string& s)
{
    std::string quoted;

    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] == '&')
        {
            quoted.append("&amp;");
        }
        else if (s[i] == '<')
        {
            quoted.append("&lt;");
        }
        else if (s[i] == '>')
        {
            quoted.append("&gt;");
        }
        else if (s[i] == '"')
        {
            quoted.append("&quot;");
        }
        else if (s[i] == '\'')
        {
            quoted.append("&apos;");
        }
        else
        {
            quoted.append(1, s[i]);
        }
    }

    return quoted;
}

bool XmlDoc::initialise(bool frugalMemoryUse)
{
    // Create a parser instance when none already exists.
    if (m_parser == 0)
    {
        m_parser = XML_ParserCreate(0);
        if (m_parser == 0)
        {
            m_errorMessage = "Couldn't allocate memory for a parser";
            return false;
        }
    }
    else
    {
        // Reset the current parser to be used again.
        XML_ParserReset(m_parser, 0);
    }

    // Configure what data is available to the call-backs.
    XML_SetUserData(m_parser, this);
    XML_UseParserAsHandlerArg(m_parser);

    // Register the standard handlers.
    XML_SetElementHandler(m_parser, startHandler, endHandler);
    XML_SetCharacterDataHandler(m_parser, charHandler);
    XML_SetCommentHandler(m_parser, commentHandler);

    // Clear the error holding attributes.
    m_errorLineNumber = 0;
    m_errorMessage = "";

    // Initialise the node pointers.
    m_currentNode = 0;
    m_rootNode = 0;

    return true;
}

void XmlDoc::cleanup()
{
    delete m_rootNode;
    m_rootNode = 0;
    m_currentNode = 0;
}

void XmlDoc::handleStartTag(const std::string& el, const char **attr)
{
    XmlNode *node = new XmlNode(el, m_currentNode);
    if (node)
    {
        for (const char **p = attr; *p; p += 2)
        {
            XmlAttr *attribute = new XmlAttr(p[0], p[1]);
            if (attribute)
            {
                node->addAttribute(attribute);
            }
        }
        m_currentNode = node;
    }
}

void XmlDoc::handleEndTag(const std::string& el)
{
    if (m_currentNode)
    {
        XmlNode *parent = m_currentNode->getParent();
        if (parent)
        {
            parent->addChild(m_currentNode);
        }
        else
        {
            m_rootNode = m_currentNode;
        }
        m_currentNode = parent;
    }
}

void XmlDoc::handleChar(const std::string& txt)
{
    if (m_currentNode)
    {
        // Any text between the start and end nodes is added to the node value.
        m_currentNode->addValue(txt);

        // We also link the text as a child of the current node. This allows the
        // dump of the DOM to be character identical to the parsed file.
        XmlObj *object = new XmlChars();
        if (object)
        {
            object->setValue(txt);
            m_currentNode->addChild(object);
        }
    }
}

void XmlDoc::handleComment(const std::string& txt)
{
    if (m_currentNode)
    {
        // We add the comment as a child of the current node. This allows the
        // dump of the DOM to be character identical to the parsed file.
        XmlCom *comment = new XmlCom();
        if (comment)
        {
            comment->setValue(txt);
            m_currentNode->addChild(comment);
        }
    }
}

void XmlDoc::startHandler(void *data, const char *el, const char **attr)
{
    XmlDoc *doc = reinterpret_cast<XmlDoc*>(XML_GetUserData(data));
    doc->handleStartTag(el, attr);
}

void XmlDoc::endHandler(void *data, const char *el)
{
    XmlDoc *doc = reinterpret_cast<XmlDoc*>(XML_GetUserData(data));
    doc->handleEndTag(el);
}

void XmlDoc::charHandler(void *data, const char *txt, int txtlen)
{
    XmlDoc *doc = reinterpret_cast<XmlDoc*>(XML_GetUserData(data));
    doc->handleChar(std::string(txt, txtlen));
}

void XmlDoc::commentHandler(void *data, const char *txt)
{
    XmlDoc *doc = reinterpret_cast<XmlDoc*>(XML_GetUserData(data));
    doc->handleComment(txt);
}
