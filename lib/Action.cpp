#include "rgc/Action.hpp"

namespace rgc {
Action::~Action() {
  // Remove itself as user from all used values
  for (auto &&val : m_uses)
    val->removeUser(this);
}
void Action::dump(std::ostream &os) const {
  os << "Action " << this << " [use: ";
  for (auto &&val : m_uses) {
    os << val << ", ";
  }
  if (m_uses.empty())
    os << "<empty>";
  os << "] produces: ";
  Value::dump(os);
}

} // namespace rgc