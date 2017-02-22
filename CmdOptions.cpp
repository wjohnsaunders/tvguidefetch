//============================================================================
// Name        : TvGuideFetch
// Author      : John Saunders
// Copyright   : Copyright (c) 2009 John Saunders
// Description : Fetch Australian XMLTV guide data from oztivo
//============================================================================

#include <getopt.h>
#include <cstring>

#include "CmdOptions.hpp"

CmdParser::CmdParser()
:   m_argc(0),
    m_argv(0),
    m_head(0),
    m_tail(0),
    m_numOptions(0),
    m_shortOptions(0),
    m_longOptions(0)
{
}

CmdParser::~CmdParser()
{
    delete [] m_shortOptions;
    delete [] m_longOptions;
    while (m_head)
    {
        Option* tmp = m_head->m_next;
        delete m_head;
        m_head = tmp;
    }
}

CmdParser &CmdParser::instance()
{
    static CmdParser s_instance;
    return s_instance;
}

bool CmdParser::parse(int argc, char** argv)
{
    int arg;

    m_argc = argc;
    m_argv = argv;

    while ((arg = getopt_long(argc, argv, getShortOptions(), getLongOptions(), 0)) != -1)
    {
        if (arg == '?')
        {
            // getopt_long has already displayed an error message by the time
            // we get to this point. So just return a failure code.
            return false;
        }
        if (arg == 0)
        {
            // Process a long option.
            processLongOptions(arg);
        }
        else
        {
            // Process a short option.
            processShortOptions(arg);
        }
    }

    return true;
}

const char* CmdParser::getProgramName()
{
    if (m_argv)
    {
        return m_argv[0];
    }
    return "unset_program_name";
}

const char* CmdParser::getParameter(int index)
{
    if ((optind + index) < m_argc)
    {
        return m_argv[optind + index];
    }

    return 0;
}

void CmdParser::writeCmdHelp(std::ostream &out)
{
    Option* opt = m_head;
    while (opt)
    {
        out << std::endl;

        if (opt->m_longOption && opt->m_shortOption)
        {
            out << "{ -" << opt->m_shortOption << " | --" << opt->m_longOption << " }";
        }
        else if (opt->m_shortOption)
        {
            out << "-" << opt->m_shortOption;
        }
        else if (opt->m_longOption)
        {
            out << "--" << opt->m_longOption;
        }
        if (opt->m_argPtr)
        {
            const char* p1,* p2;
            if (((p1 = strchr(opt->m_description, '<')) != 0) &&
                ((p2 = strchr(p1, '>')) != 0))
            {
                out << " ";
                while (p1 <= p2)
                    out << *p1++;
            }
            else
            {
                out << " <ARG>";
            }
        }
        out << std::endl;
        const char* p1 = opt->m_description;
        while (*p1)
        {
            const char* p2 = p1,* p3 = 0;
            for (int i = 0; (i < 70) && *p2; ++i)
            {
                if (*p2 == ' ')
                    p3 = p2;
                ++p2;
            }
            if (*p2 && p3)
                p2 = p3;

            out << "        ";
            while (p1 < p2)
                out << *p1++;
            if (*p1 == ' ')
                ++p1;
            out << std::endl;
        }

        opt = opt->m_next;
    }
}

void CmdParser::addSwitchOption(char shortOpt, const char* longOpt,
        const char* description, flag* flagPtr)
{
    Option* opt = new Option();
    opt->m_shortOption = shortOpt;
    opt->m_longOption = longOpt;
    opt->m_description = description;
    opt->m_flagPtr = flagPtr;

    addOption(opt);
}

CmdParser::flag CmdParser::getSwitchValue(const std::string& longOpt)
{
    Option* opt = m_head;
    while (opt)
    {
        if (longOpt == opt->m_longOption)
        {
            if (opt->m_flagPtr)
            {
                return *(opt->m_flagPtr);
            }
            break;
        }

        opt = opt->m_next;
    }
    return 0;
}

