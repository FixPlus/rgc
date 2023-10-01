#include <cassert>

#include "rgc/Action.hpp"
#include "rgc/Value.hpp"

namespace rgc {
void Value::dump(std::ostream &os) const {
  os << "Value " << this << " t: ";
  type()->dump(os);
  os << " [users: ";
  for (auto &&[action, index] : m_users)
    os << "(a: " << action << ", i: " << index << "); ";
  if (m_users.empty()) {
    os << "<unused>";
  }
  os << "]";
}
Value::~Value() { assert(unused() && "Trying to remove value in use"); }
void Value::replaceAllUsesWith(Value *value) {
  for (auto &&[action, index] : m_users) {
    action->replaceUse(index, value);
    value->addUser(action, index);
  }
  m_users.clear();
}

} // namespace rgc