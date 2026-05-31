#include <grpcpp/grpcpp.h>
#include <iostream>
#include <memory>
#include <string>
#include <random>

#include "order.pb.h"
#include "order.grpc.pb.h"

class OrderServiceImpl final : public order::OrderService::Service {
public:
    grpc::Status CreateOrder(
        grpc::ServerContext* context,
        const order::CreateOrderRequest* request,
        order::CreateOrderResponse* response) override {

        std::cout << "[CreateOrder] Received request for account_id: " 
                  << request->account_id() << std::endl;
        std::cout << "[CreateOrder] buyer_id: " << request->buyer_id() << std::endl;
        std::cout << "[CreateOrder] buyer_phone: " << request->buyer_phone() << std::endl;

        if (request->account_id().empty()) {
            response->set_success(false);
            auto* error = response->mutable_error();
            error->set_error_code(order::ERROR_CODE::INTERNAL_ERROR);
            error->set_http_status_code(400);
            error->set_user_message("Account ID cannot be empty");
            error->set_error_message("Invalid request: missing account_id");
            error->set_timestamp(time(nullptr));
            return grpc::Status::OK;
        }

        std::string order_sn = generate_order_sn();
        std::string payment_url = "https://payment.example.com/pay/" + order_sn;

        response->set_success(true);
        response->set_order_sn(order_sn);
        response->set_payment_url(payment_url);

        std::cout << "[CreateOrder] Generated order_sn: " << order_sn << std::endl;
        std::cout << "[CreateOrder] Generated payment_url: " << payment_url << std::endl;

        return grpc::Status::OK;
    }

    grpc::Status UpdateOrderStatus(
        grpc::ServerContext* context,
        const order::UpdateOrderStatusRequest* request,
        order::UpdateOrderStatusResponse* response) override {

        std::cout << "[UpdateOrderStatus] Received token: " 
                  << request->token() << std::endl;
        std::cout << "[UpdateOrderStatus] Received password: *****" << std::endl;

        if (request->token().empty()) {
            response->set_success(false);
            auto* error = response->mutable_error();
            error->set_error_code(order::ERROR_CODE::INVALID_TOKEN);
            error->set_http_status_code(401);
            error->set_user_message("Token is required");
            error->set_error_message("Authentication failed: missing token");
            error->set_timestamp(time(nullptr));
            return grpc::Status::OK;
        }

        if (request->token() == "expired_token") {
            response->set_success(false);
            auto* error = response->mutable_error();
            error->set_error_code(order::ERROR_CODE::TOKEN_EXPIRED);
            error->set_http_status_code(401);
            error->set_user_message("Token has expired");
            error->set_error_message("Authentication failed: token expired");
            error->set_timestamp(time(nullptr));
            return grpc::Status::OK;
        }

        response->set_success(true);
        std::cout << "[UpdateOrderStatus] Order status updated successfully" << std::endl;

        return grpc::Status::OK;
    }

private:
    std::string generate_order_sn() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(100000, 999999);
        
        time_t now = time(nullptr);
        char buffer[64];
        strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", localtime(&now));
        
        std::string order_sn = "ORD";
        order_sn += buffer;
        order_sn += std::to_string(dis(gen));
        return order_sn;
    }
};

void run_server(const std::string& port) {
    std::string server_address("0.0.0.0:" + port);
    OrderServiceImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    
    std::cout << "=============================================" << std::endl;
    std::cout << "  OrderService gRPC Server Starting..." << std::endl;
    std::cout << "  Listening on: " << server_address << std::endl;
    std::cout << "=============================================" << std::endl;

    server->Wait();
}

int main(int argc, char** argv) {
    std::string port = "50051";
    if (argc > 1) {
        port = argv[1];
    }
    
    run_server(port);
    return 0;
}