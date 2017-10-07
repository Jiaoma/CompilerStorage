template <typename Left, typename Right>
struct ConcatExpr;

template <typename Left, typename Right>
struct AltExpr;

template <typename SubExpr>
struct RepeatExpr;

template <char ch>
struct MatchExpr;

struct EpsilonExpr;

template <typename SubExpr>
using OptionalExpr = AltExpr<SubExpr, EpsilonExpr>;

template <typename RegExpr>
struct MatchImpl;

template <typename Left, typename Right>
struct MatchImpl<ConcatExpr<Left, Right>> {
    template <typename Continuation>
    static bool Apply(const char* target, Continuation cont) {
        return MatchImpl<Left>::Apply(target, [cont](const char* rest) -> bool {
            return MatchImpl<Right>::Apply(rest, cont);
        });
    }
};

template <typename Left, typename Right>
struct MatchImpl<AltExpr<Left, Right>> {
    template <typename Continuation>
    static bool Apply(const char* target, Continuation cont) {
        return MatchImpl<Left>::Apply(target, cont) ||
        MatchImpl<Right>::Apply(target, cont);
    }
};

template <typename SubExpr>
struct MatchImpl<RepeatExpr<SubExpr>> {
    template <typename Continuation>
    static bool Apply(const char* target, Continuation cont) {
        return MatchImpl<SubExpr>::Apply(
                                         target,
                                         [target, cont](const char* rest) -> bool {
                                             return target < rest &&
                                             MatchImpl<RepeatExpr<SubExpr>>::Apply(rest, cont);
                                         }) ||
        cont(target);
    }
};

template <char ch>
struct MatchImpl<MatchExpr<ch>> {
    template <typename Continuation>
    static bool Apply(const char* target, Continuation cont) {
        return *target && *target == ch && cont(target + 1);
    }
};

template <>
struct MatchImpl<EpsilonExpr> {
    template <typename Continuation>
    static bool Apply(const char* target, Continuation cont) {
        return cont(target);
    }
};

template <typename RegExpr>
bool RegexMatch(const char* target) {
    return MatchImpl<RegExpr>::Apply(
                                     target, [](const char* rest) -> bool { return *rest == '\0'; });
}

template <typename RegExpr>
bool RegexSearch(const char* target) {
    return MatchImpl<RegExpr>::Apply(
                                     target, [](const char* rest) -> bool { return true; }) ||
    (*target && RegexSearch<RegExpr>(target + 1));
}

#include <cassert>

int main() {
    assert((RegexMatch<ConcatExpr<MatchExpr<'a'>, MatchExpr<'b'>>>("ab")));
    assert((RegexMatch<AltExpr<MatchExpr<'a'>, MatchExpr<'b'>>>("a")));
    assert((RegexMatch<AltExpr<MatchExpr<'a'>, MatchExpr<'b'>>>("b")));
    assert((RegexMatch<RepeatExpr<MatchExpr<'a'>>>("aaaaa")));
    assert((RegexMatch<ConcatExpr<RepeatExpr<MatchExpr<'a'>>, MatchExpr<'b'>>>(
                                                                               "aaaaab")));
    assert((
            RegexMatch<ConcatExpr<RepeatExpr<MatchExpr<'a'>>, MatchExpr<'b'>>>("b")));
    assert((RegexSearch<ConcatExpr<RepeatExpr<MatchExpr<'a'>>, MatchExpr<'b'>>>(
                                                                                "aaaaabb")));
    assert((RegexMatch<OptionalExpr<MatchExpr<'a'>>>("a")));
    assert((RegexMatch<OptionalExpr<MatchExpr<'a'>>>("")));
    assert((RegexMatch<OptionalExpr<ConcatExpr<MatchExpr<'a'>, MatchExpr<'b'>>>>(
                                                                                 "ab")));
    assert((RegexMatch<OptionalExpr<ConcatExpr<MatchExpr<'a'>, MatchExpr<'b'>>>>(
                                                                                 "")));
    assert((!RegexMatch<RepeatExpr<MatchExpr<'a'>>>("aaaaab")));
    assert((RegexMatch<RepeatExpr<OptionalExpr<MatchExpr<'a'>>>>("")));
    return 0;
}
