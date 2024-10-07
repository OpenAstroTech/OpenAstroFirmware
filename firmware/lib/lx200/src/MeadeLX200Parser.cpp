#include "MeadeLX200Parser.hpp"

#include <string>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(lx200, CONFIG_LX200_LOG_LEVEL);

MeadeLX200Parser::MeadeLX200Parser() {}

MeadeLX200Parser::~MeadeLX200Parser() {}

void MeadeLX200Parser::processCommand(std::string command) {
    LOG_INF("Processing command: %s", command.c_str());

    // if command does not start with ":" or end with "#", it is invalid
    if (command[0] != ':' || command[command.size() - 1] != '#') {
        LOG_WRN("Invalid command: %s", command.c_str());
        return;
    }

    // remove ":" and "#" from the command
    command = command.substr(1, command.size() - 2);

    if (command == "I") {
        handleInitializeTelescope();
    } else {
        LOG_WRN("Unknown command: %s", command.c_str());
    }
}

// Default implementations with LOG_ERR
void MeadeLX200Parser::handleGetLocalTime24Hour() {
    LOG_ERR("handleGetLocalTime24Hour function not implemented");
}

void MeadeLX200Parser::handleGetLargerSizeLimit() {
    LOG_ERR("handleGetLargerSizeLimit function not implemented");
}

void MeadeLX200Parser::handleGetSite1Name() {
    LOG_ERR("handleGetSite1Name function not implemented");
}

void MeadeLX200Parser::handleGetSite2Name() {
    LOG_ERR("handleGetSite2Name function not implemented");
}

void MeadeLX200Parser::handleGetSite3Name() {
    LOG_ERR("handleGetSite3Name function not implemented");
}

void MeadeLX200Parser::handleGetSite4Name() {
    LOG_ERR("handleGetSite4Name function not implemented");
}

// Add similar default implementations for other virtual functions
void MeadeLX200Parser::handleInitializeTelescope() {
    LOG_ERR("handleInitializeTelescope function not implemented");
}

void MeadeLX200Parser::handleAutomaticAlignment() {
    LOG_ERR("handleAutomaticAlignment function not implemented");
}

void MeadeLX200Parser::handleLandAlignmentMode() {
    LOG_ERR("handleLandAlignmentMode function not implemented");
}

void MeadeLX200Parser::handlePolarAlignmentMode() {
    LOG_ERR("handlePolarAlignmentMode function not implemented");
}

void MeadeLX200Parser::handleAltAzAlignmentMode() {
    LOG_ERR("handleAltAzAlignmentMode function not implemented");
}

void MeadeLX200Parser::handleSetAltitudeDecAntibacklash() {
    LOG_ERR("handleSetAltitudeDecAntibacklash function not implemented");
}

void MeadeLX200Parser::handleSetAzimuthRAAntibacklash() {
    LOG_ERR("handleSetAzimuthRAAntibacklash function not implemented");
}

void MeadeLX200Parser::handleIncreaseReticuleBrightness() {
    LOG_ERR("handleIncreaseReticuleBrightness function not implemented");
}

void MeadeLX200Parser::handleDecreaseReticuleBrightness() {
    LOG_ERR("handleDecreaseReticuleBrightness function not implemented");
}

void MeadeLX200Parser::handleSetReticleFlashRate(int n) {
    LOG_ERR("handleSetReticleFlashRate function not implemented");
}

void MeadeLX200Parser::handleSetReticuleDutyCycle(int n) {
    LOG_ERR("handleSetReticuleDutyCycle function not implemented");
}

void MeadeLX200Parser::handleSynchronizeSelenographicCoordinates() {
    LOG_ERR("handleSynchronizeSelenographicCoordinates function not implemented");
}

void MeadeLX200Parser::handleSynchronizeDatabaseCoordinates() {
    LOG_ERR("handleSynchronizeDatabaseCoordinates function not implemented");
}

void MeadeLX200Parser::handleRequestDistanceBars() {
    LOG_ERR("handleRequestDistanceBars function not implemented");
}

void MeadeLX200Parser::handleTurnOnFan() {
    LOG_ERR("handleTurnOnFan function not implemented");
}

void MeadeLX200Parser::handleTurnOffFan() {
    LOG_ERR("handleTurnOffFan function not implemented");
}

