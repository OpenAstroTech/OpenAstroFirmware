#pragma once

class IMount
{
public:
    virtual ~IMount() = default;

    /**
     * @brief Initialize the mount
     */
    virtual void initialize() = 0;

    /**
     * @brief Set the Target Dec
     *
     * @param d degrees (-90 to +90)
     * @param m arcminutes (0-59)
     * @param s arcseconds (0-59)
     *
     * @return true if successful, false otherwise
     */
    virtual bool setTargetDec(int d, unsigned int m, unsigned int s) = 0;

    /**
     * @brief Set the Target Ra
     *
     * @param h hours (0-23)
     * @param m minutes (0-59)
     * @param s seconds (0-59)
     *
     * @return true if successful, false otherwise
     */
    virtual bool setTargetRa(unsigned int h, unsigned int m, unsigned int s) = 0;
};