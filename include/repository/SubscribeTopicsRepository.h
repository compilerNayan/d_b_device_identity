#ifndef SUBSCRIBETOPICSREPOSITORY_H
#define SUBSCRIBETOPICSREPOSITORY_H

#include <StandardDefines.h>
#include "CpaRepository.h"
#include "../entity/SubscribeTopics.h"

/* @Repository */
DefineStandardPointers(SubscribeTopicsRepository)
class SubscribeTopicsRepository : public CpaRepository<SubscribeTopics, int> {
    Public Virtual ~SubscribeTopicsRepository() = default;
};

#endif // SUBSCRIBETOPICSREPOSITORY_H