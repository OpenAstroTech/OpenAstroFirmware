#pragma once

#include <cstddef>
#include <atomic>

enum MountCommandType
{
    InitializeMount,
    SetTargetRA
};

struct MountCommand
{
    const MountCommandType type;
    const size_t corr_id;

    MountCommand(const MountCommandType type) : corr_id(counter++), type(type) {}

    static std::atomic<unsigned long> counter;
};

struct InitializeMountCommand : public MountCommand
{
    InitializeMountCommand() : MountCommand(InitializeMount) {}
};

struct SetTargetRACommand : public MountCommand
{
    const unsigned int hh;
    const unsigned int mm;
    const unsigned int ss;

    SetTargetRACommand(unsigned int hh, unsigned int mm, unsigned int ss) : MountCommand(SetTargetRA), hh(hh), mm(mm), ss(ss) {}
};