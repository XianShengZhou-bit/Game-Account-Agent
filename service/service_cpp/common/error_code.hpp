#ifndef ERROR_CODE_HPP
#define ERROR_CODE_HPP

#include <string>
#include <map>
#include <grpcpp/grpcpp.h>

namespace error_code {

enum class BusinessErrorCode {
    SUCCESS = 0,
    ACCOUNT_LOCKED = 1,
    ACCOUNT_SOLD = 2,
    INVALID_TOKEN = 3,
    TOKEN_EXPIRED = 4,
    ORDER_NOT_FOUND = 5,
    ORDER_PAID = 6,
    BALANCE_INSUFFICIENT = 7,
    INTERNAL_ERROR = 8,
    DB_ERROR = 9,
    REDIS_ERROR = 10
};

struct ErrorInfo {
    grpc::StatusCode grpc_status;
    int http_status;
    std::string user_message;
};

inline ErrorInfo getErrorInfo(BusinessErrorCode code) {
    static std::map<BusinessErrorCode, ErrorInfo> error_map = {
        {BusinessErrorCode::SUCCESS, {grpc::StatusCode::OK, 200, "Success"}},
        {BusinessErrorCode::ACCOUNT_LOCKED, {grpc::StatusCode::RESOURCE_EXHAUSTED, 429, "账号正在被其他买家操作，请稍后重试"}},
        {BusinessErrorCode::ACCOUNT_SOLD, {grpc::StatusCode::ALREADY_EXISTS, 409, "该账号已售出"}},
        {BusinessErrorCode::INVALID_TOKEN, {grpc::StatusCode::UNAUTHENTICATED, 401, "支付链接无效，请重新下单"}},
        {BusinessErrorCode::TOKEN_EXPIRED, {grpc::StatusCode::UNAUTHENTICATED, 401, "支付链接已过期，请重新下单"}},
        {BusinessErrorCode::ORDER_NOT_FOUND, {grpc::StatusCode::NOT_FOUND, 404, "订单不存在"}},
        {BusinessErrorCode::ORDER_PAID, {grpc::StatusCode::ALREADY_EXISTS, 409, "该订单已支付完成"}},
        {BusinessErrorCode::BALANCE_INSUFFICIENT, {grpc::StatusCode::FAILED_PRECONDITION, 400, "余额不足，请充值"}},
        {BusinessErrorCode::INTERNAL_ERROR, {grpc::StatusCode::INTERNAL, 500, "系统繁忙，请稍后重试"}},
        {BusinessErrorCode::DB_ERROR, {grpc::StatusCode::INTERNAL, 500, "系统繁忙，请稍后重试"}},
        {BusinessErrorCode::REDIS_ERROR, {grpc::StatusCode::UNAVAILABLE, 503, "服务暂不可用，请稍后重试"}}
    };

    auto it = error_map.find(code);
    if (it != error_map.end()) {
        return it->second;
    }
    
    return {grpc::StatusCode::INTERNAL, 500, "Unknown error"};
}

}

#endif