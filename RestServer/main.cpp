#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <filesystem>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include "crow.h"
#include "simopticonLib/src/controller/Controller.h"

enum class ConnectionState {
    AuthenticatedConnection,
    Processing,
    Closed
};

namespace fs = std::filesystem;

crow::SimpleApp app;
std::unordered_set<std::string> connectionRequests;
std::unordered_map<crow::websocket::connection*, std::tuple<std::unique_ptr<Controller>, ConnectionState>> connections;

int maxConnections = 2;
int port = 18080;
std::mutex mtx;



void signalHandler(int signum) {
    std::cout << "Received SIGTERM signal. Exiting..." << std::endl;
    app.stop();
    exit(signum);
}

void handleMessage(const std::string& message, std::tuple<std::unique_ptr<Controller>, ConnectionState>& connectionData,crow::websocket::connection& conn) {
    std::unique_ptr<Controller>& controller = std::get<0>(connectionData);
    ConnectionState& state = std::get<1>(connectionData);
    switch (state) {
        case ConnectionState::AuthenticatedConnection:
            if (message == "start") {
                std::thread([&controller,&conn]() {
                    controller->run();
                }).detach();
                state = ConnectionState::Processing;
                std::cout << "Transitioning to Processing state\n";
            } else if (message == "close") {
                state = ConnectionState::Closed;
                std::cout << "Transitioning to Closed state\n";
            }
            break;
        case ConnectionState::Processing:
            if (message == "end") {
                controller->abort();
                state = ConnectionState::Closed;
            }
            break;
        case ConnectionState::Closed:
            std::cout << "Connection is closed, cannot handle message\n";
            break;
    }
}

void handleBinaryMessage(const std::string& message, std::tuple<std::unique_ptr<Controller>, ConnectionState>& connectionData,crow::websocket::connection& conn) {
    std::unique_ptr<Controller>& controller = std::get<0>(connectionData);
    ConnectionState& state = std::get<1>(connectionData);
    switch (state) {
        case ConnectionState::AuthenticatedConnection:
        //Not allowed
            break;
        case ConnectionState::Processing:

            break;
        case ConnectionState::Closed:

            break;
    }
}

crow::response saveProject(const crow::json::rvalue& json_data, const crow::request& req) {
    if (!json_data.has("name") || !json_data.has("mainFileData") || !json_data.has("parameterData") ||
        !json_data.has("optimizerData") || !json_data.has("runnerData") || !json_data.has("evalData")) {
        return crow::response(400, "Missing required fields");
    }
    // std::string projectName = json_data["name"].s();
    auto mainFileData = json_data["mainFileData"];
    auto parameterData = json_data["parameterData"];
    auto optimizerData = json_data["optimizerData"];
    auto runnerData = json_data["runnerData"];
    auto evalData = json_data["evalData"];

    std::string controllerFileName = mainFileData["controller"]["params"].s();
    std::string optimizerFileName = mainFileData["optimizer"]["config"].s();
    std::string runnerFileName = mainFileData["runner"]["config"].s();
    std::string evalFileName = mainFileData["evaluation"]["config"].s();

    std::string uuidStr = boost::uuids::to_string(boost::uuids::random_generator()());
    connectionRequests.insert(uuidStr);

    std::string tempDirPath = fs::temp_directory_path() / uuidStr;
    std::cout << tempDirPath << std::endl;
    if (fs::create_directories(tempDirPath)) {
        std::ofstream main_file(tempDirPath + "/" + "project.json");
        main_file << mainFileData;
        main_file.close();

        std::ofstream parameter_file(tempDirPath + "/" + controllerFileName);
        parameter_file << parameterData;
        parameter_file.close();

        std::ofstream optimizer_file(tempDirPath + "/" + optimizerFileName);
        optimizer_file << optimizerData;
        optimizer_file.close();

        std::ofstream runner_file(tempDirPath + "/" + runnerFileName);
        runner_file << runnerData;
        runner_file.close();

        std::ofstream eval_file(tempDirPath + "/" + evalFileName);
        eval_file << evalData;
        eval_file.close();

        return crow::response(201, uuidStr);
    } else {
        return crow::response(500, "Failed to create temporary directory");
    }
}

int main(int argc, char* argv[]) {
    signal(SIGTERM, signalHandler);

    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            port = std::atoi(argv[i + 1]);
            i++;
        } else if (std::strcmp(argv[i], "--maxConnections") == 0 && i + 1 < argc) {
            maxConnections = std::atoi(argv[i + 1]);
            i++;
        }
    }

    std::cout << std::system("pwd") << std::endl;


    CROW_ROUTE(app, "/closeProgramm").methods(crow::HTTPMethod::PUT)([](const crow::request& req) {
        for (auto& connection : connections) {
            auto& controllerPtr = std::get<0>(connection.second);
            if (controllerPtr) {
                controllerPtr->abort();
            }
        }
        app.stop();
        return crow::response(200, "Server stoped\n");
    });

    CROW_ROUTE(app, "/simulationStatus").methods(crow::HTTPMethod::GET)([](const crow::request& req) {
        return crow::response(200, "UNKNOWN\n");
    });

    CROW_WEBSOCKET_ROUTE(app, "/ws")
        .onopen([&](crow::websocket::connection& conn) {
            std::cout << "Hi to " << conn.get_remote_ip() << "\n";
        })
        .onclose([&](crow::websocket::connection& conn, const std::string& reason) {
            std::cout << "Bye from " << conn.get_remote_ip() << " because of " << reason << "\n";
            {
                std::lock_guard<std::mutex> lock(mtx);
                connections.erase(&conn);
            }
            std::cout << "Open Connections " << connections.size() << std::endl;
            std::cout << "Open Connections requests " << connectionRequests.size() << std::endl;
        })
        .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
            auto connection = connections.find(&conn);
            if (connection != connections.end()) {
                if (is_binary) {
                    handleBinaryMessage(data,connection->second,conn);
                }else{
                    handleMessage(data, connection->second,conn);
                }
            } else {
                std::lock_guard<std::mutex> lock(mtx);
                auto uuidIt = connectionRequests.find(data);
                
                if (uuidIt != connectionRequests.end()) {/*if valid uuid in request accept the connection*/

                    /*Callback for sending status data in (json format)*/
                    auto statusCallback = [&conn](const std::string& status) {
                        conn.send_text(status);
                    };
                    /*Callback for sending binary data. For example result files*/
                    auto dataCallback = [&conn](const std::string& data) {
                        conn.send_binary(data);
                    };

                    std::string configFile = fs::temp_directory_path() / data / "project.json";
                    std::unique_ptr<Controller> controller = std::make_unique<Controller>(configFile,statusCallback,dataCallback);

                    connections.emplace(&conn, std::make_tuple(std::move(controller), ConnectionState::AuthenticatedConnection));
                    connectionRequests.erase(uuidIt);
                } else {
                    conn.close();
                }
            }
        });

    CROW_ROUTE(app, "/setSimulationData").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        auto json_data = crow::json::load(req.body);
        if (!json_data) {
            return crow::response(400);
        }
        if (connectionRequests.size() >= maxConnections) {
            return crow::response(503, "Max connections reached");
        }
        return saveProject(json_data, req);
    });

    CROW_ROUTE(app, "/")([]() {
        crow::mustache::context ctx;
        auto page = crow::mustache::load("index.html").render();
        return page;
    });

    app.port(port).run();

    std::cout << "Server closed..." << std::endl;
    return 0;
}