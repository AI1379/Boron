#include "gtest/gtest.h"
#include <gtest/gtest.h>

#include "Boron/ByteArray.hpp"
#include "Boron/Common.hpp"

TEST(ByteArrayView, DefaultConstructor) {
  Boron::ByteArrayView view;
  EXPECT_EQ(view.size(), 0);
  EXPECT_EQ(view.data(), nullptr);
}

TEST(ByteArrayView, ArrayConstructor) {
  constexpr const Boron::byte data[] = {0x01, 0x02, 0x03, 0x04};
  Boron::ByteArrayView view(data, sizeof(data));
  EXPECT_EQ(view.size(), 4);
  EXPECT_EQ(view[3], 0x04);
}

TEST(ByteArrayView, ArrayConstructorWithZeroTerminated) {
  constexpr const Boron::byte data[] = {0x01, 0x02, 0x03, 0x04, 0x00};
  Boron::ByteArrayView view(data);
  EXPECT_EQ(view.size(), 4);
  EXPECT_EQ(view[3], 0x04);
  const auto data2 = new Boron::byte[5];
  std::copy(data, data + 5, data2);
  Boron::ByteArrayView view2(data2);
  EXPECT_EQ(view2.size(), 4);
  EXPECT_EQ(view2[2], 0x03);
}

TEST(ByteArrayView, PointerConstructor) {
  const Boron::byte data[] = {0x01, 0x02, 0x03, 0x04};
  Boron::ByteArrayView view(data, data + 3);
  EXPECT_EQ(view.size(), 3);
  EXPECT_EQ(view[1], 0x02);
}

TEST(ByteArrayView, VectorConstructor) {
  std::vector<Boron::byte> data = {0x01, 0x02, 0x03, 0x04};
  Boron::ByteArrayView view(data);
  EXPECT_EQ(view.size(), 4);
  EXPECT_EQ(view[3], 0x04);
}

TEST(ByteArrayView, CopyConstructor) {
  constexpr const Boron::byte data[] = {0x01, 0x02, 0x03, 0x04};
  Boron::ByteArrayView view1(data, sizeof(data));
  Boron::ByteArrayView view2(view1);
  EXPECT_EQ(view2.size(), 4);
  EXPECT_EQ(view2[2], 0x03);
}

TEST(ByteArrayView, MoveConstructor) {
  constexpr const Boron::byte data[] = {0x01, 0x02, 0x03, 0x04};
  Boron::ByteArrayView view1(data, sizeof(data));
  Boron::ByteArrayView view2(std::move(view1));
  EXPECT_EQ(view2.size(), 4);
  EXPECT_EQ(view2[1], 0x02);
}

TEST(ByteArrayView, FromArray) {
  constexpr const Boron::byte data[] = {0x01, 0x02, 0x03, 0x04};
  auto view = Boron::ByteArrayView::fromArray(data);
  EXPECT_EQ(view.size(), 4);
  EXPECT_EQ(view[2], 0x03);
}

TEST(ByteArray, ToByteArray) {
  constexpr const Boron::byte data[] = {0x01, 0x02, 0x03, 0x04};
  Boron::ByteArrayView view(data, sizeof(data));
  auto array = view.toByteArray();
  EXPECT_EQ(array.size(), 4);
  EXPECT_EQ(array[1], 0x02);
}

TEST(ByteArray, Constructor) {
  auto ba = Boron::ByteArray(4, 0x01);
  EXPECT_EQ(ba.size(), 4);
  EXPECT_EQ(ba[2], 0x01);
  EXPECT_DEATH(ba.at(4), ".*");
}

TEST(ByteArray, ConstructorFromPointer) {
  constexpr const Boron::byte data[] = {0x01, 0x02, 0x03, 0x04};
  auto ba = Boron::ByteArray(data, sizeof(data));
  EXPECT_EQ(ba.size(), 4);
  EXPECT_EQ(ba[3], 0x04);
}

TEST(ByteArray, ConstructorFromPointerWithSize) {
  constexpr const Boron::byte data[] = {0x01, 0x02, 0x03, 0x04};
  auto ba = Boron::ByteArray(data, 3);
  EXPECT_EQ(ba.size(), 3);
  EXPECT_EQ(ba[2], 0x03);
}

TEST(ByteArray, CopyConstructor) {
  constexpr const Boron::byte data[] = {0x01, 0x02, 0x03, 0x04};
  auto ba1 = Boron::ByteArray(data, sizeof(data));
  auto ba2 = ba1;
  EXPECT_EQ(ba2.size(), 4);
  EXPECT_EQ(ba2[1], 0x02);
}

