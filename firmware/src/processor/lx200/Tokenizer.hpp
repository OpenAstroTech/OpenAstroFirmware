#include <string_view>
#include <tuple>
#include <optional>
#include <utility>     // std::move, std::apply, std::forward
#include <charconv>    // std::from_chars
#include <type_traits> // For requires clause if not using full <concepts>
#include <version>     // For feature checks

// Optional: Include concepts header for more advanced constraints if needed
// #include <concepts>

// --- C++ Feature Checks (Revised) ---
#if !defined(__cpp_concepts) || __cpp_concepts < 201907L
#error "Requires C++20 Concepts"
#endif

// Check for std::from_chars support (accept __cpp_lib_to_chars as sufficient)
#if (!defined(__cpp_lib_charconv) || __cpp_lib_charconv < 201611L) && \
    (!defined(__cpp_lib_to_chars) || __cpp_lib_to_chars < 201611L)
#error "Requires C++17 <charconv> (std::from_chars support)"
#endif

#if !defined(__cpp_lib_starts_ends_with) || __cpp_lib_starts_ends_with < 201711L
#warning "C++20 std::string_view::starts_with not fully supported?"
#endif
#if !defined(__cpp_lib_apply) || __cpp_lib_apply < 201603L
#error "Requires C++17 std::apply" // Core dependency
#endif

namespace tokenizer
{

    // --- Forward Declarations ---
    template <typename L, typename R>
    struct SequenceMatcher;

    // --- Result Type for Evaluation ---
    // Contains the remaining string view and the captured tuple
    template <typename TupleType>
    struct EvalResult
    {
        std::string_view remaining;
        TupleType captured;
    };

    // --- Forward declare the free eval function (implementation detail) ---
    template <typename Matcher, typename Func>
    bool detail_eval(const Matcher &matcher, std::string_view input, Func &&func);

    // --- Static String Matcher ---
    struct StaticStringMatcher
    {
        std::string_view expected;

        // Constructor from C-string literal
        constexpr StaticStringMatcher(const char *s) : expected(s) {}
        // Constructor from std::string_view (more useful in C++20 constexpr)
        constexpr StaticStringMatcher(std::string_view sv) : expected(sv) {}

        // Internal evaluate method
        template <typename CurrentTuple>
        constexpr std::optional<EvalResult<CurrentTuple>>
        evaluate(std::string_view input, CurrentTuple current_captured) const
        {
            // Use C++20 starts_with
            if (input.starts_with(expected))
            {
                return EvalResult<CurrentTuple>{
                    input.substr(expected.size()),
                    std::move(current_captured) // No new captures
                };
            }
            return std::nullopt;
        }

        // Public member eval function
        template <typename Func>
        bool eval(std::string_view input, Func &&func) const
        {
            // Forward to the actual implementation function
            return tokenizer::detail_eval(*this, input, std::forward<Func>(func));
        }
    };

    // --- Unsigned Integer Matcher ---
    struct UnsignedIntMatcher
    {
        // Internal evaluate method
        template <typename CurrentTuple>
        std::optional<EvalResult<decltype(std::tuple_cat(std::declval<CurrentTuple>(), std::tuple<unsigned int>{}))>>
        evaluate(std::string_view input, CurrentTuple current_captured) const
        {
            unsigned int value = 0;
            const char *const start = input.data();
            const char *const end = input.data() + input.size();
            auto result = std::from_chars(start, end, value);

            // Check for success and that at least one digit was consumed
            if (result.ec == std::errc{} && result.ptr != start)
            {
                // Determine the type of the tuple after adding the new value
                using NextTuple = decltype(std::tuple_cat(std::move(current_captured), std::tuple<unsigned int>{}));
                return EvalResult<NextTuple>{
                    input.substr(result.ptr - start),                                            // Remaining view
                    std::tuple_cat(std::move(current_captured), std::tuple<unsigned int>{value}) // Combined tuple
                };
            }
            return std::nullopt;
        }

        // Public member eval function
        template <typename Func>
        bool eval(std::string_view input, Func &&func) const
        {
            return tokenizer::detail_eval(*this, input, std::forward<Func>(func));
        }
    };

    // --- Signed Integer Matcher ---
    struct SignedIntMatcher
    {
        // Internal evaluate method
        template <typename CurrentTuple>
        std::optional<EvalResult<decltype(std::tuple_cat(std::declval<CurrentTuple>(), std::tuple<int>{}))>>
        evaluate(std::string_view input, CurrentTuple current_captured) const
        {
            int value = 0;
            const char *const start = input.data();
            const char *const end = input.data() + input.size();
            auto result = std::from_chars(start, end, value);

            // Check for success and consumption
            if (result.ec == std::errc{} && result.ptr != start)
            {
                using NextTuple = decltype(std::tuple_cat(std::move(current_captured), std::tuple<int>{}));
                return EvalResult<NextTuple>{
                    input.substr(result.ptr - start),
                    std::tuple_cat(std::move(current_captured), std::tuple<int>{value})};
            }
            return std::nullopt;
        }

