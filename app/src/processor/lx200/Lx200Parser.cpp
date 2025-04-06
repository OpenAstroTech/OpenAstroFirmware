#include "Lx200Parser.hpp"

// C++ standard library headers
#include <functional>

// Zephyr headers
#include <zephyr/logging/log.h>

// Project-specific headers
#include "Tokenizer.hpp"

LOG_MODULE_DECLARE(lx200, CONFIG_LX200_LOG_LEVEL);

using namespace std;
using namespace tokenizer;

bool Lx200Parser::parse(string &command, Lx200CommandHandler *handler)
{
    auto bind = [&](auto Fn, auto... args)
    {
        return std::bind_front(Fn, handler, args...);
    };

    switch (command[1])
    {
    case 'I': // :I#
        return s(":I#").eval(command, bind(&Lx200CommandHandler::handleInitialize));
    case 'S': // Set family
        switch (command[2])
        {
        case 'd': // :SdsDD*MM:SS#
            return (":Sd" + I + "*" + U + ":" + U + "#").eval(command, bind(&Lx200CommandHandler::handleSetDec));
        case 'r': // :SrHH:MM:SS#
            return (":Sr" + U + ":" + U + ":" + U + "#").eval(command, bind(&Lx200CommandHandler::handleSetRa));
        }
    }

    LOG_WRN("Unknown command: %s", command.data());
    return false;
}