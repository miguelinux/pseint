#ifndef LOAD_H
#define LOAD_H
#include <cstddef>
#include <string>
using namespace std;
class Entity;
bool Save(const char *filename=nullptr);
bool Load(const char *filename=nullptr);
void SetProc(Entity *proc);
void CreateEmptyProc(string type);
void New();
#endif

