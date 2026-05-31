#include <grpcpp/grpcpp.h>
#include <csignal>
#include <iostream>
#include <memory>
#include <string>
#include <atomic>

#include "order.grpc.pb.h"
#include "order_service.hpp"
#include "common/config.hpp"
#include "common/logger.hpp"
#include "common/redis_client.hpp"
#include "common/mysql_pool.hpp"

std::unique_ptr<grpc::Server> g_server;
std::shared_ptr<OrderServiceImpl> g_order_service;
std::atomic<bool> g_shutting_down(false);

void shutdownServer() {
    if (g_shutting_down.exchange(true)) {
        return;
    }
    LOG_INFO("正在优雅关闭服务...");
    if (g_server) {
        g_server->Shutdown();
    }
}

void signalHandler(int signal) {
    LOG_INFO("收到信号 {}，触发优雅关闭...", signal);
    std::thread shutdown_thread(shutdownServer);
    shutdown_thread.detach();
}

void runServer(int port) {
    std::string server_address = "0.0.0.0:" + std::to_string(port);

    g_order_service = std::make_shared<OrderServiceImpl>();

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(g_order_service.get());

    builder.SetMaxSendMessageSize(1024 * 1024 * 10);
    builder.SetMaxReceiveMessageSize(1024 * 1024 * 10);

    builder.AddChannelArgument(GRPC_ARG_MAX_CONCURRENT_STREAMS, 100);
    builder.AddChannelArgument(GRPC_ARG_INITIAL_RECONNECT_BACKOFF_MS, 1000);
    builder.AddChannelArgument(GRPC_ARG_MIN_RECONNECT_BACKOFF_MS, 1000);

    g_server = builder.BuildAndStart();

    if (!g_server) {
        LOG_ERROR("gRPC 服务器启动失败，端口: {}", port);
        return;
    }

    LOG_INFO("=============================================");
    LOG_INFO("  订单服务 gRPC 服务器启动中...");
    LOG_INFO("  监听地址: {}", server_address);
    LOG_INFO("=============================================");

    g_server->Wait();
}

int main(int argc, char** argv) {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    config::Config& config = const_cast<config::Config&>(config::Config::getInstance());
    if (!config.loadFromEnv(".env")) {
        std::cerr << "[Main] 配置文件加载失败，使用默认配置" << std::endl;
    }

    logger::Logger::getInstance().init(true, "order_service.log", 10 * 1024 * 1024, 5);
    LOG_INFO("日志系统初始化完成");

    if (!redis_client::RedisClient::getInstance().connect()) {
        LOG_ERROR("Redis 连接失败");
        std::cerr << "[Main] Redis 连接失败" << std::endl;
    } else {
        LOG_INFO("Redis 连接成功");
    }

    if (!mysql_pool::MySQLPool::getInstance().init(10)) {
        LOG_ERROR("MySQL 连接池初始化失败");
        std::cerr << "[Main] MySQL 连接池初始化失败" << std::endl;
    } else {
        LOG_INFO("MySQL 连接池初始化成功");
    }

    int port = config.getOrderServicePort();
    if (port <= 0) {
        port = 8083;
    }

    runServer(port);

    LOG_INFO("服务器关闭中，正在清理资源...");

    redis_client::RedisClient::getInstance().disconnect();

    LOG_INFO("资源清理完成，服务已关闭！");

    return 0;
}