void CmdParser::addArgumentOption(char shortOpt, const char* longOpt,
        const char* description, arg* argPtr)
{
    Option* opt = new Option();
    opt->m_shortOption = shortOpt;
    opt->m_longOption = longOpt;
    opt->m_description = description;
    opt->m_argPtr = argPtr;

    addOption(opt);
}

CmdParser::arg CmdParser::getArgumentValue(const std::string& longOpt)
{
    Option* opt = m_head;
    while (opt)
    {
        if (longOpt == opt->m_longOption)
        {
            if (opt->m_argPtr)
            {
                return *(opt->m_argPtr);
            }
            break;
        }

        opt = opt->m_next;
    }
    return 0;
}

void CmdParser::addMethodOption(char shortOpt, const char* longOpt,
        const char* description, method* methodPtr)
{
    Option* opt = new Option();
    opt->m_shortOption = shortOpt;
    opt->m_longOption = longOpt;
    opt->m_description = description;
    opt->m_methodPtr = methodPtr;

    addOption(opt);
}

void CmdParser::addOption(Option* opt)
{
    opt->m_next = 0;
    if (m_tail)
    {
        m_tail->m_next = opt;
        m_tail = opt;
    }
    else
    {
        m_head = m_tail = opt;
    }
    ++m_numOptions;
}

const char* CmdParser::getShortOptions()
{
    delete [] m_shortOptions;
    m_shortOptions = new char[m_numOptions * 2 + 1];

    char* optPtr = m_shortOptions;
    for (Option* opt = m_head; opt; opt = opt->m_next)
    {
        if (opt->m_shortOption != 0)
        {
            *optPtr++ = opt->m_shortOption;
            if (opt->m_argPtr)
            {
                *optPtr++ = ':';
            }
        }
    }
    *optPtr = 0;

    return m_shortOptions;
}

struct option* CmdParser::getLongOptions()
{
    delete [] m_longOptions;
    m_longOptions = new struct option[m_numOptions + 1];

    struct option* optPtr = m_longOptions;
    for (Option* opt = m_head; opt; opt = opt->m_next)
    {
        if (opt->m_longOption != 0)
        {
            optPtr->name = opt->m_longOption;
            optPtr->has_arg = opt->m_argPtr ? required_argument : no_argument;
            optPtr->flag = &opt->m_flag;
            optPtr->val = 1;
            ++optPtr;
        }
    }
    optPtr->name = 0;
    optPtr->has_arg = 0;
    optPtr->flag = 0;
    optPtr->val = 0;

    return m_longOptions;
}

void CmdParser::processShortOptions(int arg)
{
    Option* opt = m_head;
    while (opt)
    {
        if (opt->m_shortOption == arg)
        {
            if (opt->m_flagPtr)
            {
                *(opt->m_flagPtr) = *(opt->m_flagPtr) + 1;
            }
            if (opt->m_argPtr)
            {
                *(opt->m_argPtr) = optarg;
            }
            if (opt->m_methodPtr)
            {
                opt->m_methodPtr(*this);
            }
        }

        opt = opt->m_next;
    }
}

void CmdParser::processLongOptions(int arg)
{
    Option* opt = m_head;
    while (opt)
    {
        if (opt->m_flag == 1)
        {
            opt->m_flag = 0;
            if (opt->m_flagPtr)
            {
                *(opt->m_flagPtr) = *(opt->m_flagPtr) + 1;
            }
            if (opt->m_argPtr)
            {
                *(opt->m_argPtr) = optarg;
            }
            if (opt->m_methodPtr)
            {
                opt->m_methodPtr(*this);
            }
        }

        opt = opt->m_next;
    }
}

CmdParser::Option::Option()
:   m_next(0),
    m_shortOption(0),
    m_longOption(0),
    m_description(0),
    m_flag(0),
    m_flagPtr(0),
    m_argPtr(0),
    m_methodPtr(0)
{
}
