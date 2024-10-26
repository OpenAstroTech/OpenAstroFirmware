#include "CommandParser.hpp"

#include <string>
#include <optional>
#include <memory>
#include <vector>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(lx200, CONFIG_LX200_LOG_LEVEL);

namespace lx200
{
    using namespace std;

    namespace tokenizer
    {
        struct MatchResult
        {
            size_t match_size;
            optional<string> arg;

            explicit operator bool() const
            {
                return match_size > 0;
            }
        };

        struct Token
        {
            unique_ptr<Token> next;

            Token(unique_ptr<Token> next) : next(move(next)) {}

            virtual MatchResult match(string str) = 0;
        };

        struct SeparatorToken : public Token
        {
            string value;

            SeparatorToken(const string &val, unique_ptr<Token> next = nullptr) : Token(move(next)), value(val) {}
            SeparatorToken(const char *val, unique_ptr<Token> next = nullptr) : Token(move(next)), value(val) {}

            SeparatorToken(const SeparatorToken &other) = default;
            ~SeparatorToken() = default;

            MatchResult match(string str) override
            {
                if (str.find(value) == 0)
                {
                    // return nullopt argument because a string token is just a separator for now
                    // LOG_INF("Matched string token: %s", value.c_str());
                    return MatchResult{value.length(), nullopt};
                }

                LOG_WRN("Failed to match string token \"%s\" in \"%s\"", value.data(), str.data());
                return MatchResult{0, nullopt};
            }
        };

        struct IntToken : public Token
        {
            size_t digits;

            /**
             * @brief Construct a new Int Token object
             *
             * @param digits The number of digits to match. If 0, match any number of digits (except zero).
             */
            IntToken(size_t digits = 0) : Token(nullptr), digits(digits) {}

            MatchResult match(string str) override
            {
                size_t size = 0;
                size_t sign = 0;
                while ((size + sign) < str.length() && (digits == 0 || size < digits))
                {
                    if (size == 0 && sign == 0 && (str[size] == '+' || str[size] == '-'))
                    {
                        sign++;
                    }
                    else if (isdigit(str[size + sign]))
                    {
                        size++;
                    } else {
                        break;
                    }
                }

                if (size > 0 && (digits == 0 || size == digits))
                {
                    return MatchResult{size + sign, str.substr(0, size + sign)};
                }

                LOG_WRN("Failed to match int token in \"%s\"", str.data());
                return MatchResult{0, nullopt};
            }
        };

        unique_ptr<Token> str(const char *val)
        {
            return make_unique<SeparatorToken>(val);
        }

        unique_ptr<Token> num(const int size = 0)
        {
            return make_unique<IntToken>(size);
        }

        template <typename Func>
        void parse(unique_ptr<Token> token, string command, Func func)
        {
            size_t index = 0;
            vector<string> results;

            while (token)
            {
                if (auto result = token->match(command.substr(index)))
                {

                    if (result.arg)
                    {
                        results.push_back(*result.arg);
                    }

                    if (token->next)
                    {
                        token = move(token->next);
                        index += result.match_size;
                    }
                    else
                    {
                        func(results);
                        return;
                    }
                }
                else
                {
                    LOG_WRN("Failed to match command: %s", command.data());
                    return;
                }
            }
        }
    } // namespace tokenizer

    unique_ptr<tokenizer::Token> operator+(unique_ptr<tokenizer::Token> lhs, unique_ptr<tokenizer::Token> rhs)
    {
        if (!lhs)
        {
            return move(rhs);
        }

        if (!rhs)
        {
            return move(lhs);
        }

        // find the last token in the lhs token chain
        unique_ptr<lx200::tokenizer::Token> *last = &lhs;
        while ((*last)->next)
        {
            last = &(*last)->next;
        }

        (*last)->next = move(rhs);
        return move(lhs);
    }

    unique_ptr<tokenizer::Token> operator+(const std::string &lhs, unique_ptr<tokenizer::Token> rhs) {
        return make_unique<tokenizer::SeparatorToken>(lhs) + move(rhs);
    }

    unique_ptr<tokenizer::Token> operator+(unique_ptr<tokenizer::Token> lhs, const std::string &rhs) {
        return move(lhs) + make_unique<tokenizer::SeparatorToken>(rhs);
    }

    void CommandParser::parse(string &command)
    {
#define COMMAND(tokens, func) \
    tokenizer::parse(tokens, command, [this](vector<string> args) func)

        // using namespace ctre;
        using namespace std;
        using namespace tokenizer;

        switch (command[1])
        {
        case 'I': // :I#
            COMMAND(str(":I#"), {
                mount.initialize();
            });
            return;
        case 'S': // Set family
            switch (command[2])
            {
            case 'd': // :SdsDD*MM:SS#
                COMMAND(":Sd" + num(2) + "*" + num(2) + ":" + num(2) + "#", {
                    mount.setTargetDec(stoi(args[0]), stoi(args[1]), stoi(args[2]));
                });
                return;
            case 'r': // :SrHH:MM:SS#
                COMMAND(":Sr" + num(2) + ":" + num(2) + ":" + num(2) + "#", {
                    mount.setTargetRa(stoi(args[0]), stoi(args[1]), stoi(args[2]));
                });
                return;
            }
        }

        LOG_WRN("Unknown command: %s", command.data());
    }
} // namespace lx200