/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef TRACELIB_H
#define TRACELIB_H

#include "tracelib_config.h"

#include "backtrace.h"
#include "crashhandler.h"
#include "filemodificationmonitor.h"
#include "filter.h"
#include "getcurrentthreadid.h"
#include "mutex.h"
#include "output.h"
#include "serializer.h"
#include "shutdownnotifier.h"
#include "variabledumping.h"

#include <vector>

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

#ifndef NDEBUG
#  define TRACELIB_VARIABLE_SNAPSHOT_MSG(verbosity, vars, msg) \
{ \
    static TRACELIB_NAMESPACE_IDENT(TracePoint) tracePoint(TRACELIB_NAMESPACE_IDENT(TracePointType)::Watch, (verbosity), TRACELIB_CURRENT_FILE_NAME, TRACELIB_CURRENT_LINE_NUMBER, TRACELIB_CURRENT_FUNCTION_NAME); \
    TRACELIB_NAMESPACE_IDENT(VariableSnapshot) *variableSnapshot = new TRACELIB_NAMESPACE_IDENT(VariableSnapshot); \
    (*variableSnapshot) << vars; \
    TRACELIB_NAMESPACE_IDENT(getActiveTrace)()->visitTracePoint( &tracePoint, (msg), variableSnapshot ); \
}

#  define TRACELIB_VISIT_TRACEPOINT_MSG(type, verbosity, msg) \
{ \
    static TRACELIB_NAMESPACE_IDENT(TracePoint) tracePoint(type, (verbosity), TRACELIB_CURRENT_FILE_NAME, TRACELIB_CURRENT_LINE_NUMBER, TRACELIB_CURRENT_FUNCTION_NAME); \
    TRACELIB_NAMESPACE_IDENT(getActiveTrace)()->visitTracePoint( &tracePoint, msg ); \
}
#  define TRACELIB_VAR(v) TRACELIB_NAMESPACE_IDENT(makeConverter)(#v, v)
#else
#  define TRACELIB_VARIABLE_SNAPSHOT_MSG(verbosity, vars, msg) (void)0;
#  define TRACELIB_VISIT_TRACEPOINT_MSG(type, verbosity, msg) (void)0;
#  define TRACELIB_VAR(v) (void)0;
#endif

TRACELIB_NAMESPACE_BEGIN

class Configuration;

struct TracePointType {
    enum Value {
        None = 0
#define TRACELIB_TRACEPOINTTYPE(name) ,name
#include "tracepointtypes.def"
#undef TRACELIB_TRACEPOINTTYPE
    };

    static const int *values() {
        static const int a[] = {
            None,
#define TRACELIB_TRACEPOINTTYPE(name) name,
#include "tracepointtypes.def"
#undef TRACELIB_TRACEPOINTTYPE
            -1
        };
        return a;
    }

    static const char *valueAsString( Value v ) {
#define TRACELIB_TRACEPOINTTYPE(name) static const char str_##name[] = #name;
#include "tracepointtypes.def"
#undef TRACELIB_TRACEPOINTTYPE
        switch ( v ) {
            case None: return "None";
#define TRACELIB_TRACEPOINTTYPE(name) case name: return str_##name;
#include "tracepointtypes.def"
#undef TRACELIB_TRACEPOINTTYPE
        }
        return 0;
    }
};

struct TracePoint {
    TracePoint( TracePointType::Value type_, unsigned short verbosity_, const char *sourceFile_, unsigned int lineno_, const char *functionName_ )
        : type( type_ ),
        verbosity( verbosity_ ),
        sourceFile( sourceFile_ ),
        lineno( lineno_ ),
        functionName( functionName_ ),
        lastUsedConfiguration( 0 ),
        active( false ),
        backtracesEnabled( false ),
        variableSnapshotEnabled( false )
    {
    }

    const TracePointType::Value type;
    const unsigned short verbosity;
    const char * const sourceFile;
    const unsigned int lineno;
    const char * const functionName;
    const Configuration *lastUsedConfiguration;
    bool active;
    bool backtracesEnabled;
    bool variableSnapshotEnabled;
};

class TracePointSet
{
public:
    static const unsigned int IgnoreTracePoint = 0x0000;
    static const unsigned int LogTracePoint = 0x0001;
    static const unsigned int YieldBacktrace = LogTracePoint | 0x0100;
    static const unsigned int YieldVariables = LogTracePoint | 0x0200;

    TracePointSet( Filter *filter, unsigned int actions );
    ~TracePointSet();

    unsigned int actionForTracePoint( const TracePoint *tracePoint );

private:
    TracePointSet( const TracePointSet &other );
    void operator=( const TracePointSet &rhs );

    Filter *m_filter;
    const unsigned int m_actions;
};

struct TracedProcess
{
    ProcessId id;
    time_t startTime;
};

struct TraceEntry
{
    TraceEntry( const TracePoint *tracePoint_, const char *msg = 0 );
    ~TraceEntry();

    static const TracedProcess process;
    const ThreadId threadId;
    const time_t timeStamp;
    const TracePoint *tracePoint;
    VariableSnapshot *variables;
    Backtrace *backtrace;
    const char * const message;
};

struct ProcessShutdownEvent
{
    ProcessShutdownEvent();

    const TracedProcess * const process;
    const time_t shutdownTime;
};


class Trace : public FileModificationMonitorObserver, public ShutdownNotifierObserver
{
public:
    Trace();
    ~Trace();

    void configureTracePoint( TracePoint *tracePoint ) const;
    void visitTracePoint( TracePoint *tracePoint,
                          const char *msg = 0,
                          VariableSnapshot *variables = 0 );

    void addEntry( const TraceEntry &e );

    void setSerializer( Serializer *serializer );
    void setOutput( Output *output );

    virtual void handleFileModification( const std::string &fileName, NotificationReason reason );

    virtual void handleProcessShutdown();

private:
    Trace( const Trace &trace );
    void operator=( const Trace &trace );

    void reloadConfiguration( const std::string &fileName );

    Serializer *m_serializer;
    Mutex m_serializerMutex;
    Output *m_output;
    Mutex m_outputMutex;
    std::vector<TracePointSet *> m_tracePointSets;
    Configuration *m_configuration;
    mutable Mutex m_configurationMutex;
    BacktraceGenerator m_backtraceGenerator;
    FileModificationMonitor *m_configFileMonitor;
};

Trace *getActiveTrace();
void setActiveTrace( Trace *trace );

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_H)
