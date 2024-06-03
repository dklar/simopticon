#ifndef STATUS_CONTROLLER_H
#define STATUS_CONTROLLER_H

#include "../simopticonLib/src/controller/Controller.h"

// enum class ProcessState {
//     UNINITILAIZED,
//     READY,
//     RUNNING,
//     FINISH
// };

class statusController
{
private:
//std::unique_ptr<Controller> mController;
    Controller *mController;
    ProcessState mStatus;
    std::function<void(std::string)> mUpdateStatusCallback;
public:
    statusController(const std::filesystem::path &configPath, bool isStub = false);
    statusController();
    ~statusController();

    void setController(const std::filesystem::path &configPath, bool isStub = false);
    void run();
    void abort();
    ProcessState getState();
    void packValues(std::string v);
};


statusController::statusController(const std::filesystem::path &configPath, bool isStub){
    mController = new Controller(configPath, isStub);
    mController->setUpdateStatusCallback(std::bind(&statusController::packValues, this, std::placeholders::_1));
    mStatus = ProcessState::READY;
}
statusController::statusController(): mStatus(ProcessState::UNINITILAIZED) {}

void statusController::packValues(std::string v){
    std::cout << "Callback called" << std::endl;
    std::cout << v << std::endl;
}

void statusController::run(){
    if (mStatus == ProcessState::READY ) {
        mStatus = ProcessState::RUNNING;
        mController->run();
        mStatus = ProcessState::FINISH;
    }
}

ProcessState statusController::getState(){
    return mStatus;
}

void statusController::setController(const std::filesystem::path &configPath, bool isStub) {
    if (mStatus == ProcessState::UNINITILAIZED ) {
        mController = new Controller(configPath, isStub);
        mStatus = ProcessState::READY;
    }
}

void statusController::abort(){
    if (mController){
        mController->abort();
    }
}

statusController::~statusController(){
    delete mController;
}


#endif
