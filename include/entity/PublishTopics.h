#ifndef PUBLISHTOPICS_H
#define PUBLISHTOPICS_H

#include <StandardDefines.h>

/* @Entity */
class PublishTopics {

    /* @Id */
    Public optional<int> id;

    Public optional<StdString> statusTopic;

    Public optional<StdString> telemetryTopic;

    Public optional<StdString> logsTopic;

    Public optional<StdString> eventsTopic;

};

#endif // PUBLISHTOPICS_H