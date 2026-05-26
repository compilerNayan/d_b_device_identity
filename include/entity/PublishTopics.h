#ifndef PUBLISHTOPICS_H
#define PUBLISHTOPICS_H

#include <StandardDefines.h>

/* @Entity */
class PublishTopics {

    /* @NotNull */
    optional<StdString> statusTopic;

    /* @NotNull */
    optional<StdString> telemetryTopic;

    /* @NotNull */
    optional<StdString> logsTopic;

    /* @NotNull */
    optional<StdString> eventsTopic;

};

#endif // PUBLISHTOPICS_H