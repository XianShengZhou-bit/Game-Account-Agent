#include <iostream>
#include "../service/cpp/common/jwt.hpp"
#include "../service/cpp/common/config.hpp"
#include <windows.h>

int main() {
    SetConsoleOutputCP(CP_UTF8);
    
    if (!config::Config::getInstance().loadFromEnv("../../../.env")) {
        std::cerr << "Failed to load environment configuration" << std::endl;
        return 1;
    }
    
    const std::string algorithm = config::Config::getInstance().getJwtAlgorithm();
    const std::string secret = config::Config::getInstance().getJwtSecret();
    
    if (secret.empty()) {
        std::cerr << "JWT_SECRET not found in environment configuration" << std::endl;
        return 1;
    }
    
    std::cout << "========================================" << std::endl;
    std::cout << "JWT 各环节数据打印测试" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    std::cout << "【第1步】原始数据" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::string order_sn = "OS08817475619653";
    double price = 99.50;
    int64_t expire_seconds = 3600;
    std::cout << "order_sn: " << order_sn << std::endl;
    std::cout << "price: " << price << std::endl;
    std::cout << "expire_seconds: " << expire_seconds << std::endl;
    std::cout << std::endl;

    std::cout << "【第2步】createToken 生成 Token" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::string token = jwt::JWTUtils::createToken(order_sn, price, algorithm, secret, expire_seconds);
    std::cout << "生成的 Token:" << std::endl;
    std::cout << token << std::endl;
    std::cout << std::endl;

    std::cout << "【第3步】Token 结构分析" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Token 由三部分组成，用 '.' 分割:" << std::endl;
    
    size_t first_dot = token.find('.');
    size_t second_dot = token.find('.', first_dot + 1);
    std::string header_b64 = token.substr(0, first_dot);
    std::string payload_b64 = token.substr(first_dot + 1, second_dot - first_dot - 1);
    std::string signature = token.substr(second_dot + 1);
    
    std::cout << "1. Header (Base64): " << header_b64 << std::endl;
    std::cout << "   长度: " << header_b64.length() << " 字符" << std::endl;
    std::cout << std::endl;
    std::cout << "2. Payload (Base64): " << payload_b64 << std::endl;
    std::cout << "   长度: " << payload_b64.length() << " 字符" << std::endl;
    std::cout << std::endl;
    std::cout << "3. Signature (HMAC-SHA256): " << signature << std::endl;
    std::cout << "   长度: " << signature.length() << " 字符" << std::endl;
    std::cout << std::endl;

    std::cout << "【第4步】Base64 解码后的 Header" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::string header_json = jwt::JWTUtils::base64Decode(header_b64);
    std::cout << header_json << std::endl;
    std::cout << std::endl;

    std::cout << "【第5步】Base64 解码后的 Payload" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::string payload_json = jwt::JWTUtils::base64Decode(payload_b64);
    std::cout << payload_json << std::endl;
    std::cout << std::endl;

    std::cout << "【第6步】verifyToken 验证 Token" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    jwt::Payload result = jwt::JWTUtils::verifyToken(token, algorithm, secret);
    
    std::cout << "valid: " << (result.valid ? "true" : "false") << std::endl;
    if (result.valid) {
        std::cout << "order_sn: " << result.order_sn << std::endl;
        std::cout << "price: " << result.price << std::endl;
        std::cout << "exp: " << result.exp << std::endl;
    } else {
        std::cout << "error_message: " << result.error_message << std::endl;
    }
    std::cout << std::endl;

    std::cout << "【第7步】模拟篡改后的验证" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "尝试修改 payload 中的 price 从 99.50 改为 0.01..." << std::endl;
    
    std::string tampered_payload = jwt::JWTUtils::base64Encode(
        "{\"order_sn\":\"OS08817475619653\",\"price\":\"0.01\",\"exp\":\"1748236800\"}"
    );
    std::string tampered_token = header_b64 + "." + tampered_payload + "." + signature;
    
    std::cout << "篡改后的 Token:" << std::endl;
    std::cout << tampered_token << std::endl;
    std::cout << std::endl;
    
    jwt::Payload tampered_result = jwt::JWTUtils::verifyToken(tampered_token, algorithm, secret);
    std::cout << "验证结果: " << (tampered_result.valid ? "通过" : "失败") << std::endl;
    if (!tampered_result.valid) {
        std::cout << "错误原因: " << tampered_result.error_message << std::endl;
    }
    std::cout << std::endl;

    std::cout << "========================================" << std::endl;
    std::cout << "测试完成" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
