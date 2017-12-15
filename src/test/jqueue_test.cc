#include <assert.h>

#include <iostream>
#include <queue>

#include "shared/jqueue.h"

int main(int argc, char** argv) {
  stdj::Queue<int> jqueue;
  std::queue<int> squeue;

  jqueue.Add(10);
  squeue.push(10);
  assert(jqueue.Size() == squeue.size());

  jqueue.Add(100);
  squeue.push(100);
  assert(jqueue.Size() == squeue.size());

  assert(jqueue.Remove() == squeue.front());
  squeue.pop();

  jqueue.Add(1000);
  squeue.push(1000);
  assert(jqueue.Size() == squeue.size());

  assert(jqueue.Remove() == squeue.front());
  squeue.pop();

  assert(jqueue.Remove() == squeue.front());
  squeue.pop();

  return 0;
}
