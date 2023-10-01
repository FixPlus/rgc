#ifndef RENDERGRAPHCOMPILER_VALUE_HPP
#define RENDERGRAPHCOMPILER_VALUE_HPP

#include <ostream>
#include <unordered_map>

namespace rgc {

class Action;
class Type;

class Value {
public:
  explicit Value(Type *type) : m_type(type){};

  auto &users() const { return m_users; }

  bool unused() const { return m_users.empty(); }

  bool hasUser(Action *action) const { return m_users.contains(action); }

  void addUser(Action *action, unsigned Index) {
    assert(!m_users.contains(action) && "double insertion");
    m_users.emplace(action, Index);
  }

  void replaceAllUsesWith(Value *value);

  void removeUser(Action *action) { m_users.erase(action); }

  virtual void dump(std::ostream &os) const;

  auto *type() const { return m_type; }

  virtual ~Value();

private:
  std::unordered_map<Action *, unsigned> m_users;
  Type *m_type;
};

} // namespace rgc

#endif // RENDERGRAPHCOMPILER_VALUE_HPP
