/**
 * @file
 * In this file, the implementation of the Controller class is defined.
 */

#include "Controller.h"
#include <cstdlib>
#include <memory>
#include <utility>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <thread>

#include "StubController.h"
#include "ValueMap.h"
#include "../optimizer/direct/DirectOptimizer.h"
#include "../runner/plexe/PlexeSimulationRunner.h"
#include "../evaluation/constant_headway/ConstantHeadway.h"
#include "nlohmann/json.hpp"

#include <functional>
#include "../ReportGenerator.h"
/**
 * Helper method parsing a json object from the given file.
 * @param baseDir: Directory the json file resides in.
 * @param config: Name of the json file.
 * @return A json object parsed from the given file.
 */
nlohmann::json getConfigByPath(std::filesystem::path baseDir, const std::string &config) {
    std::filesystem::path configPath = std::move(baseDir);
    configPath.append(config);
    std::ifstream configStream(configPath);
    nlohmann::json result = nlohmann::json::parse(configStream, nullptr, true, true);
    configStream.close();
    return result;
}

Controller::Controller(const std::filesystem::path &configPath, bool isStub) {
    std::cout << "Initializing optimization...";
    std::cout.flush();

    // Controller settings
    nlohmann::json baseConfig = getConfigByPath(configPath.parent_path(), configPath.filename().string());
    keepFiles = baseConfig.at("controller").at("keepResultFiles").get<bool>();
    unsigned int nrTopEntries = baseConfig.at("controller").at("nrTopEntries").get<unsigned int>();
    valueMap = std::make_unique<ValueMap>(nrTopEntries);
    double seconds = baseConfig.at("controller").at("updateInterval").get<double>();
    statusInterval = std::chrono::milliseconds((long) round(seconds * 1000));

    // Read Parameters
    nlohmann::json paramJson = getConfigByPath(configPath.parent_path(),
                                               baseConfig.at("controller").at("params").get<std::string>());
    std::list<std::shared_ptr<ParameterDefinition>> params;
    for (auto param: paramJson) {
        coordinate min = param.at("min").get<coordinate>();
        coordinate max = param.at("max").get<coordinate>();
        std::string config;
        if (param.contains("config")) {
            config = param.at("config").get<std::string>();
        }
        std::string unit;
        if (param.contains("unit")) {
            unit = param.at("unit").get<std::string>();
        }
        params.push_back(std::make_shared<ParameterDefinition>(min, max, config, unit));
    }

    // Optimizer settings
    nlohmann::json optimizerConfig = getConfigByPath(configPath.parent_path(),
                                                     baseConfig.at("optimizer").at("config").get<std::string>());
    std::string opt = baseConfig.at("optimizer").at("optimizer").get<std::string>();
    if (opt == "Direct") {
        bool trackProgress = optimizerConfig.at("output").at("progress");
        bool printValues = optimizerConfig.at("output").at("values");
        optimizer = std::unique_ptr<Optimizer>(
                new DirectOptimizer(*this, params, StoppingCondition(optimizerConfig.at("stopCon")), trackProgress,
                                    printValues));
    } else {
        throw std::runtime_error("Optimzer not found: " + opt);
    }

    // If StubController is used, initialization of SimulationRunner and Evaluation may be skipped
    if (isStub) {
        runner = std::unique_ptr<SimulationRunner>(nullptr);
        evaluation = std::unique_ptr<Evaluation>(nullptr);
        std::cout << "done\n" << std::endl;
        return;
    }

    // SimulationRunner settings
    nlohmann::json runnerConfig = getConfigByPath(configPath.parent_path(),
                                                  baseConfig.at("runner").at("config").get<std::string>());
    std::string run = baseConfig.at("runner").at("runner").get<std::string>();
    if (run == "Plexe") {
        unsigned int nrThreads = runnerConfig.at("nrThreads").get<unsigned int>();
        unsigned int repeat = runnerConfig.at("repeat").get<unsigned int>();

        std::vector<std::string> scenarios(runnerConfig.at("scenarios").size());
        for (int i = 0; i < scenarios.size(); ++i) {
            scenarios[i] = runnerConfig.at("scenarios").at(i).get<std::string>();
        }

        std::string dir = runnerConfig.at("configDirectory").get<std::string>();
        runner = std::unique_ptr<SimulationRunner>(new PlexeSimulationRunner(nrThreads, repeat, scenarios,
                                                                             ConfigEditor(dir, runnerConfig.at(
                                                                                     "controller"))));
    } else {
        throw std::runtime_error("SimulationRunner not found: " + run);
    }

    // Evaluation settings
    nlohmann::json evalConfig = getConfigByPath(configPath.parent_path(),
                                                baseConfig.at("evaluation").at("config").get<std::string>());
    std::string eval = baseConfig.at("evaluation").at("evaluation").get<std::string>();
    if (eval == "ConstantHeadway") {
        std::string omnetppDirectory = evalConfig.at("omnetppDirectory").get<std::string>();
        setenv("OMNETPP_HOME", omnetppDirectory.c_str(), true);

        unsigned int nrThreads = evalConfig.at("nrThreads").get<unsigned int>();
        std::filesystem::path scriptPath(evalConfig.at("pythonScript").get<std::string>());
        evaluation = std::unique_ptr<Evaluation>(new ConstantHeadway(nrThreads, scriptPath));
    } else {
        throw std::runtime_error("SimulationRunner not found: " + run);
    }

    std::cout << "done\n" << std::endl;
}

