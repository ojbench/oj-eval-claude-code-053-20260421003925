#pragma once
#include <queue>
#include <string>
#include <unordered_set>
#include <vector>

namespace Grammar {
class NFA;
NFA MakeStar(const char &character);
NFA MakePlus(const char &character);
NFA MakeQuestion(const char &character);
NFA Concatenate(const NFA &nfa1, const NFA &nfa2);
NFA Union(const NFA &nfa1, const NFA &nfa2);
NFA MakeSimple(const char &character);

enum class TransitionType { Epsilon, a, b };

struct Transition {
  TransitionType type;
  int to;
  Transition(TransitionType type, int to) : type(type), to(to) {}
};

class NFA {
private:
  int start;
  std::unordered_set<int> ends;
  std::vector<std::vector<Transition>> transitions;

public:
  NFA() = default;
  ~NFA() = default;

  std::unordered_set<int> GetEpsilonClosure(std::unordered_set<int> states) const {
    std::unordered_set<int> closure;
    std::queue<int> queue;
    for (const auto &state : states) {
      if (closure.find(state) != closure.end())
        continue;
      queue.push(state);
      closure.insert(state);
    }
    while (!queue.empty()) {
      int current = queue.front();
      queue.pop();
      for (const auto &transition : transitions[current]) {
        if (transition.type == TransitionType::Epsilon) {
          if (closure.find(transition.to) == closure.end()) {
            queue.push(transition.to);
            closure.insert(transition.to);
          }
        }
      }
    }
    return closure;
  }

  std::unordered_set<int> Advance(std::unordered_set<int> current_states,
                                  char character) const {
    // Get epsilon closure of current states
    std::unordered_set<int> epsilon_closure = GetEpsilonClosure(current_states);

    // Get next states by transitioning with the character
    std::unordered_set<int> next_states;
    TransitionType char_type = (character == 'a') ? TransitionType::a : TransitionType::b;

    for (int state : epsilon_closure) {
      for (const auto &transition : transitions[state]) {
        if (transition.type == char_type) {
          next_states.insert(transition.to);
        }
      }
    }

    // Get epsilon closure of next states
    if (next_states.empty()) {
      return next_states;
    }
    return GetEpsilonClosure(next_states);
  }

  bool IsAccepted(int state) const {
    return ends.find(state) != ends.end();
  }

  int GetStart() const {
    return start;
  }

  friend NFA MakeStar(const char &character);
  friend NFA MakePlus(const char &character);
  friend NFA MakeQuestion(const char &character);
  friend NFA MakeSimple(const char &character);
  friend NFA Concatenate(const NFA &nfa1, const NFA &nfa2);
  friend NFA Union(const NFA &nfa1, const NFA &nfa2);
};

class RegexChecker {
private:
  NFA nfa;

public:
  bool Check(const std::string &str) const {
    std::unordered_set<int> current_states;
    current_states.insert(nfa.GetStart());

    // Process each character
    for (char c : str) {
      current_states = nfa.Advance(current_states, c);
      if (current_states.empty()) {
        return false;
      }
    }

    // Check if any of the current states is an accept state
    for (int state : current_states) {
      if (nfa.IsAccepted(state)) {
        return true;
      }
    }
    return false;
  }

