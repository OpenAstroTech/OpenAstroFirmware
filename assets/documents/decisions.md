# Pending Decisions

## Hardware
- Do we have a default board?
- What steppers/drivers are we supporting? Same as before?
- Do we want to support a hand controller of some sort? I2C via Network cable? Serial via USB cable?

## Software
- How do we support hardware variations? #defines like before, or include everything?
- What do we use for the menu system? Needs UX designer input or we use an industry standard.
- What subsystems can/should run on their own threads with minimal synchronization issues.
- Are we supporting the full LX200 command set from OAT/OAM V1?
 
# Made Decisions

## Hardware
- 32bit boards only, no 8-bit board support.
- At least 4GB memory
- I2C required