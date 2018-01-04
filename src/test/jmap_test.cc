#include "test.h"

#include "shared/jmap.h"

int main(int argc, char** argv) {
  stdj::Map<int, int, -1> map;

  map.Set(1, 10);
  ASSERT_EQ(map.Get(1), 10);
  ASSERT_EQ(map.Get(2), -1);
  ASSERT_EQ(map.Get(3), -1);

  map.Set(2, 20);
  ASSERT_EQ(map.Get(1), 10);
  ASSERT_EQ(map.Get(2), 20);
  ASSERT_EQ(map.Get(3), -1);

  map.Set(1, 100);
  ASSERT_EQ(map.Get(1), 100);
  ASSERT_EQ(map.Get(2), 20);
  ASSERT_EQ(map.Get(3), -1);

  map.Remove(1);
  ASSERT_EQ(map.Get(1), -1);
  ASSERT_EQ(map.Get(2), 20);
  ASSERT_EQ(map.Get(3), -1);

  map.Set(1, 10);
  map.Set(3, 30);
  ASSERT_EQ(map.Get(1), 10);
  ASSERT_EQ(map.Get(2), 20);
  ASSERT_EQ(map.Get(3), 30);

  map.Remove(3);
  ASSERT_EQ(map.Get(1), 10);
  ASSERT_EQ(map.Get(2), 20);
  ASSERT_EQ(map.Get(3), -1);

  map.Remove(1);
  map.Remove(2);
  ASSERT_EQ(map.Get(1), -1);
  ASSERT_EQ(map.Get(2), -1);
  ASSERT_EQ(map.Get(3), -1);

  return 0;
}