  RegexChecker(const std::string &regex) {
    // Parse regex and build NFA
    // | has lowest precedence, so split by | first
    std::vector<std::string> parts;
    std::string current_part;

    for (char c : regex) {
      if (c == '|') {
        if (!current_part.empty()) {
          parts.push_back(current_part);
          current_part.clear();
        }
      } else {
        current_part += c;
      }
    }
    if (!current_part.empty()) {
      parts.push_back(current_part);
    }

    // Build NFA for each part separated by |
    std::vector<NFA> nfas;
    for (const auto &part : parts) {
      // Parse concatenation
      NFA part_nfa;
      bool has_nfa = false;

      for (size_t i = 0; i < part.size(); ++i) {
        char c = part[i];
        NFA current_nfa;

        if (c == 'a' || c == 'b') {
          // Check for operators after this character
          if (i + 1 < part.size()) {
            char op = part[i + 1];
            if (op == '*') {
              current_nfa = MakeStar(c);
              ++i;
            } else if (op == '+') {
              current_nfa = MakePlus(c);
              ++i;
            } else if (op == '?') {
              current_nfa = MakeQuestion(c);
              ++i;
            } else {
              current_nfa = MakeSimple(c);
            }
          } else {
            current_nfa = MakeSimple(c);
          }

          if (!has_nfa) {
            part_nfa = current_nfa;
            has_nfa = true;
          } else {
            part_nfa = Concatenate(part_nfa, current_nfa);
          }
        }
      }

      if (has_nfa) {
        nfas.push_back(part_nfa);
      }
    }

    // Union all parts
    if (!nfas.empty()) {
      nfa = nfas[0];
      for (size_t i = 1; i < nfas.size(); ++i) {
        nfa = Union(nfa, nfas[i]);
      }
    }
  }
};

NFA MakeStar(const char &character) {
  NFA nfa;
  nfa.start = 0;
  nfa.ends.insert(0);
  nfa.transitions.push_back(std::vector<Transition>());
  if (character == 'a') {
    nfa.transitions[0].push_back({TransitionType::a, 0});
  } else {
    nfa.transitions[0].push_back({TransitionType::b, 0});
  }
  return nfa;
}

NFA MakePlus(const char &character) {
  // a+ means one or more 'a'
  // We need: start state -> a -> accept state, accept state -> a -> accept state
  NFA nfa;
  nfa.start = 0;
  nfa.ends.insert(1);
  nfa.transitions.push_back(std::vector<Transition>());
  nfa.transitions.push_back(std::vector<Transition>());

  if (character == 'a') {
    nfa.transitions[0].push_back({TransitionType::a, 1});
    nfa.transitions[1].push_back({TransitionType::a, 1});
  } else {
    nfa.transitions[0].push_back({TransitionType::b, 1});
    nfa.transitions[1].push_back({TransitionType::b, 1});
  }
  return nfa;
}

NFA MakeQuestion(const char &character) {
  // a? means zero or one 'a'
  // start state is also accept state, and has transition to another accept state
  NFA nfa;
  nfa.start = 0;
  nfa.ends.insert(0);
  nfa.ends.insert(1);
  nfa.transitions.push_back(std::vector<Transition>());
  nfa.transitions.push_back(std::vector<Transition>());

  if (character == 'a') {
    nfa.transitions[0].push_back({TransitionType::a, 1});
  } else {
    nfa.transitions[0].push_back({TransitionType::b, 1});
  }
  return nfa;
}

NFA Concatenate(const NFA &nfa1, const NFA &nfa2) {
  // Concatenate nfa1 and nfa2
  NFA nfa;
  int offset = nfa1.transitions.size();

  // Copy nfa1's transitions
  nfa.transitions = nfa1.transitions;

  // Copy nfa2's transitions with offset
  for (const auto &trans_list : nfa2.transitions) {
    std::vector<Transition> new_trans_list;
    for (const auto &trans : trans_list) {
      new_trans_list.push_back({trans.type, trans.to + offset});
    }
    nfa.transitions.push_back(new_trans_list);
  }

  // Add epsilon transitions from nfa1's end states to nfa2's start state
  for (int end_state : nfa1.ends) {
    nfa.transitions[end_state].push_back({TransitionType::Epsilon, nfa2.start + offset});
  }

  // Set start state
  nfa.start = nfa1.start;

  // Set end states (only nfa2's end states)
  for (int end_state : nfa2.ends) {
    nfa.ends.insert(end_state + offset);
  }

  return nfa;
}

NFA Union(const NFA &nfa1, const NFA &nfa2) {
  // Create new start state
  NFA nfa;
  nfa.start = 0;
  nfa.transitions.push_back(std::vector<Transition>());

  int offset1 = 1;
  int offset2 = offset1 + nfa1.transitions.size();

  // Copy nfa1's transitions with offset1
  for (const auto &trans_list : nfa1.transitions) {
    std::vector<Transition> new_trans_list;
    for (const auto &trans : trans_list) {
      new_trans_list.push_back({trans.type, trans.to + offset1});
    }
    nfa.transitions.push_back(new_trans_list);
  }

  // Copy nfa2's transitions with offset2
  for (const auto &trans_list : nfa2.transitions) {
    std::vector<Transition> new_trans_list;
    for (const auto &trans : trans_list) {
      new_trans_list.push_back({trans.type, trans.to + offset2});
    }
    nfa.transitions.push_back(new_trans_list);
  }

  // Add epsilon transitions from new start state to both nfa1 and nfa2 start states
  nfa.transitions[0].push_back({TransitionType::Epsilon, nfa1.start + offset1});
  nfa.transitions[0].push_back({TransitionType::Epsilon, nfa2.start + offset2});

  // Set end states (both nfa1 and nfa2 end states)
  for (int end_state : nfa1.ends) {
    nfa.ends.insert(end_state + offset1);
  }
  for (int end_state : nfa2.ends) {
    nfa.ends.insert(end_state + offset2);
  }

  return nfa;
}

NFA MakeSimple(const char &character) {
  // Simple single character match: start -> character -> accept
  NFA nfa;
  nfa.start = 0;
  nfa.ends.insert(1);
  nfa.transitions.push_back(std::vector<Transition>());
  nfa.transitions.push_back(std::vector<Transition>());

  if (character == 'a') {
    nfa.transitions[0].push_back({TransitionType::a, 1});
  } else {
    nfa.transitions[0].push_back({TransitionType::b, 1});
  }
  return nfa;
}

} // namespace Grammar
