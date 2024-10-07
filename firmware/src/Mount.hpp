#ifndef MOUNT_H
#define MOUNT_H

class Mount {
private:
public:
    Mount();  // Constructor
    ~Mount(); // Destructor

    void initialize(); // Method to initialize the mount
    void moveTo(double azimuth, double altitude); // Method to move the mount
    void stop(); // Method to stop the mount
};

#endif // MOUNT_H