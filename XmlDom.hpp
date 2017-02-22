//============================================================================
// Name        : TvGuideFetch
// Author      : John Saunders
// Copyright   : Copyright (c) 2009 John Saunders
// Description : Fetch Australian XMLTV guide data from oztivo
//============================================================================
#ifndef XML_HPP
#define XML_HPP

#include <list>
#include <string>
#include <iostream>

#include "expat.h"

class XmlDoc;

//////////////////////////////////////////////////////////////////////////////
// XmlObj
//////////////////////////////////////////////////////////////////////////////
class XmlObj
{
public:
    XmlObj();
    virtual ~XmlObj();
    XmlObj(const XmlObj &copy);
    XmlObj &operator=(const XmlObj &copy);

    inline std::string getValue() const { return m_value; }
    inline void setValue(const std::string& value) { m_value = value; }
    inline void addValue(const std::string& value) { m_value.append(value); }

    inline XmlObj *getNext() const { return m_next; }
    inline void setNext(XmlObj *next) { m_next = next; }

protected:
    std::string m_value;
    XmlObj *m_next;
};

//////////////////////////////////////////////////////////////////////////////
// XmlCom
//////////////////////////////////////////////////////////////////////////////
class XmlCom : public XmlObj
{
public:
    XmlCom();
    virtual ~XmlCom();
};

//////////////////////////////////////////////////////////////////////////////
// XmlChars
//////////////////////////////////////////////////////////////////////////////
class XmlChars : public XmlObj
{
public:
    XmlChars();
    virtual ~XmlChars();
};

//////////////////////////////////////////////////////////////////////////////
// XmlAttr
//////////////////////////////////////////////////////////////////////////////
class XmlAttr
{
public:
    XmlAttr(const std::string& name, const std::string& value);
    virtual ~XmlAttr();
    XmlAttr(const XmlAttr &copy);
    XmlAttr &operator=(const XmlAttr &copy);

    inline std::string getName() const { return m_name; }
    inline void setName(const std::string& name) { m_name = name; }

    inline std::string getValue() const { return m_value; }
    inline void setValue(const std::string& value) { m_value = value; }

    inline XmlAttr *getNext() const { return m_next; }
    inline void setNext(XmlAttr *next) { m_next = next; }

private:
    XmlAttr();

    std::string m_name;
    std::string m_value;
    XmlAttr *m_next;
};

//////////////////////////////////////////////////////////////////////////////
// XmlNode
//////////////////////////////////////////////////////////////////////////////
class XmlNode : public XmlObj
{
public:
    XmlNode(const std::string& name, XmlNode *parent);
    virtual ~XmlNode();
    XmlNode(const XmlNode &copy);
    XmlNode &operator=(const XmlNode &copy);

    inline std::string getName() const { return m_name; }

    inline XmlNode *getParent() const { return m_parent; }

    inline XmlAttr *getFirstAttribute() const { return m_firstAttribute; }
    XmlAttr *findAttribute(const std::string& name);
    void addAttribute(XmlAttr *attribute);

    inline XmlObj *getFirstChild() const { return m_firstChild; }
    XmlNode *findChildNode(const std::string& name);
    void addChild(XmlObj *child);

private:
    XmlNode();

    std::string m_name;
    XmlNode *m_parent;
    XmlAttr *m_firstAttribute;
    XmlAttr *m_lastAttribute;
    XmlObj *m_firstChild;
    XmlObj *m_lastChild;
};

//////////////////////////////////////////////////////////////////////////////
// XmlNav
//////////////////////////////////////////////////////////////////////////////
class XmlNav
{
public:
    explicit XmlNav(XmlDoc &doc);
    virtual ~XmlNav();
    XmlNav(const XmlNav &copy);
    XmlNav &operator=(const XmlNav &copy);

    // node navigation
    bool gotoNode(const std::string& path);
    bool gotoRootNode();
    bool gotoParentNode();
    bool gotoChildNode(const std::string& name);
    bool gotoFirstChildNode();
    bool gotoNextSiblingNode();
    unsigned short getNodeLevel() const;
    std::string getNodeName() const;

    // attribute navigation within the current node
    bool gotoAttr(const std::string& id);
    bool gotoFirstAttr();
    bool gotoNextAttr();
    std::string getAttrName() const;
    std::string getAttrValue() const;
    bool setAttrValue(const std::string& val);

    // char data navigation within the current node
    std::string getAllChars() const;
    bool setAllChars(const std::string& chars);
    bool gotoFirstChars();
    bool gotoNextChars();
    std::string getChars() const;
    bool setChars(const std::string& chars);

private:
    XmlNav();
    bool getPathElement(std::string& path, std::string& name) const;

    XmlDoc *m_doc;
    XmlNode *m_currentNode;
    XmlAttr *m_currentAttr;
    XmlChars *m_currentChars;
};

std::ostream &operator<<(std::ostream &s, const XmlNav &nav);

//////////////////////////////////////////////////////////////////////////////
// XmlDoc
//////////////////////////////////////////////////////////////////////////////
class XmlDoc
{
public:
    XmlDoc();
    virtual ~XmlDoc();
    XmlDoc(const XmlDoc &copy);
    XmlDoc &operator=(const XmlDoc &copy);

    bool parse(std::string s, bool frugalMemoryUse = false);
    bool parse(std::istream &s, bool frugalMemoryUse = false);

    inline int getErrorLineNumber() const { return m_errorLineNumber; }
    inline std::string getErrorMessage() const { return m_errorMessage; }

    static std::string quoteString(const std::string& s);

    inline bool isLoaded() const { return m_rootNode != 0; }

    inline XmlNav getNav() { return XmlNav(*this); }

    friend class XmlNav;

private:
    bool initialise(bool frugalMemoryUse);
    void cleanup();
    void handleStartTag(const std::string& el, const char **attr);
    void handleEndTag(const std::string& el);
    void handleChar(const std::string& txt);
    void handleComment(const std::string& data);
    static void startHandler(void *data, const char *el, const char **attr);
    static void endHandler(void *data, const char *el);
    static void charHandler(void *data, const char *txt, int txtlen);
    static void commentHandler(void *data, const char *txt);

    XML_Parser m_parser;
    int m_errorLineNumber;
    std::string m_errorMessage;

    XmlNode *m_currentNode;
    XmlNode *m_rootNode;
};

#endif
