/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef TRACELIB_H
#define TRACELIB_H

/*! \mainpage froglogic tracelib
 *
 * \section intro_sec Introduction
 *
 * tracelib is a library for tracing the execution of a C or C++ program and
 * inspecting its state. This is achieved by instrumenting the source code of
 * the target program and linking the recompiled sources against a shared
 * tracelib library.
 *
 * \section components_sec Components
 *
 * The tracelib distribution consists of multiple components (the file names are
 * given for a Windows build, they are named similarly on Unix):
 *
 * \li \c tracelib.dll is a dynamic library (plus accompanying header files)
 * which has to be linked into any application which wishes to use the tracing
 * functionality.
 *
 * \li \c tracelib_qtsupport.dll is a dynamic library which provides additional
 * support for Qt objects. Applications using the
 * <a href="http://qt.nokia.com">Qt GUI framework</a> may which to link against
 * this library in addition to \c tracelib.lib.
 *
 * \li \c tracegui.exe is a GUI for reviewing previously recorded traces as
 * well as recording and watching the trace generated by running applications
 * live.
 *
 * \li \c traced.exe is a daemon process which collects and stores trace data
 * in the background without running a GUI.Traced applications can be
 * configured to send their output over a network connection to \c traced.exe.
 * The recorded traces can be sent to other people and reviewed later
 * using \c tracegui.exe.
 *
 * \li \c trace2xml.exe is a utility program for dumping a trace database
 * generated by \c tracegui.exe or \c traced.exe into an XML file which can then
 * be processed by other scripts.
 *
 * \li \c convertdb.exe is a helper utility for converting earlier versions of
 * databases with tracelib traces.
 *
 * \section quickstart_sec Quick Start
 *
 * Here's a quick step by step guide on how to instrument a basic program
 * so that it generates trace information. Here's the initial source code
 * of the sample program:
 *
 * \include hello.cpp
 *
 * First, instrument the above source code so that the \c tracelib.h
 * header is included and insert a few calls to various tracelib macros into
 * the source code. Here's the instrumented code:
 *
 * \include hello_instrumented.cpp
 *
 * Save the resulting file to e.g. \c hello_instrumented.cpp.
 *
 * Then, recompile the program and link against \c tracelib.dll as well as
 * \c ws2_32.lib. You might also want to link against \c tracelib_qtsupport.dll
 * in case Qt objects are being printed using the #TRACELIB_VAR macro.
 *
 * Here's a sample compile line for use with \c cl.exe (the compiler
 * which comes with Microsoft Visual Studio):
 *
 * \verbatim
cl /EHsc hello_instrumented.cpp /I <TRACELIB_PREFIX>\include\tracelib
    <TRACELIB_PREFIX>\lib\tracelib.lib
    ws2_32.lib
\endverbatim
 *
 * The resulting binary (\c hello_instrumented.exe in the above case) will
 * behave as before. The tracing (and the resulting change to runtime
 * performance) is only activated in case a special configuration file was
 * detected.
 *
 * Creating a configuration file is a matter of writing some XML. Save
 * the following file as \c tracelib.xml and store it in the same directory
 * as \c hello_instrumented.exe which was built above:
 *
 * \verbinclude tracelib.xml
 *
 * Now, running the program will generate quite a bit of additional output:
\verbatim
03.09.2010 16:00:56: Process 2524 [started at 03.09.2010 16:00:56] (Thread 468): [LOG] 'main() entered' hello_instrumented.cpp:8: int __cdecl main(void)
Please enter your name: Max
03.09.2010 16:00:57: Process 2524 [started at 03.09.2010 16:00:56] (Thread 468): [WATCH] hello_instrumented.cpp:16: int __cdecl main(void)
Hello, Max!
03.09.2010 16:00:57: Process 2524 [started at 03.09.2010 16:00:56] (Thread 468): [LOG] 'main() finished' hello_instrumented.cpp:20: int __cdecl main(void)
03.09.2010 16:00:57: Process 2524 [started at 03.09.2010 16:00:56] finished
\endverbatim
 * This concludes the tracelib quick start.
 */

