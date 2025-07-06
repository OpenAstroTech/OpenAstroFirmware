#pragma once

#include <inttypes.h>

class Mount
{
public:
    Mount();
    ~Mount();

    /**
     * @brief Initialize the mount
     */
    void initialize();

    /**
     * @brief Set the Target Dec
     *
     * @param d degrees (-90 to +90)
     * @param m arcminutes (0-59)
     * @param s arcseconds (0-59)
     * 
     * @return true if successful, false otherwise
     */
    bool setTargetDec(int d, unsigned int m, unsigned int s);

    /**
     * @brief Set the Target Ra
     *
     * @param h hours (0-23)
     * @param m minutes (0-59)
     * @param s seconds (0-59)
     * 
     * @return true if successful, false otherwise
     */
    bool setTargetRa(unsigned int h, unsigned int m, unsigned int s);
};
