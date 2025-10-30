
int getNextCounterValue(const char* counterType){
    struct Counter counter;

    int cfd = open(COUNTERS_DB, O_RDWR);
    if(cfd < 0){
        perror("Failed to open counter database");
        return -1;
    }

    lockFile(cfd, F_WRLCK, 0, 0);

    int readBytes = read(cfd, &counter, sizeof(counter));
    int nextValue = -1;

    if(readBytes < 0){
        perror("Failed to read counter data");
        close(cfd);
        return -1;
    }
    else if(readBytes == 0){
        // Initialize counters if file is empty
        counter.lastCustId = 1000;
        counter.lastEmpId = 100;
        counter.lastAccNo = 10000;
        counter.lastLoanId = 1000;
    }
    
    if(strcmp(counterType, "custId") == 0){
        counter.lastCustId++;
        nextValue = counter.lastCustId;
    }
    else if(strcmp(counterType, "empId") == 0){
        counter.lastEmpId++;
        nextValue = counter.lastEmpId;
    }
    else if(strcmp(counterType, "accNo") == 0){
        counter.lastAccNo++;
        nextValue = counter.lastAccNo;
    }
    else if(strcmp(counterType, "loanId") == 0){
        counter.lastLoanId++;
        nextValue = counter.lastLoanId;
    }
    else if(strcmp(counterType, "txnId") == 0){
        counter.lastTxnId++;
        nextValue = counter.lastTxnId;
    }
    else{
        printf("Invalid counter type requested - %s\n", counterType);
        unlockFile(cfd, 0, 0);
        close(cfd);
        return -1;
    }

    // Write back updated counters
    lseek(cfd, 0, SEEK_SET);
    write(cfd, &counter, sizeof(counter));
    unlockFile(cfd, 0, 0);

    close(cfd);
    return nextValue;
}