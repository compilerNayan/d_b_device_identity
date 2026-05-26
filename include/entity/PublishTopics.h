#ifndef PUBLISHTOPICS_H
#define PUBLISHTOPICS_H

#include <StandardDefines.h>

/* @Entity */
class PublishTopics {

    /* @NotNull */
    Public optional<StdString> statusTopic;

    /* @NotNull */
    Public optional<StdString> telemetryTopic;

    /* @NotNull */
    Public optional<StdString> logsTopic;

    /* @NotNull */
    Public optional<StdString> eventsTopic;

};

#endif // PUBLISHTOPICS_H