#include "dlldefs.h"
#include "tracelib_config.h"
#include "tracepoint.h"
#include "variabledumping.h"

#include <sstream>
#include <string>

#ifdef _MSC_VER
#  define TRACELIB_CURRENT_FILE_NAME __FILE__
#  define TRACELIB_CURRENT_LINE_NUMBER __LINE__
#  define TRACELIB_CURRENT_FUNCTION_NAME __FUNCSIG__
#elif __GNUC__
#  define TRACELIB_CURRENT_FILE_NAME __FILE__
#  define TRACELIB_CURRENT_LINE_NUMBER __LINE__
#  define TRACELIB_CURRENT_FUNCTION_NAME __PRETTY_FUNCTION__
#else
#  error "Unsupported compiler!"
#endif

#ifndef TRACELIB_DISABLE_TRACE_CODE
#  define TRACELIB_VARIABLE_SNAPSHOT_KEY_MSG(verbosity, key, vars, msg) \
{ \
    static TRACELIB_NAMESPACE_IDENT(TracePoint) tracePoint(TRACELIB_NAMESPACE_IDENT(TracePointType)::Watch, (verbosity), TRACELIB_CURRENT_FILE_NAME, TRACELIB_CURRENT_LINE_NUMBER, TRACELIB_CURRENT_FUNCTION_NAME, key); \
    TRACELIB_NAMESPACE_IDENT(VariableSnapshot) *variableSnapshot = new TRACELIB_NAMESPACE_IDENT(VariableSnapshot); \
    (*variableSnapshot) << vars; \
    TRACELIB_NAMESPACE_IDENT(visitTracePoint)( &tracePoint, (msg), variableSnapshot ); \
}

#  define TRACELIB_VISIT_TRACEPOINT_KEY_MSG(type, verbosity, key, msg) \
{ \
    static TRACELIB_NAMESPACE_IDENT(TracePoint) tracePoint(type, (verbosity), TRACELIB_CURRENT_FILE_NAME, TRACELIB_CURRENT_LINE_NUMBER, TRACELIB_CURRENT_FUNCTION_NAME, key); \
    TRACELIB_NAMESPACE_IDENT(visitTracePoint)( &tracePoint, msg ); \
}
#  define TRACELIB_VAR(v) TRACELIB_NAMESPACE_IDENT(makeConverter)(#v, v)
#else
#  define TRACELIB_VARIABLE_SNAPSHOT_KEY_MSG(verbosity, key, vars, msg) (void)0;
#  define TRACELIB_VISIT_TRACEPOINT_KEY_MSG(type, verbosity, key, msg) (void)0;
#  define TRACELIB_VAR(v) (void)0;
#endif

#define TRACELIB_DEBUG_IMPL                   TRACELIB_VISIT_TRACEPOINT_KEY_MSG(TRACELIB_NAMESPACE_IDENT(TracePointType)::Debug, 1, 0, 0)
#define TRACELIB_DEBUG_MSG_IMPL(msg)          TRACELIB_VISIT_TRACEPOINT_KEY_MSG(TRACELIB_NAMESPACE_IDENT(TracePointType)::Debug, 1, 0, TRACELIB_NAMESPACE_IDENT(StringBuilder)() << msg)
#define TRACELIB_DEBUG_KEY_IMPL(key)          TRACELIB_VISIT_TRACEPOINT_KEY_MSG(TRACELIB_NAMESPACE_IDENT(TracePointType)::Debug, 1, key, 0)
#define TRACELIB_DEBUG_KEY_MSG_IMPL(key, msg) TRACELIB_VISIT_TRACEPOINT_KEY_MSG(TRACELIB_NAMESPACE_IDENT(TracePointType)::Debug, 1, key, TRACELIB_NAMESPACE_IDENT(StringBuilder)() << msg)