TEST(ByteArray, MoveConstructor) {
  constexpr const Boron::byte data[] = {0x01, 0x02, 0x03, 0x04};
  auto ba1 = Boron::ByteArray(data, sizeof(data));
  auto ba2 = std::move(ba1);
  EXPECT_EQ(ba2.size(), 4);
  EXPECT_EQ(ba2[3], 0x04);
  EXPECT_EQ(ba1.size(), 0);
  EXPECT_TRUE(ba1.isEmpty());
}

TEST(ByteArray, Swap) {
  constexpr const Boron::byte data1[] = {0x01, 0x02, 0x03, 0x04};
  constexpr const Boron::byte data2[] = {0x05, 0x06, 0x07, 0x08};
  auto ba1 = Boron::ByteArray(data1, sizeof(data1));
  auto ba2 = Boron::ByteArray(data2, sizeof(data2));
  ba1.swap(ba2);
  EXPECT_EQ(ba1.size(), 4);
  EXPECT_EQ(ba1[2], 0x07);
  EXPECT_EQ(ba2.size(), 4);
  EXPECT_EQ(ba2[1], 0x02);
}

TEST(ByteArray, Resize) {
  auto ba = Boron::ByteArray(4, 0x01);
  ba.resize(6);
  EXPECT_EQ(ba.size(), 6);
  EXPECT_EQ(ba[4], 0x00);
  ba.resize(3);
  EXPECT_EQ(ba.size(), 3);
  EXPECT_EQ(ba[2], 0x01);
  EXPECT_DEATH(ba.at(3), ".*");
  ba.resize(5, 0x02);
  EXPECT_EQ(ba.size(), 5);
  EXPECT_EQ(ba[4], 0x02);
}

TEST(ByteArray, Fill) {
  auto ba = Boron::ByteArray(4, 0x01);
  ba.fill(0x02, 6);
  EXPECT_EQ(ba.size(), 6);
  EXPECT_EQ(ba[4], 0x02);
  ba.fill(0x03, 3);
  EXPECT_EQ(ba.size(), 3);
  EXPECT_EQ(ba[2], 0x03);
}

TEST(ByteArray, Capacity) {
  auto ba = Boron::ByteArray(4, 0x01);
  EXPECT_EQ(ba.capacity(), 4);
  ba.resize(6);
  EXPECT_EQ(ba.capacity(), 6);
  ba.reserve(10);
  EXPECT_EQ(ba.capacity(), 10);
  ba.squeeze();
  EXPECT_EQ(ba.capacity(), 6);
}

TEST(ByteArray, IndexOf) {
  constexpr const Boron::byte data[] = {0x01, 0x02, 0x03, 0x04};
  auto ba = Boron::ByteArray(data, sizeof(data));
  EXPECT_EQ(ba.indexOf(0x03), 2);
  EXPECT_EQ(ba.indexOf(0x05), -1);
  auto needle = Boron::ByteArray(data, 2);
  EXPECT_EQ(ba.indexOf(needle), 0);
  auto needle2 = Boron::ByteArray(data + 1, 2);
  EXPECT_EQ(ba.indexOf(needle2), 1);
}

TEST(ByteArray, Contains) {
  constexpr const Boron::byte data[] = {0x01, 0x02, 0x03, 0x04};
  auto ba = Boron::ByteArray(data, sizeof(data));
  EXPECT_TRUE(ba.contains(0x03));
  EXPECT_FALSE(ba.contains(0x05));
  auto needle = Boron::ByteArray(data, 2);
  EXPECT_TRUE(ba.contains(needle));
  auto needle2 = Boron::ByteArray(data + 1, 2);
  EXPECT_TRUE(ba.contains(needle2));
}

TEST(ByteArray, Count) {
  constexpr const Boron::byte data[] = {0x01, 0x02, 0x03, 0x04, 0x02, 0x03};
  auto ba = Boron::ByteArray(data, sizeof(data));
  EXPECT_EQ(ba.count(0x03), 2);
  auto needle = Boron::ByteArray(data, 2);
  EXPECT_EQ(ba.count(needle), 1);
  auto needle2 = Boron::ByteArray(data + 1, 2);
  EXPECT_EQ(ba.count(needle2), 2);
}

TEST(ByteArray, Compare) {
  constexpr const Boron::byte data1[] = {0x01, 0x02, 0x03, 0x04};
  constexpr const Boron::byte data2[] = {0x01, 0x02, 0x03, 0x04};
  auto ba1 = Boron::ByteArray(data1, sizeof(data1));
  auto ba2 = Boron::ByteArray(data2, sizeof(data2));
  EXPECT_EQ(ba1, ba2);
  auto ba3 = Boron::ByteArray(data1, 3);
  EXPECT_NE(ba1, ba3);
  auto ba4 = Boron::ByteArray(data1, 4);
  EXPECT_EQ(ba1, ba4);
}
