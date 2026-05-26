#ifndef SUBSCRIBETOPICS_H
#define SUBSCRIBETOPICS_H

#include <StandardDefines.h>

/* @Serializable */
class SubscribeTopics {

    /* @NotNull */
    Public optional<StdString> commandTopic;

    /* @NotNull */
    Public optional<StdString> otaUpdateTopic;

    /* @NotNull */
    Public optional<StdString> featureFlagTopic;
};

#endif // SUBSCRIBETOPICS_H