Controller::Controller(const std::filesystem::path &configPath, std::function<void(std::string)> callback,std::function<void(std::string)> dataCallback, bool isStub) {
    mUpdateStatusCallback = callback;
    mDataSendCallback = dataCallback;
    mStatus = ProcessState::UNINITILAIZED;
    sendStatus("Initializing optimization...");

    // Controller settings
    nlohmann::json baseConfig = getConfigByPath(configPath.parent_path(), configPath.filename().string());
    keepFiles = baseConfig.at("controller").at("keepResultFiles").get<bool>();
    unsigned int nrTopEntries = baseConfig.at("controller").at("nrTopEntries").get<unsigned int>();
    valueMap = std::make_unique<ValueMap>(nrTopEntries);
    double seconds = baseConfig.at("controller").at("updateInterval").get<double>();
    statusInterval = std::chrono::milliseconds((long)round(seconds * 1000));

    // Read Parameters
    nlohmann::json paramJson = getConfigByPath(configPath.parent_path(),
                                               baseConfig.at("controller").at("params").get<std::string>());
    std::list<std::shared_ptr<ParameterDefinition>> params;
    for (auto param : paramJson) {
        coordinate min = param.at("min").get<coordinate>();
        coordinate max = param.at("max").get<coordinate>();
        std::string config;
        if (param.contains("config")) {
            config = param.at("config").get<std::string>();
        }
        std::string unit;
        if (param.contains("unit")) {
            unit = param.at("unit").get<std::string>();
        }
        params.push_back(std::make_shared<ParameterDefinition>(min, max, config, unit));
    }

    // Optimizer settings
    nlohmann::json optimizerConfig = getConfigByPath(configPath.parent_path(),
                                                     baseConfig.at("optimizer").at("config").get<std::string>());
    std::string opt = baseConfig.at("optimizer").at("optimizer").get<std::string>();
    if (opt == "Direct") {
        bool trackProgress = optimizerConfig.at("output").at("progress");
        bool printValues = optimizerConfig.at("output").at("values");
        optimizer = std::unique_ptr<Optimizer>(
            new DirectOptimizer(*this, params, StoppingCondition(optimizerConfig.at("stopCon")), trackProgress,
                                printValues));
    } else {
        sendError("Optimzer not found: " + opt);
        mStatus = ProcessState::ERROR;
        return;
    }

    // If StubController is used, initialization of SimulationRunner and Evaluation may be skipped
    if (isStub) {
        runner = std::unique_ptr<SimulationRunner>(nullptr);
        evaluation = std::unique_ptr<Evaluation>(nullptr);
        mStatus = ProcessState::READY;
        sendStatus("Initalization done");
        return;
    }

    // SimulationRunner settings
    nlohmann::json runnerConfig = getConfigByPath(configPath.parent_path(),
                                                  baseConfig.at("runner").at("config").get<std::string>());
    std::string run = baseConfig.at("runner").at("runner").get<std::string>();
    if (run == "Plexe") {
        unsigned int nrThreads = runnerConfig.at("nrThreads").get<unsigned int>();
        unsigned int repeat = runnerConfig.at("repeat").get<unsigned int>();

        std::vector<std::string> scenarios(runnerConfig.at("scenarios").size());
        for (int i = 0; i < scenarios.size(); ++i) {
            scenarios[i] = runnerConfig.at("scenarios").at(i).get<std::string>();
        }

        std::string dir = runnerConfig.at("configDirectory").get<std::string>();
        runner = std::unique_ptr<SimulationRunner>(
            new PlexeSimulationRunner(nrThreads, repeat, scenarios, ConfigEditor(dir, runnerConfig.at("controller"))));
    } else {
        sendError("SimulationRunner not found: " + run);
        mStatus = ProcessState::ERROR;
        return;
    }

    // Evaluation settings
    nlohmann::json evalConfig = getConfigByPath(configPath.parent_path(),
                                                baseConfig.at("evaluation").at("config").get<std::string>());
    std::string eval = baseConfig.at("evaluation").at("evaluation").get<std::string>();
    if (eval == "ConstantHeadway") {
        std::string omnetppDirectory = evalConfig.at("omnetppDirectory").get<std::string>();
        setenv("OMNETPP_HOME", omnetppDirectory.c_str(), true);

        unsigned int nrThreads = evalConfig.at("nrThreads").get<unsigned int>();
        std::filesystem::path scriptPath(evalConfig.at("pythonScript").get<std::string>());
        evaluation = std::unique_ptr<Evaluation>(new ConstantHeadway(nrThreads, scriptPath));
    } else {
        sendError("Evaluation not found: " + eval);
        mStatus = ProcessState::ERROR;
        return;
    }

    mStatus = ProcessState::READY;
    sendStatus("Initalization done");
}

