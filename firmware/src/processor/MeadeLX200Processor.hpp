#include <MeadeLX200Parser.hpp>

class MeadeLX200Processor : public MeadeLX200Parser
{
public:
    MeadeLX200Processor();
    ~MeadeLX200Processor();
protected:
    void handleInitializeTelescope();
};