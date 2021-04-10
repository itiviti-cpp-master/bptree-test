#include "bptree.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <iomanip>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::mt19937_64 rgen(std::random_device{}());

struct BigOne
{
    std::array<unsigned char, 2032> data;

    BigOne()
    {
        data.fill(0);
    }

    BigOne(const int x)
    {
        data.fill(0);
        std::ostringstream ss;
        ss << std::setw(data.size() / 2) << std::setfill('0') << x;
        auto str = ss.str();
        std::copy(str.begin(), str.end(), data.begin());
    }

    operator int () const
    {
        return std::stoi(std::string(data.begin(), data.end()));
    }

    friend std::ostream & operator << (std::ostream & strm, const BigOne & x)
    {
        return strm << static_cast<int>(x);
    }
};

template <class T>
struct IntCompat
{
    static T create(const int x)
    { return x; }
    static int value(const T & x)
    { return x; }
};

template <>
struct IntCompat<std::string>
{
    static constexpr std::size_t width = 10;
    static std::string create(const int x)
    {
        std::ostringstream ss;
        ss << std::setw(width) << std::setfill('0') << x;
        return ss.str();
    }
    static int value(const std::string & x)
    { return std::stoi(x); }
};

template <class Key, class Value>
struct Type
{
    using key_type = Key;
    using value_type = Value;

    static std::pair<Key, Value> create(const int x)
    { return {create_key(x), create_value(x)}; }
    static Key create_key(const int x)
    { return IntCompat<Key>::create(x); }
    static Value create_value(const int x)
    { return IntCompat<Value>::create(x); }

    static int key(const Key & x)
    { return IntCompat<Key>::value(x); }
    static int key(const std::pair<const Key, Value> & x)
    { return IntCompat<Key>::value(x.first); }
    static int value(const std::pair<const Key, Value> & x)
    { return IntCompat<Value>::value(x.second); }
    static int value(const Value & x)
    { return IntCompat<Value>::value(x); }
};

template <class T>
struct BPTreeTest : ::testing::Test
{
    using Tree = BPTree<typename T::key_type, typename T::value_type>;
    Tree tree;

    const Tree & const_tree()
    {
        return tree;
    }

    void insert(const std::pair<Key, Value> & el)
    {
        tree.insert(el.first, el.second);
    }
};

using TestedTypes = ::testing::Types<Type<int, std::string>, Type<std::string, int>, Type<std::string, std::string>, Type<int, BigOne>>;
TYPED_TEST_SUITE(BPTreeTest, TestedTypes);

} // anonymous namespace

TYPED_TEST(BPTreeTest, count)
{
    this->insert(TypeParam::create(7));
    EXPECT_EQ(0, this->const_tree().count(6));
    EXPECT_EQ(1, this->tree.count(7));
}

TYPED_TEST(BPTreeTest, contains)
{
    this->insert(TypeParam::create(11));
    EXPECT_FALSE(this->tree.contains(12));
    EXPECT_TRUE(this->const_tree().contains(11));
}

TYPED_TEST(BPTreeTest, equal_range)
{
    {
        const auto [from, to] = this->tree.equal_range(TypeParam::create_key(3));
        EXPECT_TRUE(from == to);
    }
    this->insert(TypeParam::create(5));
    {
        auto [from, to] = this->tree.equal_range(TypeParam::create_key(5));
        EXPECT_FALSE(from == to);
        EXPECT_EQ(5, TypeParam::key(*from));
        from->second = TypeParam::create_value(11);
        ++from;
        EXPECT_TRUE(from == to);
    }
    this->insert(TypeParam::create(6));
    this->insert(TypeParam::create(4));
    {
        auto [from, to] = this->const_tree().equal_range(TypeParam::create_key(5));
        EXPECT_FALSE(from == to);
        EXPECT_EQ(5, TypeParam::key(*from));
        EXPECT_TRUE(std::is_const_v<decltype(from->second)>);
        ++from;
        EXPECT_TRUE(from == to);
    }
}

