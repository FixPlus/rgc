#ifndef RENDERGRAPHCOMPILER_CONSTANT_HPP
#define RENDERGRAPHCOMPILER_CONSTANT_HPP

#include "rgc/Action.hpp"
#include "rgc/Type.hpp"
#include <any>
#include <unordered_set>

namespace rgc {

/**
 * @class Constant
 *
 * Value that is known at compile time.
 *
 */
class Constant : public Value {
public:
  explicit Constant(Type *type) : Value(type) {}

  virtual size_t hash() const = 0;
  bool equal(Constant *another) const { return m_equal(another); }

  struct Hash {
    std::size_t operator()(Constant *s) const noexcept { return s->hash(); }
  };

  struct Equal {
    bool operator()(Constant *c1, Constant *c2) const noexcept {
      return c1->equal(c2);
    }
  };

protected:
  virtual bool m_equal(Constant *another) const = 0;
};

/**
 * @class ConstantPool
 *
 * Set of constants.
 *
 */
class ConstantPool
    : public std::unordered_set<Constant *, Constant::Hash, Constant::Equal> {
public:
  template <class CT, typename... Args>
  requires std::derived_from<CT, Constant>
  auto *get(Args &&...args) {
    auto newC = std::make_unique<CT>(std::forward<Args>(args)...);
    if (contains(newC.get())) {
      return *find(newC.get());
    }
    return *emplace(newC.release()).first;
  }
  ConstantPool() = default;
  ConstantPool(const ConstantPool &another) = delete;
  ConstantPool(ConstantPool &&another) = default;
  ConstantPool &operator=(const ConstantPool &another) = delete;
  ConstantPool &operator=(ConstantPool &&another) = default;

  ~ConstantPool() {
    for (auto &c : *this) {
      delete c;
    }
  }
};

/**
 * @class NullConstant
 *
 * Constant that represents null. Can be used in Actions to
 * represent absent of value dependency where appropriate.
 *
 */

class NullConstant : public Constant {
public:
  explicit NullConstant(TypePool &typePool)
      : Constant(typePool.get<NullType>()) {}

  size_t hash() const override { return 0; };

  bool m_equal(Constant *another) const override { return true; }
};

} // namespace rgc
#endif // RENDERGRAPHCOMPILER_CONSTANT_HPP
