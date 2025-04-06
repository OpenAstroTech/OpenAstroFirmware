#pragma once

/**
 * @brief Interface for handling LX200 protocol commands.
 *
 * This abstract class defines the interface for handling commands following
 * the LX200 telescope control protocol. Classes that implement this interface
 * are responsible for processing specific LX200 commands and executing the
 * appropriate telescope control actions.
 *
 * Derived classes must implement the pure virtual functions to handle
 * initialization and coordinate setting operations.
 */
class Lx200CommandHandler
{
public:
    /**
     * @brief Initializes the LX200 command handler.
     *
     * This method should be called once after the command handler is created to
     * perform any necessary setup and initialization. Since this is a pure virtual
     * function, derived classes must provide their own implementation.
     */
    virtual void handleInitialize() = 0;

    /**
     * @brief Sets the telescope's declination coordinates.
     *
     * This method should be called when the telescope's declination coordinates
     * are updated. The declination is specified in degrees, arcminutes, and
     * arcseconds. Since this is a pure virtual function, derived classes must
     * provide their own implementation.
     *
     * @param degrees The declination in degrees (-90 to +90)
     * @param arcminutes The declination in arcminutes (0-59)
     * @param arcseconds The declination in arcseconds (0-59)
     */
    virtual void handleSetDec(int degrees, unsigned int arcminutes, unsigned int arcseconds) = 0;

    /**
     * @brief Sets the telescope's right ascension coordinates.
     *
     * This method should be called when the telescope's right ascension coordinates
     * are updated. The right ascension is specified in hours, minutes, and seconds.
     * Since this is a pure virtual function, derived classes must provide their own
     * implementation.
     *
     * @param hours The right ascension in hours (0-23)
     * @param minutes The right ascension in minutes (0-59)
     * @param seconds The right ascension in seconds (0-59)
     */
    virtual void handleSetRa(unsigned int hours, unsigned int minutes, unsigned int seconds) = 0;
};
