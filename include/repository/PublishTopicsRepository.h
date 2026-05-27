#ifndef PUBLISHTOPICSREPOSITORY_H
#define PUBLISHTOPICSREPOSITORY_H

#include <StandardDefines.h>
#include "CpaRepository.h"
#include "../entity/PublishTopics.h"

/* @Repository */
DefineStandardPointers(PublishTopicsRepository)
class PublishTopicsRepository : public CpaRepository<PublishTopics, int> {
    Public Virtual ~PublishTopicsRepository() = default;
};

#endif // PUBLISHTOPICSREPOSITORY_H