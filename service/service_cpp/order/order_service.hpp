#ifndef ORDER_SERVICE_HPP
#define ORDER_SERVICE_HPP

#include <string>
#include <memory>
#include <grpcpp/grpcpp.h>
#include "order.grpc.pb.h"
#include "common/config.hpp"
#include "common/logger.hpp"
#include "common/error_code.hpp"
#include "common/jwt.hpp"
#include "common/redis_client.hpp"
#include "common/redis_lock.hpp"
#include "common/mysql_pool.hpp"

class OrderServiceImpl final : public order::OrderService::Service {
public:
    grpc::Status CreateOrder(
        grpc::ServerContext* context,
        const order::CreateOrderRequest* request,
        order::CreateOrderResponse* response) override {

        LOG_INFO("[CreateOrder] 收到创建订单请求 - account_id: {}, buyer_id: {}, phone: {}",
                 request->account_id(), request->buyer_id(), request->buyer_phone());

        if (request->account_id().empty()) {
            return setErrorResponse(response, error_code::BusinessErrorCode::INTERNAL_ERROR,
                                    "账号ID不能为空");
        }

        if (request->buyer_id() <= 0) {
        }

        std::string lock_key = "exchange:lock:account:" + request->account_id();
        redis_lock::ScopedLock account_lock(lock_key, 300, 10000, 100);
        if (!account_lock.isAcquired()) {
            LOG_WARN("[CreateOrder] 账号加锁失败: {}", request->account_id());
            return setErrorResponse(response, error_code::BusinessErrorCode::ACCOUNT_LOCKED,
                                    "账号正在被其他买家操作，请稍后重试");
        }

        mysql_pool::AccountInfo account = mysql_pool::MySQLPool::getInstance().getAccountById(request->account_id());
        if (account.account_id.empty()) {
            LOG_WARN("[CreateOrder] 账号不存在: {}", request->account_id());
            return setErrorResponse(response, error_code::BusinessErrorCode::ORDER_NOT_FOUND,
                                    "账号不存在");
        }

        if (account.status != 0) {
            LOG_WARN("[CreateOrder] 账号已售出: {}", request->account_id());
            return setErrorResponse(response, error_code::BusinessErrorCode::ACCOUNT_SOLD,
                                    "该账号已售出");
        }

        double price = account.price / 100.0;

        if (!mysql_pool::MySQLPool::getInstance().updateAccountStatus(request->account_id(), 1)) {
            LOG_ERROR("[CreateOrder] 账号状态更新失败");
            return setErrorResponse(response, error_code::BusinessErrorCode::DB_ERROR,
                                    "系统繁忙，请稍后重试");
        }

        mysql_pool::OrderInfo order = mysql_pool::MySQLPool::getInstance().createOrder(
            std::stoll(request->account_id()),
            request->buyer_id(),
            request->buyer_phone(),
            request->buyer_id_card(),
            price
        );

        if (order.order_sn.empty()) {
            mysql_pool::MySQLPool::getInstance().updateAccountStatus(request->account_id(), 0);
            LOG_ERROR("[CreateOrder] 订单创建失败");
            return setErrorResponse(response, error_code::BusinessErrorCode::DB_ERROR,
                                    "系统繁忙，请稍后重试");
        }

        const config::Config& config = config::Config::getInstance();
        std::string token = jwt::JWTUtils::createToken(
            order.order_sn, 
            price, 
            config.getJwtAlgorithm(), 
            config.getJwtSecret(), 
            3600
        );

        response->set_success(true);
        response->set_order_sn(order.order_sn);
        response->set_payment_token(token);
        response->set_game_name(account.game_name);
        response->set_server_area(account.server_area);
        response->set_account_title(account.title);
        response->set_price(price);

        LOG_INFO("[CreateOrder] 订单创建成功 - order_sn: {}", order.order_sn);

        return grpc::Status::OK;
    }

    grpc::Status UpdateOrderStatus(
        grpc::ServerContext* context,
        const order::UpdateOrderStatusRequest* request,
        order::UpdateOrderStatusResponse* response) override {

        LOG_INFO("[UpdateOrderStatus] 收到支付请求");

        if (request->token().empty()) {
            return setErrorResponse(response, error_code::BusinessErrorCode::INVALID_TOKEN,
                                    "支付链接无效，请重新下单");
        }

        const config::Config& config = config::Config::getInstance();
        jwt::Payload payload = jwt::JWTUtils::verifyToken(
            request->token(), 
            config.getJwtAlgorithm(), 
            config.getJwtSecret()
        );

        if (!payload.valid) {
            LOG_WARN("[UpdateOrderStatus] Token验证失败: {}", payload.error_message);
            
            if (payload.error_message == "Token expired") {
                return setErrorResponse(response, error_code::BusinessErrorCode::TOKEN_EXPIRED,
                                        "支付链接已过期，请重新下单");
            }
            return setErrorResponse(response, error_code::BusinessErrorCode::INVALID_TOKEN,
                                    "支付链接无效，请重新下单");
        }

        LOG_INFO("[UpdateOrderStatus] Token验证成功, order_sn: {}", payload.order_sn);

        std::string lock_key = "exchange:lock:order:" + payload.order_sn;
        redis_lock::ScopedLock order_lock(lock_key, 60, 10000, 100);
        if (!order_lock.isAcquired()) {
            LOG_WARN("[UpdateOrderStatus] 订单加锁失败: {}", payload.order_sn);
            return setErrorResponse(response, error_code::BusinessErrorCode::ORDER_PAID,
                                    "订单正在处理中，请稍后");
        }

        mysql_pool::OrderInfo order = mysql_pool::MySQLPool::getInstance().getOrderBySn(payload.order_sn);
        if (order.order_sn.empty()) {
            LOG_WARN("[UpdateOrderStatus] 订单不存在: {}", payload.order_sn);
            return setErrorResponse(response, error_code::BusinessErrorCode::ORDER_NOT_FOUND,
                                    "订单不存在");
        }

        if (order.status != 0) {
            LOG_WARN("[UpdateOrderStatus] 订单已支付: {}", payload.order_sn);
            return setErrorResponse(response, error_code::BusinessErrorCode::ORDER_PAID,
                                    "该订单已支付完成");
        }

        if (!mysql_pool::MySQLPool::getInstance().updateOrderStatus(payload.order_sn, 1)) {
            LOG_ERROR("[UpdateOrderStatus] 订单状态更新失败");
            return setErrorResponse(response, error_code::BusinessErrorCode::DB_ERROR,
                                    "系统繁忙，请稍后重试");
        }

        if (!mysql_pool::MySQLPool::getInstance().updateAccountStatus(std::to_string(order.account_id), 2)) {
            LOG_ERROR("[UpdateOrderStatus] 账号状态更新失败");
        }

        response->set_success(true);

        LOG_INFO("[UpdateOrderStatus] 订单支付成功: {}", payload.order_sn);

        return grpc::Status::OK;
    }

private:
    template<typename ResponseType>
    grpc::Status setErrorResponse(ResponseType* response,
                                   error_code::BusinessErrorCode error_code,
                                   const std::string& user_message) {
        response->set_success(false);

        auto error_info = error_code::getErrorInfo(error_code);
        auto* error = response->mutable_error();

        error->set_error_code(static_cast<order::ERROR_CODE>(static_cast<int>(error_code)));
        error->set_http_status_code(error_info.http_status);
        error->set_user_message(error_info.user_message);
        error->set_error_message(user_message);
        error->set_timestamp(time(nullptr));

        return grpc::Status::OK;
    }
};

#endif