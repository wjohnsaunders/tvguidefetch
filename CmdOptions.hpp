//============================================================================
// Name        : TvGuideFetch
// Author      : John Saunders
// Copyright   : Copyright (c) 2009 John Saunders
// Description : Fetch Australian XMLTV guide data from oztivo
//============================================================================
#ifndef CMDOPTIONS_HPP
#define CMDOPTIONS_HPP

#include <ostream>

class CmdParser
{
public:
    static CmdParser &instance();

    bool parse(int argc, char** argv);
    const char* getProgramName();
    const char* getParameter(int index);
    void writeCmdHelp(std::ostream &out);

    // A switch option simply sets a flag when the option is specified.
    typedef unsigned char flag;
    void addSwitchOption(char shortOpt, const char* longOpt,
            const char* description, flag* flagPtr);
    flag getSwitchValue(const std::string& longOpt);

    // An argument option stores the specified switch argument.
    typedef const char* arg;
    void addArgumentOption(char shortOpt, const char* longOpt,
            const char* description, arg* argPtr);
    arg getArgumentValue(const std::string& longOpt);

    // A method option calls a function when the option is specified.
    typedef void (method)(CmdParser &parser);
    void addMethodOption(char shortOpt, const char* longOpt,
            const char* description, method* methodPtr);

private:
    // Singleton pattern _must_ make these methods private.
    CmdParser();
    ~CmdParser();
    CmdParser(const CmdParser &);
    CmdParser &operator=(const CmdParser &);

    class Option
    {
    public:
        Option();
        Option* m_next;
        char m_shortOption;
        const char* m_longOption;
        const char* m_description;
        int m_flag;
        flag* m_flagPtr;
        arg* m_argPtr;
        method* m_methodPtr;
    };

    void addOption(Option* opt);
    const char* getShortOptions();
    struct option* getLongOptions();
    void processShortOptions(int arg);
    void processLongOptions(int arg);

    // Parsed command line words.
    int m_argc;
    char** m_argv;

    // List of options.
    Option* m_head;
    Option* m_tail;

    // Data structures built for getopt_long.
    int m_numOptions;
    char* m_shortOptions;
    struct option* m_longOptions;
};

#endif
