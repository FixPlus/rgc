#ifndef RENDERGRAPHCOMPILER_ACTION_HPP
#define RENDERGRAPHCOMPILER_ACTION_HPP

#include <memory>
#include <ostream>
#include <span>
#include <vector>

#include "rgc/IList.hpp"
#include "rgc/Type.hpp"
#include "rgc/Value.hpp"

namespace rgc {

class Value;

/**
 * @class Action
 *
 * Represent a use and def point for values.
 *
 * There are 4 kinds of Actions:
 *
 * 1) Allocation - always a first def point for resource. May
 * or may not use other values.
 * 2) Composition - do not produce new resources, but rather groups
 * them together. Although it may use multiple values, it is
 * never a use point for any underlying resources. Must use at least
 * one value.
 * 3) RealAction - represent some kind of work that modifies a resource.
 * It is always use-def point for that resource. May be a use point
 * for some other value that is distinct from modified resource.
 * 4) Terminator - last use point for resource. Value of terminator
 * must not be used further.
 *
 */
class Action : public IListNode<Action>, public Value {
public:
  enum Kind { Allocation, Composition, RealAction, Terminator };

  Action(Kind kind, Type *type) : Value(type), m_kind(kind){};

  std::span<Value *const> uses() const { return m_uses; }

  void replaceUse(unsigned Index, Value *value) { m_uses.at(Index) = value; }

  auto actionKind() const { return m_kind; }

  void dump(std::ostream &os) const override;

  ~Action() override;

protected:
  void m_push_use(Value *v) {
    assert(v && "value cannot be nullptr");
    v->addUser(this, m_uses.size());
    m_uses.push_back(v);
  }

private:
  std::vector<Value *> m_uses;
  Kind m_kind;
};

/**
 * @class Allocation
 *
 * Action that allocates resources.
 * It is a starting definition in any use-def sequence. That way
 * it is a leaf node.
 * Allocated resource may remain uninitialized until next def point.
 *
 * There are 2 types of Allocation:
 * 1) Static - it does not depend on any other value.
 * 2) Dynamic - allocation process depends on precalculation of some
 * other value.
 *
 */
class Allocation : public Action {
public:
  enum Kind { Static, Dynamic };

  explicit Allocation(Type *type)
      : Action(Action::Kind::Allocation, type), m_kind(Kind::Static) {}

  explicit Allocation(Type *type, Value *use)
      : Action(Action::Kind::Allocation, type), m_kind(Kind::Dynamic) {
    m_push_use(use);
  }
  auto allocationKind() const { return m_kind; }

private:
  Kind m_kind;
};

/**
 * @class Composition
 *
 * Action that combines/transforms one or several values in another one.
 * It does not use nor define underlying resources.
 *
 */
class Composition : public Action {
public:
  Composition(Type *type, std::span<Value *> uses)
      : Action(Action::Kind::Composition, type) {
    for (auto &&use : uses)
      m_push_use(use);
  }
};

/**
 * @class RealAction
 *
 * Action that performs any modification to it's useDef resource. Value type
 * of RealAction is always the same as useDef value.
 * For it's useDef resource, RealAction is a point of use and simultaneously
 * a point of next def in a sequence.
 * RealAction also has a 'use' value, for any resource of which it is
 * strictly a point of use.
 * 'use' value may be a constant of NullType.
 */
class RealAction : public Action {
public:
  RealAction(Value *useDef, Value *use)
      : Action(Action::Kind::RealAction, useDef->type()) {
    m_push_use(useDef);
    m_push_use(use);
  }

  auto *getUseDef() const { return uses()[0]; }

  auto *getUse() const { return uses()[1]; }
};

/**
 * @class Terminator
 *
 * Action that terminates a use-def sequence of resource, being it's final use
 * point. Type of all terminators is NullType.
 *
 */
class Terminator : public Action {
public:
  explicit Terminator(TypePool &tp, Value *use)
      : Action(Action::Kind::Terminator, tp.get<NullType>()) {
    m_push_use(use);
  }
};

} // namespace rgc

#endif // RENDERGRAPHCOMPILER_ACTION_HPP