TYPED_TEST(BPTreeTest, at)
{
    // TODO
}

TYPED_TEST(BPTreeTest, index)
{
    // TODO
}

TYPED_TEST(BPTreeTest, insert)
{
    // TODO
}

TYPED_TEST(BPTreeTest, erase_by_iterator)
{
    // TODO
}

TYPED_TEST(BPTreeTest, erase_range)
{
    // TODO
}

TYPED_TEST(BPTreeTest, erase_key)
{
    // TODO
}

TYPED_TEST(BPTreeTest, empty)
{
    EXPECT_TRUE(this->tree.empty());
    EXPECT_EQ(0, this->tree.size());
    EXPECT_EQ(this->tree.end(), this->tree.find(TypeParam::create_key(0)));
    EXPECT_EQ(this->tree.end(), this->tree.find(TypeParam::create_key(13)));
    EXPECT_EQ(this->tree.end(), this->tree.find(TypeParam::create_key(101)));
    EXPECT_EQ(this->tree.end(), this->tree.lower_bound(TypeParam::create_key(53)));
    EXPECT_EQ(this->tree.end(), this->tree.upper_bound(TypeParam::create_key(67)));
    std::size_t count = 0;
    for ([[maybe_unused]] const auto & x : this->tree) {
        ++count;
    }
    EXPECT_EQ(0, count);
}

TYPED_TEST(BPTreeTest, singleton)
{
    this->insert(TypeParam::create(17));
    EXPECT_FALSE(this->tree.empty());
    EXPECT_EQ(1, this->tree.size());
    EXPECT_NE(this->tree.end(), this->tree.find(TypeParam::create_key(17)));
    EXPECT_EQ(17, TypeParam::key(*this->tree.find(TypeParam::create_key(17))));
    EXPECT_EQ(1, this->tree.count(TypeParam::create_key(17)));
    EXPECT_EQ(this->tree.find(TypeParam::create_key(17)), this->tree.lower_bound(TypeParam::create_key(17)));
    EXPECT_EQ(this->tree.end(), this->tree.upper_bound(TypeParam::create_key(17)));
    EXPECT_EQ(this->tree.end(), this->tree.find(TypeParam::create_key(7)));
    EXPECT_EQ(this->tree.end(), this->tree.lower_bound(TypeParam::create_key(19)));
    EXPECT_EQ(this->tree.end(), this->tree.upper_bound(TypeParam::create_key(18)));
    std::size_t count = 0;
    for (const auto & x : this->tree) {
        EXPECT_EQ(17, TypeParam::key(x));
        EXPECT_EQ(17, TypeParam::value(x));
        ++count;
    }
    EXPECT_EQ(1, count);
}

