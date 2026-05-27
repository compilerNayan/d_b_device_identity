#ifndef SUBSCRIBETOPICS_H
#define SUBSCRIBETOPICS_H

#include <StandardDefines.h>

/* @Entity */
class SubscribeTopics {

    /* @Id */
    Public optional<int> id;

    Public optional<StdString> commandTopic;

    Public optional<StdString> otaUpdateTopic;

    Public optional<StdString> featureFlagTopic;
};

#endif // SUBSCRIBETOPICS_H