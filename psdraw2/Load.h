#ifndef LOAD_H
#define LOAD_H
#include <cstddef>
using namespace std;
class Entity;
bool Save(const char *filename=NULL);
bool Load(const char *filename=NULL);
void SetProc(Entity *proc);
void New();
#endif