TYPED_TEST(BPTreeTest, several)
{
    const int max = 31;
    for (int i = 0; i < max; ++i) {
        this->insert(TypeParam::create(i));
    }
    EXPECT_FALSE(this->tree.empty());
    EXPECT_EQ(max, this->tree.size());
    for (const auto & [key, value] : this->tree) {
        EXPECT_EQ(TypeParam::key(key), TypeParam::value(value));
    }
    for (int i = 0; i < max; ++i) {
        EXPECT_NE(this->tree.end(), this->tree.find(TypeParam::create_key(i))) << "Not found key " << i;
        EXPECT_EQ(1, this->tree.count(TypeParam::create_key(i))) << "Invalid number of entries for key " << i;
        auto it = this->tree.find(TypeParam::create_key(i));
        EXPECT_EQ(i, TypeParam::key(*it)) << "Not found correct key value for " << i;
        EXPECT_EQ(it, this->tree.lower_bound(TypeParam::create_key(i))) << "Not found correct lower bound for key " << i;
        ++it;
        EXPECT_EQ(it, this->tree.upper_bound(TypeParam::create_key(i))) << "Not found correct upper bound for key " << i;
        EXPECT_EQ(i, TypeParam::value(this->tree.at(TypeParam::create_key(i)))) << "Invalid value at " << i;
        EXPECT_EQ(i, TypeParam::value(this->tree[TypeParam::create_key(i)])) << "Invalid value at " << i;
    }
    for (int i = -max; i < 0; ++i) {
        EXPECT_EQ(this->tree.end(), this->tree.find(TypeParam::create_key(i))) << "Found non-existing key " << i;
        EXPECT_EQ(0, this->tree.count(TypeParam::create_key(i))) << "Invalid number of entries for key " << i;
        EXPECT_EQ(this->tree.begin(), this->tree.lower_bound(TypeParam::create_key(i))) << "Not found correct lower bound for key " << i;
        EXPECT_EQ(this->tree.begin(), this->tree.upper_bound(TypeParam::create_key(i))) << "Not found correct upper bound for key " << i;
    }
    for (int i = max; i < 2 * max; ++i) {
        EXPECT_EQ(this->tree.end(), this->tree.find(TypeParam::create_key(i))) << "Found non-existing key " << i;
        EXPECT_EQ(0, this->tree.count(TypeParam::create_key(i))) << "Invalid number of entries for key " << i;
        EXPECT_EQ(this->tree.end(), this->tree.lower_bound(TypeParam::create_key(i))) << "Not found correct lower bound for key " << i;
        EXPECT_EQ(this->tree.end(), this->tree.upper_bound(TypeParam::create_key(i))) << "Not found correct upper bound for key " << i;
    }
}

TYPED_TEST(BPTreeTest, mutating_range_iteration)
{
    const int max = 9;
    for (int i = 0; i < max; ++i) {
        this->insert(TypeParam::create(i));
    }
    for (auto & [key, value] : this->tree) {
        const int v = TypeParam::value(value);
        EXPECT_EQ(TypeParam::key(key), v);
        value = TypeParam::create_value(v * v);
    }
    for (int i = 0; i < max; ++i) {
        const auto it = this->tree.find(TypeParam::create_key(i));
        EXPECT_NE(it, this->tree.end());
        EXPECT_EQ(i, TypeParam::key(*it));
        EXPECT_EQ(i * i, TypeParam::value(*it));
    }
    for (const auto & [key, value] : this->tree) {
        const int k = TypeParam::key(key);
        EXPECT_EQ(k * k, TypeParam::value(value));
    }
}

TYPED_TEST(BPTreeTest, unsorted_insert)
{
    const auto elements = {
        TypeParam::create(111), TypeParam::create(-1), TypeParam::create(0), TypeParam::create(31),
        TypeParam::create(7), TypeParam::create(11), TypeParam::create(17), TypeParam::create(97),
        TypeParam::create(1001), TypeParam::create(-59), TypeParam::create(23)
    };
    this->insert(elements.begin(), elements.end());

    std::vector<typename TypeParam::key_type> sorted_keys;
    sorted_keys.reserve(elements.size());
    for (const auto & x : elements) {
        sorted_keys.emplace_back(x.first);
    }
    std::sort(sorted_keys.begin(), sorted_keys.end());

    EXPECT_EQ(sorted_keys.size(), this->tree.size());
    auto keys_it = sorted_keys.begin();
    for (const auto & [key, value] : this->tree) {
        EXPECT_EQ(*keys_it, key);
        ++keys_it;
    }
}