void MeadeLX200Parser::handleReturnOTATemperature() {
    LOG_ERR("handleReturnOTATemperature function not implemented");
}

void MeadeLX200Parser::handleStartFocuserInward() {
    LOG_ERR("handleStartFocuserInward function not implemented");
}

void MeadeLX200Parser::handleStartFocuserOutward() {
    LOG_ERR("handleStartFocuserOutward function not implemented");
}

void MeadeLX200Parser::handleHaltFocuserMotion() {
    LOG_ERR("handleHaltFocuserMotion function not implemented");
}

void MeadeLX200Parser::handleSetFocusSpeedFastest() {
    LOG_ERR("handleSetFocusSpeedFastest function not implemented");
}

void MeadeLX200Parser::handleSetFocusSpeedSlowest() {
    LOG_ERR("handleSetFocusSpeedSlowest function not implemented");
}

void MeadeLX200Parser::handleSetFocuserSpeed(int n) {
    LOG_ERR("handleSetFocuserSpeed function not implemented");
}

void MeadeLX200Parser::handleTurnOnGPS() {
    LOG_ERR("handleTurnOnGPS function not implemented");
}

void MeadeLX200Parser::handleTurnOffGPS() {
    LOG_ERR("handleTurnOffGPS function not implemented");
}

void MeadeLX200Parser::handleTurnOnNMEAGPSDataStream() {
    LOG_ERR("handleTurnOnNMEAGPSDataStream function not implemented");
}

void MeadeLX200Parser::handleUpdateSystemTimeFromGPS() {
    LOG_ERR("handleUpdateSystemTimeFromGPS function not implemented");
}

void MeadeLX200Parser::handleGetAlignmentMenuEntry0() {
    LOG_ERR("handleGetAlignmentMenuEntry0 function not implemented");
}

void MeadeLX200Parser::handleGetAlignmentMenuEntry1() {
    LOG_ERR("handleGetAlignmentMenuEntry1 function not implemented");
}

void MeadeLX200Parser::handleGetAlignmentMenuEntry2() {
    LOG_ERR("handleGetAlignmentMenuEntry2 function not implemented");
}

void MeadeLX200Parser::handleGetTelescopeAltitude() {
    LOG_ERR("handleGetTelescopeAltitude function not implemented");
}

void MeadeLX200Parser::handleGetLocalTelescopeTime12Hour() {
    LOG_ERR("handleGetLocalTelescopeTime12Hour function not implemented");
}

void MeadeLX200Parser::handleGetBrowseBrighterMagnitudeLimit() {
    LOG_ERR("handleGetBrowseBrighterMagnitudeLimit function not implemented");
}

void MeadeLX200Parser::handleGetCurrentDate() {
    LOG_ERR("handleGetCurrentDate function not implemented");
}

void MeadeLX200Parser::handleGetCalendarFormat() {
    LOG_ERR("handleGetCalendarFormat function not implemented");
}

void MeadeLX200Parser::handleGetTelescopeDeclination() {
    LOG_ERR("handleGetTelescopeDeclination function not implemented");
}

void MeadeLX200Parser::handleGetCurrentTargetDeclination() {
    LOG_ERR("handleGetCurrentTargetDeclination function not implemented");
}

void MeadeLX200Parser::handleGetFindFieldDiameter() {
    LOG_ERR("handleGetFindFieldDiameter function not implemented");
}

void MeadeLX200Parser::handleGetBrowseFaintMagnitudeLimit() {
    LOG_ERR("handleGetBrowseFaintMagnitudeLimit function not implemented");
}

void MeadeLX200Parser::handleGetUTCOffsetTime() {
    LOG_ERR("handleGetUTCOffsetTime function not implemented");
}

void MeadeLX200Parser::handleGetCurrentSiteLongitude() {
    LOG_ERR("handleGetCurrentSiteLongitude function not implemented");
}

void MeadeLX200Parser::handleGetHighLimit() {
    LOG_ERR("handleGetHighLimit function not implemented");
}

void MeadeLX200Parser::handleGetLowerLimit() {
    LOG_ERR("handleGetLowerLimit function not implemented");
}

void MeadeLX200Parser::handleGetMinimumQualityForFind() {
    LOG_ERR("handleGetMinimumQualityForFind function not implemented");
}

