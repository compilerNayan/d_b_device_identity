#ifndef SUBSCRIBETOPICS_H
#define SUBSCRIBETOPICS_H

#include <StandardDefines.h>

/* @Entity */
class SubscribeTopics {

    /* @NotNull */
    optional<StdString> commandTopic;

    /* @NotNull */
    optional<StdString> otaUpdateTopic;

    /* @NotNull */
    optional<StdString> featureFlagTopic;
};

#endif // SUBSCRIBETOPICS_H