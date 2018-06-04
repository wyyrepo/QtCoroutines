#include "qtcoiterator.h"

thread_local QStack<void*> QtCoroutine::IteratorContainer::_consumers;
