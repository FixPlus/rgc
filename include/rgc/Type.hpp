#ifndef RENDERGRAPHCOMPILER_TYPE_HPP
#define RENDERGRAPHCOMPILER_TYPE_HPP

#include <memory>
#include <ostream>
#include <span>
#include <typeindex>
#include <unordered_set>
#include <vector>

namespace rgc {

/**
 * @class Type
 *
 * Represents type of a value.
 *
 * There 2 kinds of types:
 * 1) Scalar - basic types, that represent allocatable resources
 * such as Image or Buffer.
 * 2) Aggregate - some form of composition of Scalar or other aggregate
 * types.
 *
 */
class Type {
public:
  enum Kind { Scalar, Aggregate };

  explicit Type(Kind kind) : m_kind(kind){};

  virtual size_t hash() const = 0;
  virtual bool equal(Type *another) const = 0;
  /**
   * @return std::type_index of C++ type instance of which could be used
   * to represent value of this Type. By default it is typeid(void)
   * which means there are no C++ type that can be used to represent
   * value of this type.
   */
  virtual std::type_index typeIndex() const { return typeid(void); }

  virtual void dump(std::ostream &os) const;

  auto typeKind() const { return m_kind; }
  struct Hash {
    std::size_t operator()(Type *t) const noexcept { return t->hash(); }
  };

  struct Equal {
    bool operator()(Type *t1, Type *t2) const noexcept { return t1->equal(t2); }
  };

  virtual ~Type() = default;

private:
  Kind m_kind;
};

class TypePool : public std::unordered_set<Type *, Type::Hash, Type::Equal> {
public:
  template <class T, typename... Args>
  requires std::derived_from<T, Type>
  auto *get(Args &&...args) {
    auto newT = std::make_unique<T>(std::forward<Args>(args)...);
    if (contains(newT.get())) {
      return *find(newT.get());
    }
    return *emplace(newT.release()).first;
  }
  TypePool() = default;
  TypePool(const TypePool &another) = delete;
  TypePool(TypePool &&another) = default;
  TypePool &operator=(const TypePool &another) = delete;
  TypePool &operator=(TypePool &&another) = default;

  ~TypePool() {
    for (auto &t : *this) {
      delete t;
    }
  }
};

/**
 * @class AggregateType
 *
 * Represents complex composed types.
 *
 */
class AggregateType : public Type {
public:
  enum Kind {
    Array,
    DynArray,
  };

  explicit AggregateType(Kind kind, std::span<Type *const> memberTypes)
      : Type(Type::Kind::Aggregate), m_kind(kind) {
    m_memberTypes.reserve(memberTypes.size());
    std::copy(memberTypes.begin(), memberTypes.end(),
              std::back_inserter(m_memberTypes));
  }
  std::span<Type *const> memberTypes() const { return m_memberTypes; }

  auto aggregateKind() const { return m_kind; }

private:
  Kind m_kind;
  std::vector<Type *> m_memberTypes;
};

/**
 * @class AggregateType
 *
 * Represents basic resource types.
 *
 */
class ScalarType : public Type {
public:
  enum OwnerType { Host, Device, None };
  enum Kind { Image, Buffer, Null, Integer };
  auto scalarKind() const { return m_kind; }
  auto ownerType() const { return m_ownerType; }
  ScalarType(Kind kind, OwnerType type)
      : Type(Type::Kind::Scalar), m_kind(kind), m_ownerType(type){};

private:
  OwnerType m_ownerType;
  Kind m_kind;
};

/**
 * @class NullType
 *
 * Special scalar type that represents value of null resource.
 * A special constant of such type can be used to represent
 * absent of dependency.
 * Only terminator actions can have this type.
 *
 */
class NullType final : public ScalarType {
public:
  explicit NullType()
      : ScalarType(ScalarType::Kind::Null, ScalarType::OwnerType::None){};

  size_t hash() const final { return 0u; }
  bool equal(Type *another) const final {
    return dynamic_cast<NullType *>(another);
  }
};
} // namespace rgc
#endif // RENDERGRAPHCOMPILER_TYPE_HPP