void MeadeLX200Parser::handleGetTelescopeRA() {
    LOG_ERR("handleGetTelescopeRA function not implemented");
}

void MeadeLX200Parser::handleGetCurrentTargetRA() {
    LOG_ERR("handleGetCurrentTargetRA function not implemented");
}

void MeadeLX200Parser::handleGetSiderealTime() {
    LOG_ERR("handleGetSiderealTime function not implemented");
}

void MeadeLX200Parser::handleGetSmallerSizeLimit() {
    LOG_ERR("handleGetSmallerSizeLimit function not implemented");
}

void MeadeLX200Parser::handleGetTrackingRate() {
    LOG_ERR("handleGetTrackingRate function not implemented");
}

void MeadeLX200Parser::handleGetCurrentSiteLatitude() {
    LOG_ERR("handleGetCurrentSiteLatitude function not implemented");
}

void MeadeLX200Parser::handleGetFirmwareDate() {
    LOG_ERR("handleGetFirmwareDate function not implemented");
}

void MeadeLX200Parser::handleGetFirmwareNumber() {
    LOG_ERR("handleGetFirmwareNumber function not implemented");
}

void MeadeLX200Parser::handleGetProductName() {
    LOG_ERR("handleGetProductName function not implemented");
}

void MeadeLX200Parser::handleGetFirmwareTime() {
    LOG_ERR("handleGetFirmwareTime function not implemented");
}

void MeadeLX200Parser::handleGetDeepSkyObjectSearchString() {
    LOG_ERR("handleGetDeepSkyObjectSearchString function not implemented");
}

void MeadeLX200Parser::handleGetTelescopeAzimuth() {
    LOG_ERR("handleGetTelescopeAzimuth function not implemented");
}

void MeadeLX200Parser::handleSeekHomePosition() {
    LOG_ERR("handleSeekHomePosition function not implemented");
}

void MeadeLX200Parser::handleSeekAndAlignHomePosition() {
    LOG_ERR("handleSeekAndAlignHomePosition function not implemented");
}

void MeadeLX200Parser::handleSleepTelescope() {
    LOG_ERR("handleSleepTelescope function not implemented");
}

void MeadeLX200Parser::handleSlewToParkPosition() {
    LOG_ERR("handleSlewToParkPosition function not implemented");
}

void MeadeLX200Parser::handleWakeUpTelescope() {
    LOG_ERR("handleWakeUpTelescope function not implemented");
}

void MeadeLX200Parser::handleQueryHomeStatus() {
    LOG_ERR("handleQueryHomeStatus function not implemented");
}

void MeadeLX200Parser::handleToggleTimeFormat() {
    LOG_ERR("handleToggleTimeFormat function not implemented");
}

void MeadeLX200Parser::handleFindPreviousObject() {
    LOG_ERR("handleFindPreviousObject function not implemented");
}

void MeadeLX200Parser::handleSetTargetObjectDeepSky(int n) {
    LOG_ERR("handleSetTargetObjectDeepSky function not implemented");
}

void MeadeLX200Parser::handleFindObjectWithConstraints() {
    LOG_ERR("handleFindObjectWithConstraints function not implemented");
}

void MeadeLX200Parser::handleIdentifyObjectInField() {
    LOG_ERR("handleIdentifyObjectInField function not implemented");
}

void MeadeLX200Parser::handleGetObjectInformation() {
    LOG_ERR("handleGetObjectInformation function not implemented");
}

void MeadeLX200Parser::handleSetTargetObjectMessier(int n) {
    LOG_ERR("handleSetTargetObjectMessier function not implemented");
}

void MeadeLX200Parser::handleFindNextDeepSkyObject() {
    LOG_ERR("handleFindNextDeepSkyObject function not implemented");
}

void MeadeLX200Parser::handleSelectDeepSkyLibrary(int d) {
    LOG_ERR("handleSelectDeepSkyLibrary function not implemented");
}

void MeadeLX200Parser::handleSelectStarCatalog(int d) {
    LOG_ERR("handleSelectStarCatalog function not implemented");
}

void MeadeLX200Parser::handleSelectStar(int n) {
    LOG_ERR("handleSelectStar function not implemented");
}

void MeadeLX200Parser::handleSlewToAltAz() {
    LOG_ERR("handleSlewToAltAz function not implemented");
}

