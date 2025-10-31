
int change_password(int nsd, char *userType, int userId){
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;

    // Prompt for New Password
    strcpy(writeBuffer, "Enter New Password: ");
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
        return 0;
    }

    // Read New Password
    memset(readBuffer, 0, BUFF_SIZE);
    readBytes = read(nsd, readBuffer, sizeof(readBuffer));
    if(readBytes < 0){
        perror("Read from client failed");
        return 0;
    }
    readBuffer[readBytes] = '\0'; // Null-terminate
        
    readBuffer[strcspn(readBuffer, "\n")] = '\0'; // Remove newline
    if(strlen(readBuffer) == 0){
        return 0;
    }

    // Open appropriate database
    if(strcmp(userType, "EMPLOYEE") == 0){
        int fd = open(EMPLOYEES_DB, O_RDWR);
        if(fd < 0){
            perror("Failed to open database");
            return 0;
        }

        struct Employee emp;

        // Search for Employee
        lockFile(fd, F_WRLCK, 0, 0);
        while(read(fd, &emp, sizeof(emp)) == sizeof(emp)){
            if(emp.empID == userId){
                strncpy(emp.password, readBuffer, sizeof(emp.password));

                // Move file pointer back to overwrite
                lseek(fd, - sizeof(emp), SEEK_CUR);
                write(fd, &emp, sizeof(emp));
                break;
            }
        }
        unlockFile(fd, 0, 0);
        close(fd);
    }
    else{
        int fd = open(CUSTOMERS_DB, O_RDWR);
        if(fd < 0){
            perror("Failed to open database");
            return 0;
        }

        struct Customer cust;

        // Search for Customer
        lockFile(fd, F_WRLCK, 0, 0);
        while(read(fd, &cust, sizeof(cust)) == sizeof(cust)){
            if(cust.custId == userId){
                strncpy(cust.password, readBuffer, sizeof(cust.password));

                // Move file pointer back to overwrite
                lseek(fd, - sizeof(cust), SEEK_CUR);
                write(fd, &cust, sizeof(cust));
                break;
            }
        }
        unlockFile(fd, 0, 0);
        close(fd);
    }
    return 1;
}
