#ifndef ENTITYVERIFIER_H
#define ENTITYVERIFIER_H
#include "Entity.h"

#ifdef _CHECK
void VerifyLinks(Entity *e, bool recursive = false);
void VerifyAll();
#endif

#endif

