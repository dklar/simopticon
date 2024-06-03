/**
 * @file
 * In this file, the implementation of the StatusBar class is defined.
 */

#include "StatusBar.h"

#include "../optimizer/Optimizer.h"

#include <iostream>


const std::string StatusBar::LARGE_DIVIDER = "\n\n" + std::string(70, '#') + "\n";

const std::string StatusBar::SMALL_DIVIDER = std::string(70, '-') + "\n";

std::vector<nlohmann::json> StatusBar::updateStatusValues(Status *opt, Status *runner, Status *eval,
                             const std::pair<parameterCombination, functionValue> &currentVal, bool stepChanged,
                             step currentStep) {
    
    std::vector<nlohmann::json> result(4);
    currentStep = stepChanged ? currentStep : lastStep;
    if (stepChanged || currentVal != lastVal) {
        lastStatus = "";
        result[0] = getResult(currentVal.first, currentVal.second);
        result[1] = getStatus(opt);
        result[2] = getStatus(runner);
        result[3] = getStatus(eval);
    } else {
        result[0] = nullptr;
        result[1] = nullptr;
        result[2] = nullptr;
        result[3] = nullptr;
    }

    switch (currentStep) {
        default:
        case OPTIMIZER:
            lastStatus = opt->getStatusBar();
            break;
        case RUNNER:
            lastStatus = runner->getStatusBar();
            break;
        case EVALUATION:
            lastStatus = eval->getStatusBar();
            break;
    }
    lastVal = currentVal;
    lastStep = currentStep;                           

    return result;
}
void StatusBar::updateStatus(Status *opt, Status *runner, Status *eval,
                             const std::pair<parameterCombination, functionValue> &currentVal, bool stepChanged,
                             step currentStep) {
    currentStep = stepChanged ? currentStep : lastStep;
    if (stepChanged || currentVal != lastVal) {
        std::cout << LARGE_DIVIDER;
        std::cout << "Current optimum:\n";
        printResult(currentVal.first, currentVal.second);
        std::cout << SMALL_DIVIDER;
        printStatus(opt);
        std::cout << SMALL_DIVIDER;
        printStatus(runner);
        std::cout << SMALL_DIVIDER;
        printStatus(eval);
        std::cout << SMALL_DIVIDER;
        std::cout << "Status: ";
        lastStatus = "";
    }
    for (int i = 0; i < lastStatus.size(); ++i) {
        std::cout << "\b \b";
    }
    switch (currentStep) {
        default:
        case OPTIMIZER:
            lastStatus = opt->getStatusBar();
            break;
        case RUNNER:
            lastStatus = runner->getStatusBar();
            break;
        case EVALUATION:
            lastStatus = eval->getStatusBar();
            break;
    }
    lastVal = currentVal;
    lastStep = currentStep;
    std::cout << lastStatus;
    std::cout.flush();
}

void StatusBar::printResult(const parameterCombination &cords, functionValue optimum) {
    unsigned int i = 1;
    for (const auto &param: cords) {
        if (param->getConfig().empty()) {
            std::cout << "x" << i;
        } else {
            std::cout << param->getConfig();
        }
        std::cout << ":\t" << param->getVal() << param->getUnit() << "\n";
        i++;
    }
    std::cout << "Value: " << optimum << "\n";
}

void StatusBar::printStatus(Status *object) {
    std::cout << object->getName() << "\n";
    std::cout << object->getStatus() << "\n";
}

nlohmann::json StatusBar::getStatus(Status *object){
    nlohmann::json rootObj;
    rootObj["name"] = object->getName();
    rootObj["status"] = object->getStatus();
    return rootObj;
}

nlohmann::json StatusBar::getResult(const parameterCombination &cords, functionValue optimum) {
    nlohmann::json rootObj;
    for (const auto &param: cords) {
        nlohmann::json paramJson;
        if (param->getConfig().empty()) {
            paramJson["name"] = "x";
        } else {
            paramJson["name"] = param->getConfig();
        }
        paramJson["value"] = param->getVal();
        paramJson["unit"] = param->getUnit();
        rootObj["parameters"].push_back(paramJson);
    }
    rootObj["value"] = optimum;
    return rootObj;
}

nlohmann::json StatusBar::getResults(
    std::list<std::pair<parameterCombination, std::pair<functionValue, std::filesystem::path>>> top) {
    nlohmann::json rootObj;
    size_t i = top.size();
    for (auto it = top.rbegin(); it != top.rend(); ++it) {
        nlohmann::json run;
        run["rank"] = i--;
        run["path"] = it->second.second;
        run["value"] = it->second.first;

        nlohmann::json parameters;
        auto params = getResult(it->first, it->second.first);
        for (const auto &param : params["parameters"]) {
            parameters.push_back(param);
        }
        run["parameters"] = parameters;
        rootObj["run"].push_back(run);
    }
    return rootObj;
}

void StatusBar::printResults(
        std::list<std::pair<parameterCombination, std::pair<functionValue, std::filesystem::path>>> top) {
    std::cout << LARGE_DIVIDER;
    size_t i = top.size();
    for (auto it = top.rbegin(); it != top.rend(); ++it) {
        std::cout << i-- << ". Result\n";
        printResult(it->first, it->second.first);
        std::cout << "Path to result files: " << it->second.second << "\n";
        std::cout << SMALL_DIVIDER;
    }
    for (const auto &result: top) {
        std::cout << ++i << ".\t" << result.second.first << "\n";
    }
    std::cout.flush();
}