TYPED_TEST(BPTreeTest, many)
{
    const int max = 11997;
    for (int i = 0; i < max; ++i) {
        this->insert(TypeParam::create(i));
    }
    EXPECT_FALSE(this->tree.empty());
    EXPECT_EQ(max, this->tree.size());
    for (int i = 0; i < max; ++i) {
        EXPECT_NE(this->tree.end(), this->tree.find(TypeParam::create_key(i))) << "Not found key " << i;
        EXPECT_EQ(1, this->tree.count(TypeParam::create_key(i))) << "Invalid number of entries for key " << i;
        EXPECT_EQ(i, TypeParam::key(*this->tree.find(TypeParam::create_key(i)))) << "Not found correct key value for " << i;
        auto it = this->tree.find(TypeParam::create_key(i));
        EXPECT_EQ(it, this->tree.lower_bound(TypeParam::create_key(i))) << "Not found correct lower bound for key " << i;
        ++it;
        EXPECT_EQ(it, this->tree.upper_bound(TypeParam::create_key(i))) << "Not found correct upper bound for key " << i;
        EXPECT_EQ(i, TypeParam::value(this->tree.at(TypeParam::create_key(i)))) << "Invalid value at " << i;
        EXPECT_EQ(i, TypeParam::value(this->tree[TypeParam::create_key(i)])) << "Invalid value at " << i;
    }
    for (const auto & [key, value] : this->tree) {
        EXPECT_EQ(TypeParam::key(key), TypeParam::value(value));
    }
    for (int i = -max; i < 0; ++i) {
        EXPECT_EQ(this->tree.end(), this->tree.find(TypeParam::create_key(i))) << "Found non-existing key " << i;
        EXPECT_EQ(0, this->tree.count(TypeParam::create_key(i))) << "Invalid number of entries for key " << i;
        EXPECT_EQ(this->tree.begin(), this->tree.lower_bound(TypeParam::create_key(i))) << "Not found correct lower bound for key " << i;
        EXPECT_EQ(this->tree.begin(), this->tree.upper_bound(TypeParam::create_key(i))) << "Not found correct upper bound for key " << i;
    }
    for (int i = max; i < 2 * max; ++i) {
        EXPECT_EQ(this->tree.end(), this->tree.find(TypeParam::create_key(i))) << "Found non-existing key " << i;
        EXPECT_EQ(0, this->tree.count(TypeParam::create_key(i))) << "Invalid number of entries for key " << i;
        EXPECT_EQ(this->tree.end(), this->tree.lower_bound(TypeParam::create_key(i))) << "Not found correct lower bound for key " << i;
        EXPECT_EQ(this->tree.end(), this->tree.upper_bound(TypeParam::create_key(i))) << "Not found correct upper bound for key " << i;
    }
}

TYPED_TEST(BPTreeTest, many_unsorted)
{
    const int max = 1001;
    std::vector<int> unsorted(max, 0);
    std::iota(unsorted.begin(), unsorted.end(), 0);
    {
        const auto copy = unsorted;
        unsorted.insert(unsorted.begin(), copy.begin(), copy.end());
        unsorted.insert(unsorted.begin(), copy.begin(), copy.end());
    }
    std::shuffle(unsorted.begin(), unsorted.end(), rgen);
    for (const auto x : unsorted) {
        this->insert(TypeParam::create(x));
    }
    std::vector<int> sorted = unsorted;
    std::sort(sorted.begin(), sorted.end());
    sorted.erase(std::unique(sorted.begin(), sorted.end()), sorted.end());
    EXPECT_EQ(sorted.size(), this->tree.size());
    for (const auto x : sorted) {
        EXPECT_NE(this->tree.end(), this->tree.find(TypeParam::create_key(x)));
        EXPECT_EQ(x, TypeParam::value(*this->tree.find(TypeParam::create_key(x))));
        auto it = this->tree.lower_bound(TypeParam::create_key(x));
        EXPECT_EQ(x, TypeParam::value(*it));
        ++it;
        EXPECT_EQ(it, this->tree.upper_bound(TypeParam::create_key(x)));
    }
    auto expected_it = sorted.begin();
    for (const auto & [key, value] : this->tree) {
        EXPECT_EQ(*expected_it, TypeParam::key(key));
        EXPECT_EQ(*expected_it, TypeParam::value(value));
        ++expected_it;
    }
}
