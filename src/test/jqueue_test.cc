#include <assert.h>

#include <iostream>
#include <queue>

#include "test.h"
#include "shared/jqueue.h"

int main(int argc, char** argv) {
  stdj::Queue<int> jqueue;
  std::queue<int> squeue;

  jqueue.Add(10);
  squeue.push(10);
  ASSERT_EQ(jqueue.Size(), squeue.size());

  jqueue.Add(100);
  squeue.push(100);
  ASSERT_EQ(jqueue.Size(), squeue.size());

  ASSERT_EQ(jqueue.Remove(), squeue.front());
  squeue.pop();

  jqueue.Add(1000);
  squeue.push(1000);
  ASSERT_EQ(jqueue.Size(), squeue.size());

  ASSERT_EQ(jqueue.Remove(), squeue.front());
  squeue.pop();
  ASSERT_EQ(jqueue.Size(), squeue.size());

  ASSERT_EQ(jqueue.Remove(), squeue.front());
  squeue.pop();
  ASSERT_EQ(jqueue.Size(), squeue.size());

  return 0;
}