nlohmann::json Controller::packData(const std::string &type, const nlohmann::json &data) {
    nlohmann::json rootObj;
    rootObj["type"] = type;
    rootObj["data"] = data;
    return rootObj;
}

ProcessState Controller::getStatus(){
    return mStatus;
}

std::map<parameterCombination, functionValue> Controller::requestValues(const std::list<parameterCombination> &params) {
    updateStatus();
    std::map<parameterCombination, functionValue> result;
    std::set<parameterCombination, CmpVectorSharedParameter> simRuns;
    for (const auto &cords: params) {
        if (valueMap->isKnown(cords)) {
            result.insert(make_pair(cords, valueMap->query(cords)));
        } else {
            simRuns.insert(cords);
        }
    }

    if (!simRuns.empty()) {
        stepState.next();
        auto vecToResult = runSimulations(simRuns);
        stepState.next();
        auto simResults = evaluate(vecToResult);
        stepState.next();

        for (const auto &entry: simResults) {
            if (!aborted) {
                valueMap->insert(entry.first, entry.second);
            }
            topResults.insert(make_pair(entry.first, vecToResult[entry.first].first));
        }

        removeOldResultfiles();

        result.merge(simResults);
    }
    updateStatus();
    return result;
}

ValueMap &Controller::getValueMap() {
    return *valueMap;
}

void Controller::send(const std::string &jsonSendObject) {
    if (mUpdateStatusCallback && jsonSendObject != "") {
        mUpdateStatusCallback(jsonSendObject);
    } else {
        std::cout << "Not initalized callback or empty data" << std::endl;
    }
}

void Controller::sendBinary(const std::filesystem::path& path){
            std::ifstream file(path, std::ios::binary);
        if (file.is_open()) {
            std::stringstream content_stream;
            char byte;
            while (file.get(byte)) {
                content_stream << byte;
            }
            std::string content = content_stream.str();
            mDataSendCallback(content);
            file.close();
        } else {
            std::cerr << "Error: Unable to open file " << path << std::endl;
        }
}

void Controller::sendData(const nlohmann::json &dataMessage) {
    auto dataPackage = packData("SimulationData", dataMessage);
    send(dataPackage.dump());
}

void Controller::sendError(const std::string &errorMessage) {
    nlohmann::json messageData;
    messageData["code"] = 400;
    messageData["text"] = errorMessage;
    messageData = packData("ServerMessage", messageData);
    send(messageData.dump());
}

