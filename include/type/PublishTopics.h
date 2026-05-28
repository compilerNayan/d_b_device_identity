#ifndef PUBLISHTOPICS_H
#define PUBLISHTOPICS_H

#include <StandardDefines.h>

/* @Serializable */
class PublishTopics {

    Public optional<StdString> statusTopic;

    Public optional<StdString> telemetryTopic;

    Public optional<StdString> logsTopic;

    Public optional<StdString> eventsTopic;
};

#endif // PUBLISHTOPICS_H