        // Public member eval function
        template <typename Func>
        bool eval(std::string_view input, Func &&func) const
        {
            return tokenizer::detail_eval(*this, input, std::forward<Func>(func));
        }
    };

    // --- Sequence Matcher (Result of '+') ---
    template <typename L, typename R>
    struct SequenceMatcher
    {
        L lhs;
        R rhs;

        // Store the combined matchers
        constexpr SequenceMatcher(L l, R r) : lhs(std::move(l)), rhs(std::move(r)) {}

        // Internal evaluate method: Evaluate LHS, then RHS
        template <typename CurrentTuple>
        constexpr auto evaluate(std::string_view input, CurrentTuple current_captured) const
        {
            // Evaluate Left Hand Side
            auto lhs_result_opt = lhs.evaluate(input, std::move(current_captured));

            if (!lhs_result_opt)
            {
                // If LHS failed, the sequence fails. Return nullopt of the correct final type.
                // We deduce the return type of the RHS evaluation to get the correct optional type.
                using RhsEvalResultType = decltype(rhs.evaluate(input, lhs_result_opt->captured));
                // Check if the deduced type is indeed an optional (it should be)
                if constexpr (requires { RhsEvalResultType{std::nullopt}; })
                {
                    return RhsEvalResultType{std::nullopt};
                }
                else
                {
                    // This should ideally not happen if all evaluate methods return optionals
                    // Provide a safe fallback or trigger a compile error
                    static_assert(!sizeof(RhsEvalResultType), "Internal error: RHS evaluate did not return an optional type");
                    // return std::optional<EvalResult<std::tuple<>>>{std::nullopt}; // Fallback
                }
            }

            // If LHS succeeded, evaluate RHS with remaining input and captured values from LHS
            return rhs.evaluate(lhs_result_opt->remaining, std::move(lhs_result_opt->captured));
        }

        // Public member eval function
        template <typename Func>
        bool eval(std::string_view input, Func &&func) const
        {
            return tokenizer::detail_eval(*this, input, std::forward<Func>(func));
        }
    };

    // --- Factory Functions ---
    constexpr StaticStringMatcher s(const char *sv)
    {
        return StaticStringMatcher(sv);
    }
    // Overload for string_view literals (more natural in C++20)
    constexpr StaticStringMatcher s(std::string_view sv)
    {
        return StaticStringMatcher(sv);
    }

    constexpr UnsignedIntMatcher u()
    {
        return UnsignedIntMatcher{};
    }

    constexpr SignedIntMatcher i()
    {
        return SignedIntMatcher{};
    }

    constexpr auto I = i();
    constexpr auto U = u();

    // --- Operator+ Overload ---
    // Use C++20 requires clause for basic constraint checking
    template <typename T1, typename T2>
    // Ensures we don't accidentally overload '+' for arithmetic types etc.
    // Can be made more specific if a Matcher concept is defined.
        requires(!std::is_arithmetic_v<T1> && !std::is_arithmetic_v<T2>)
    constexpr SequenceMatcher<T1, T2> operator+(T1 lhs, T2 rhs)
    {
        return SequenceMatcher<T1, T2>(std::move(lhs), std::move(rhs));
    }

    template <typename T2>
    constexpr SequenceMatcher<StaticStringMatcher, T2> operator+(const char *lhs, T2 rhs)
    {
        return SequenceMatcher<StaticStringMatcher, T2>(s(lhs), std::move(rhs));
    }

    template <typename T1>
    constexpr SequenceMatcher<T1, StaticStringMatcher> operator+(T1 lhs, const char *rhs)
    {
        return SequenceMatcher<T1, StaticStringMatcher>(std::move(lhs), s(rhs));
    }

    // --- Top-Level Eval Implementation Function (Internal Detail) ---
    template <typename Matcher, typename Func>
    bool detail_eval(const Matcher &matcher, std::string_view input, Func &&func)
    {
        // Start evaluation process with the full input and an empty capture tuple
        auto result_opt = matcher.evaluate(input, std::tuple<>{});

        // Check if matching succeeded AND the entire input was consumed
        if (result_opt && result_opt->remaining.empty())
        {
            // Use std::apply to unpack the captured tuple elements as arguments to the lambda
            // If the lambda signature doesn't match the tuple elements, std::apply will fail compilation,
            // which provides decent feedback.
            std::apply(std::forward<Func>(func), std::move(result_opt->captured));
            return true; // Indicate success
        }
        else
        {
            // Basic error message if the pattern didn't match the entire input
            // Could be enhanced to show where matching failed.

            // std::cerr << "Input '" << input << "' did not fully match the pattern.\n";
            // TODO: Provide more detailed error information
            return false;
        }
    }

} // namespace tokenizer