void Controller::sendStatus(const std::string& message){
        nlohmann::json messageData;
        messageData["code"] = 200;
        messageData["text"] = message;
        messageData = packData("ServerMessage",messageData);
        send(messageData.dump());
    }

void Controller::run() {
    if (mStatus == ProcessState::READY) {
        try
        {
                    mStatus = ProcessState::RUNNING;
        sendStatus("Processing...");
        auto runStatusUpdate = [this]() {
            while (statusInterval != std::chrono::milliseconds(0)) {
                updateStatus();
                std::this_thread::sleep_for(statusInterval);
            }
        };
        stepState.next();
        updateStatus();
        std::thread statusThread(runStatusUpdate);
        optimizer->runOptimization();
        statusInterval = std::chrono::milliseconds(0);
        statusThread.join();
        std::list<std::pair<parameterCombination, std::pair<functionValue, std::filesystem::path>>> top;
        ReportGenerator reportGenerator;
        std::vector<std::filesystem::path> toZIP; // Folders to zip

        for (const auto &entry : valueMap->getTopVals()) {
            top.emplace_back(entry.first, std::make_pair(entry.second, topResults[entry.first]));
            toZIP.push_back(topResults[entry.first]);
        }

        auto zipFiles = reportGenerator.generateReports(toZIP);

        sendStatus("Sending Results to Client...");
        for (const auto &file: zipFiles){
            sendBinary(file);
        }
        auto data = packData("ResultOverview",StatusBar::getResults(top));
        send(data.dump());
        mStatus = ProcessState::FINISH;
        sendStatus("Optimization done...");
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            sendError("Simulation crashed");
            mStatus = ProcessState::ERROR;
        }

    }
}

std::map<parameterCombination, std::pair<std::filesystem::path, std::set<runId>>, CmpVectorSharedParameter>
Controller::runSimulations(const std::set<parameterCombination, CmpVectorSharedParameter> &runs) {
    updateStatus();
    auto res = runner->runSimulations(runs);
    updateStatus();
    return res;
}

std::map<parameterCombination, functionValue, CmpVectorSharedParameter> Controller::evaluate(
        const std::map<parameterCombination, std::pair<std::filesystem::path, std::set<runId>>, CmpVectorSharedParameter> &simulationResults) {
    updateStatus();
    std::set<std::pair<std::filesystem::path, std::set<runId>>> resultFiles;
    for (const auto &entry: simulationResults) {
        resultFiles.insert(entry.second);
    }
    auto evaluationResult = evaluation->processOutput(resultFiles);
    std::map<parameterCombination, functionValue, CmpVectorSharedParameter> result;
    for (const auto &entry: simulationResults) {
        result.insert(make_pair(entry.first, evaluationResult[entry.second]));
    }
    updateStatus();
    return result;
}

void Controller::removeOldResultfiles() {
    if (!(keepFiles || topResults.empty())) {
        std::filesystem::path resultDir = topResults.begin()->second.parent_path();
        std::filesystem::remove_all(resultDir);
    } else if (topResults.size() > valueMap->getTopVals().size()) {
        std::list<parameterCombination> toBeRemoved;
        for (const auto &entry: topResults) {
            if (!valueMap->isTopValue(entry.first)) {
                std::filesystem::remove_all(entry.second);
                toBeRemoved.push_back(entry.first);
            }
        }
        for (const auto &entry: toBeRemoved) {
            topResults.erase(entry);
        }
    }
}

void Controller::updateStatus() {
    std::pair<parameterCombination, functionValue> p =
            valueMap->getSize() != 0 ? valueMap->getTopVals().front() : make_pair(parameterCombination(),
                                                                                  (functionValue) INFINITY);
    bool stateChanged = stepState.stepChanged;
    auto dataVector = statusBar.updateStatusValues(optimizer.get(), runner.get(), evaluation.get(), p, stateChanged, stepState.get());
    if (dataVector[0]!=nullptr){
        sendData(dataVector[0]);
    }
}

void Controller::abort() {
    sendStatus("Aborting...");
    Abortable::abort();
    optimizer->abort();
}