void MeadeLX200Parser::handleMoveEast() {
    LOG_ERR("handleMoveEast function not implemented");
}

void MeadeLX200Parser::handleMoveNorth() {
    LOG_ERR("handleMoveNorth function not implemented");
}

void MeadeLX200Parser::handleMoveSouth() {
    LOG_ERR("handleMoveSouth function not implemented");
}

void MeadeLX200Parser::handleMoveWest() {
    LOG_ERR("handleMoveWest function not implemented");
}

void MeadeLX200Parser::handleSlewToTargetObject() {
    LOG_ERR("handleSlewToTargetObject function not implemented");
}

void MeadeLX200Parser::handleToggleHighPrecision() {
    LOG_ERR("handleToggleHighPrecision function not implemented");
}

void MeadeLX200Parser::handleToggleSmartDrive() {
    LOG_ERR("handleToggleSmartDrive function not implemented");
}

void MeadeLX200Parser::handleEnableDecAltPEC() {
    LOG_ERR("handleEnableDecAltPEC function not implemented");
}

void MeadeLX200Parser::handleDisableDecAltPEC() {
    LOG_ERR("handleDisableDecAltPEC function not implemented");
}

void MeadeLX200Parser::handleEnableRAAzPEC() {
    LOG_ERR("handleEnableRAAzPEC function not implemented");
}

void MeadeLX200Parser::handleDisableRAAzPEC() {
    LOG_ERR("handleDisableRAAzPEC function not implemented");
}

void MeadeLX200Parser::handleHaltAllSlewing() {
    LOG_ERR("handleHaltAllSlewing function not implemented");
}

void MeadeLX200Parser::handleHaltEastwardSlews() {
    LOG_ERR("handleHaltEastwardSlews function not implemented");
}

void MeadeLX200Parser::handleHaltNorthwardSlews() {
    LOG_ERR("handleHaltNorthwardSlews function not implemented");
}

void MeadeLX200Parser::handleHaltSouthwardSlews() {
    LOG_ERR("handleHaltSouthwardSlews function not implemented");
}

void MeadeLX200Parser::handleHaltWestwardSlews() {
    LOG_ERR("handleHaltWestwardSlews function not implemented");
}

void MeadeLX200Parser::handleTurnOnFieldDerotator() {
    LOG_ERR("handleTurnOnFieldDerotator function not implemented");
}

void MeadeLX200Parser::handleTurnOffFieldDerotator() {
    LOG_ERR("handleTurnOffFieldDerotator function not implemented");
}

void MeadeLX200Parser::handleSetSlewRateCentering() {
    LOG_ERR("handleSetSlewRateCentering function not implemented");
}

void MeadeLX200Parser::handleSetSlewRateGuiding() {
    LOG_ERR("handleSetSlewRateGuiding function not implemented");
}

void MeadeLX200Parser::handleSetSlewRateFind() {
    LOG_ERR("handleSetSlewRateFind function not implemented");
}

void MeadeLX200Parser::handleSetSlewRateMax() {
    LOG_ERR("handleSetSlewRateMax function not implemented");
}

void MeadeLX200Parser::handleSetRAAzimuthSlewRate(double dd) {
    LOG_ERR("handleSetRAAzimuthSlewRate function not implemented");
}

void MeadeLX200Parser::handleSetDecElevationSlewRate(double dd) {
    LOG_ERR("handleSetDecElevationSlewRate function not implemented");
}

void MeadeLX200Parser::handleSetGuideRate(double ss) {
    LOG_ERR("handleSetGuideRate function not implemented");
}

void MeadeLX200Parser::handleSetTargetAltitude(int s, int dd, int mm) {
    LOG_ERR("handleSetTargetAltitude function not implemented");
}

void MeadeLX200Parser::handleSetBrighterLimit(double mm) {
    LOG_ERR("handleSetBrighterLimit function not implemented");
}

void MeadeLX200Parser::handleSetBaudRate(int n) {
    LOG_ERR("handleSetBaudRate function not implemented");
}

void MeadeLX200Parser::handleChangeHandboxDate(int mm, int dd, int yy) {
    LOG_ERR("handleChangeHandboxDate function not implemented");
}

void MeadeLX200Parser::handleSetTargetDeclination(int s, int dd, int mm) {
    LOG_ERR("handleSetTargetDeclination function not implemented");
}