#define TRACELIB_ERROR_IMPL                   TRACELIB_VISIT_TRACEPOINT_KEY_MSG(TRACELIB_NAMESPACE_IDENT(TracePointType)::Error, 1, 0, 0)
#define TRACELIB_ERROR_MSG_IMPL(msg)          TRACELIB_VISIT_TRACEPOINT_KEY_MSG(TRACELIB_NAMESPACE_IDENT(TracePointType)::Error, 1, 0, TRACELIB_NAMESPACE_IDENT(StringBuilder)() << msg)
#define TRACELIB_ERROR_KEY_IMPL(key)          TRACELIB_VISIT_TRACEPOINT_KEY_MSG(TRACELIB_NAMESPACE_IDENT(TracePointType)::Error, 1, key, 0)
#define TRACELIB_ERROR_KEY_MSG_IMPL(key, msg) TRACELIB_VISIT_TRACEPOINT_KEY_MSG(TRACELIB_NAMESPACE_IDENT(TracePointType)::Error, 1, key, TRACELIB_NAMESPACE_IDENT(StringBuilder)() << msg)

#define TRACELIB_TRACE_IMPL                   TRACELIB_VISIT_TRACEPOINT_KEY_MSG(TRACELIB_NAMESPACE_IDENT(TracePointType)::Log, 1, 0, 0)
#define TRACELIB_TRACE_MSG_IMPL(msg)          TRACELIB_VISIT_TRACEPOINT_KEY_MSG(TRACELIB_NAMESPACE_IDENT(TracePointType)::Log, 1, 0, TRACELIB_NAMESPACE_IDENT(StringBuilder)() << msg)
#define TRACELIB_TRACE_KEY_IMPL(key)          TRACELIB_VISIT_TRACEPOINT_KEY_MSG(TRACELIB_NAMESPACE_IDENT(TracePointType)::Log, 1, key, 0)
#define TRACELIB_TRACE_KEY_MSG_IMPL(key, msg) TRACELIB_VISIT_TRACEPOINT_KEY_MSG(TRACELIB_NAMESPACE_IDENT(TracePointType)::Log, 1, key, TRACELIB_NAMESPACE_IDENT(StringBuilder)() << msg)

#define TRACELIB_WATCH_IMPL(vars)                   TRACELIB_VARIABLE_SNAPSHOT_KEY_MSG(1, 0, vars, 0)
#define TRACELIB_WATCH_MSG_IMPL(msg, vars)          TRACELIB_VARIABLE_SNAPSHOT_KEY_MSG(1, 0, vars, TRACELIB_NAMESPACE_IDENT(StringBuilder)() << msg)
#define TRACELIB_WATCH_KEY_IMPL(key, vars)          TRACELIB_VARIABLE_SNAPSHOT_KEY_MSG(1, key, vars, 0)
#define TRACELIB_WATCH_KEY_MSG_IMPL(key, msg, vars) TRACELIB_VARIABLE_SNAPSHOT_KEY_MSG(1, key, vars, TRACELIB_NAMESPACE_IDENT(StringBuilder)() << msg)

#define TRACELIB_VALUE_IMPL(v) #v << "=" << v

TRACELIB_NAMESPACE_BEGIN

class StringBuilder
{
public:
    StringBuilder() { }

    inline operator const char * const() {
        m_s = m_stream.str();
        return m_s.c_str();
    }

    template <class T>
    StringBuilder &operator<<( const T &v ) {
        m_stream << v;
        return *this;
    }

private:
    StringBuilder( const StringBuilder &other );
    void operator=( const StringBuilder &rhs );

    std::string m_s;
    std::ostringstream m_stream;
};

template <>
inline StringBuilder &StringBuilder::operator<<( const VariableValue &v ) {
    m_stream << VariableValue::convertToString( v );
    return *this;
}

TRACELIB_EXPORT void visitTracePoint( TracePoint *tracePoint,
                      const char *msg = 0,
                      VariableSnapshot *variables = 0 );

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_H)
