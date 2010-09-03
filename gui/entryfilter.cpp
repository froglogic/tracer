/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "entryfilter.h"

#include "../server/server.h"

typedef QMap<QString, QVariant> StorageMap;

bool EntryFilter::matches(const TraceEntry &e) const
{
    // Check is analog to LIKE %..% clause in model using a SQL query
    if (!m_application.isEmpty() && !e.processName.contains(m_application))
        return false;
    return true;
}

QVariant EntryFilter::sessionState() const
{
    StorageMap map;
    if (!m_application.isEmpty())
        map["Application"] = m_application;
    if (!m_processId.isEmpty())
        map["ProcessId"] = m_processId;
    if (!m_threadId.isEmpty())
        map["ThreadId"] = m_threadId;
    if (!m_function.isEmpty())
        map["Function"] = m_function;
    if (!m_message.isEmpty())
        map["Message"] = m_message;
    if (m_type != -1)
        map["Type"] = m_type;

    return QVariant(map);
}

bool EntryFilter::restoreSessionState(const QVariant &state)
{
    StorageMap map = state.value<StorageMap>();
    m_application = map["Application"].toString();
    m_processId = map["ProcessId"].toString();
    m_threadId = map["ThreadId"].toString();
    m_function = map["Function"].toString();
    m_message = map["Message"].toString();
    m_type = map["Type"].toInt();

    emit changed();

    return true;
}
