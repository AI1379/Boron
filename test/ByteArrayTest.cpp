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
