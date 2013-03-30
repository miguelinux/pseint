#ifndef COMM_H
#define COMM_H

bool Connect(int port, int id);

bool SendUpdate(bool run=false, bool exp=false);

bool SendHelp();

void CloseComm();

void ReadComm();

#endif

