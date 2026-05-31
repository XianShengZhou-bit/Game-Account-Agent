#ifndef JWT_HPP
#define JWT_HPP

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include "logger.hpp"

namespace jwt {

struct Payload {
    std::string order_sn;
    double price;
    int64_t exp;
    bool valid = true;
    std::string error_message;
};

class JWTUtils {
public:
    static std::string base64Encode(const std::string& input) {
        LOG_INFO("[JWT] Base64 编码开始");
        BIO* bio = BIO_new(BIO_f_base64());
        BIO* bmem = BIO_new(BIO_s_mem());
        BIO_set_close(bmem, BIO_NOCLOSE);
        bmem = BIO_push(bio, bmem);
        BIO_set_flags(bmem, BIO_FLAGS_BASE64_NO_NL);

        BUF_MEM* bptr = nullptr;
        BIO_get_mem_ptr(bmem, &bptr);

        BIO_write(bmem, input.data(), static_cast<int>(input.size()));
        BIO_flush(bmem);

        std::string result(bptr->data, bptr->length);
        BIO_free_all(bmem);
        LOG_INFO("[JWT] Base64 编码完成，长度: {}", result.length());
        return result;
    }

    static std::string base64Decode(const std::string& input) {
        LOG_INFO("[JWT] Base64 解码开始");
        std::string output;
        BIO* bio = BIO_new_mem_buf(input.data(), static_cast<int>(input.size()));
        BIO* b64 = BIO_new(BIO_f_base64());
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
        bio = BIO_push(b64, bio);

        char buffer[1024];
        int len = 0;
        while ((len = BIO_read(bio, buffer, sizeof(buffer))) > 0) {
            output.append(buffer, len);
        }

        BIO_free_all(bio);
        LOG_INFO("[JWT] Base64 解码完成，长度: {}", output.length());
        return output;
    }

    static std::string createToken(const std::string& order_sn, double price,
                                   const std::string& algorithm, const std::string& secret, 
                                   int64_t expire_seconds = 3600) {
        LOG_INFO("[JWT] 创建 Token 开始，订单号: {}，价格: {}，算法: {}，过期时间: {}秒", 
                order_sn, price, algorithm, expire_seconds);
        auto now = std::chrono::system_clock::now();
        auto exp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() + expire_seconds;

        std::map<std::string, std::string> payload_map = {
            {"order_sn", order_sn},
            {"price", std::to_string(price)},
            {"exp", std::to_string(exp)}
        };

        LOG_INFO("[JWT] 构建 Payload Map 完成");

        std::string payload_json = mapToJson(payload_map);
        std::string encoded_payload = base64Encode(payload_json);

        std::string header_json = R"({"alg":")" + algorithm + R"(","typ":"JWT"})";
        std::string encoded_header = base64Encode(header_json);

        std::string signing_input = encoded_header + "." + encoded_payload;
        std::string signature = hmacSign(signing_input, secret, algorithm);

        std::string token = signing_input + "." + signature;
        LOG_INFO("[JWT] 创建 Token 完成");
        return token;
    }

    static Payload verifyToken(const std::string& token, const std::string& algorithm, const std::string& secret) {
        LOG_INFO("[JWT] 验证 Token 开始");
        Payload payload;
        payload.valid = false;

        std::vector<std::string> parts = split(token, '.');
        if (parts.size() != 3) {
            LOG_WARN("[JWT] Token 格式无效，分段数错误: {}", parts.size());
            payload.error_message = "Token 格式无效";
            return payload;
        }

        LOG_INFO("[JWT] 分割 Token 完成，分段数: 3");

        std::string expected_signature = hmacSign(parts[0] + "." + parts[1], secret, algorithm);
        if (expected_signature != parts[2]) {
            LOG_WARN("[JWT] Token 签名验证失败");
            payload.error_message = "Token 签名无效";
            return payload;
        }

        LOG_INFO("[JWT] 签名验证通过");

        std::string payload_json = base64Decode(parts[1]);
        std::map<std::string, std::string> payload_map = jsonToMap(payload_json);

        auto it = payload_map.find("order_sn");
        if (it != payload_map.end()) {
            payload.order_sn = it->second;
            LOG_INFO("[JWT] 解析订单号: {}", payload.order_sn);
        } else {
            LOG_WARN("[JWT] Token 中缺少订单号");
            payload.error_message = "Token 中缺少订单号";
            return payload;
        }

        it = payload_map.find("price");
        if (it != payload_map.end()) {
            try {
                payload.price = std::stod(it->second);
                LOG_INFO("[JWT] 解析价格: {}", payload.price);
            } catch (...) {
                LOG_WARN("[JWT] Token 中价格格式无效");
                payload.error_message = "Token 中价格格式无效";
                return payload;
            }
        } else {
            LOG_WARN("[JWT] Token 中缺少价格");
            payload.error_message = "Token 中缺少价格";
            return payload;
        }

        it = payload_map.find("exp");
        if (it != payload_map.end()) {
            try {
                payload.exp = std::stoll(it->second);
                LOG_INFO("[JWT] 解析过期时间: {}", payload.exp);
            } catch (...) {
                LOG_WARN("[JWT] Token 中过期时间格式无效");
                payload.error_message = "Token 中过期时间格式无效";
                return payload;
            }
        } else {
            LOG_WARN("[JWT] Token 中缺少过期时间");
            payload.error_message = "Token 中缺少过期时间";
            return payload;
        }

        auto now = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        if (payload.exp < now) {
            LOG_WARN("[JWT] Token 已过期，过期时间: {}，当前时间: {}", payload.exp, now);
            payload.error_message = "Token 已过期";
            return payload;
        }

        payload.valid = true;
        LOG_INFO("[JWT] Token 验证成功");
        return payload;
    }

private:
    static std::string hmacSign(const std::string& data, const std::string& key, const std::string& algorithm) {
        const EVP_MD* md = nullptr;
        if (algorithm == "HS256") {
            md = EVP_sha256();
        } else if (algorithm == "HS384") {
            md = EVP_sha384();
        } else if (algorithm == "HS512") {
            md = EVP_sha512();
        } else {
            md = EVP_sha256();
        }

        unsigned char* digest = HMAC(md,
            key.data(), static_cast<int>(key.size()),
            reinterpret_cast<const unsigned char*>(data.data()), static_cast<int>(data.size()),
            nullptr, nullptr);

        int digest_len = EVP_MD_size(md);
        std::stringstream ss;
        for (int i = 0; i < digest_len; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
        }
        return ss.str();
    }

    static std::string mapToJson(const std::map<std::string, std::string>& m) {
        std::stringstream ss;
        ss << "{";
        bool first = true;
        for (const auto& kv : m) {
            if (!first) ss << ",";
            ss << "\"" << kv.first << "\":\"" << kv.second << "\"";
            first = false;
        }
        ss << "}";
        return ss.str();
    }

    static std::map<std::string, std::string> jsonToMap(const std::string& json) {
        std::map<std::string, std::string> result;
        std::string content = json;
        content.erase(std::remove_if(content.begin(), content.end(), ::isspace), content.end());

        if (content.front() == '{' && content.back() == '}') {
            content = content.substr(1, content.length() - 2);
        }

        size_t pos = 0;
        while (pos < content.length()) {
            size_t key_start = content.find('"', pos);
            if (key_start == std::string::npos) break;
            size_t key_end = content.find('"', key_start + 1);
            if (key_end == std::string::npos) break;

            std::string key = content.substr(key_start + 1, key_end - key_start - 1);

            size_t value_start = content.find('"', key_end + 1);
            if (value_start == std::string::npos) break;
            size_t value_end = content.find('"', value_start + 1);
            if (value_end == std::string::npos) break;

            std::string value = content.substr(value_start + 1, value_end - value_start - 1);
            result[key] = value;

            pos = value_end + 1;
            if (content[pos] == ',') pos++;
        }

        return result;
    }

    static std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> result;
        std::stringstream ss(str);
        std::string item;
        while (std::getline(ss, item, delimiter)) {
            result.push_back(item);
        }
        return result;
    }
};

}

/*
================================================================================
JWT 编码/解码数据流说明
================================================================================
【禁止删除此注释】

一、生成 Token (createToken)
────────────────────────────
    std::map ──→ mapToJson ──→ JSON 字符串 ──→ base64Encode ──→ payload
                  ↑ 序列化                              ↑
                  │                                     │
             业务数据                              JWT 标准要求


二、解析 Token (verifyToken)
────────────────────────────
    payload ──→ base64Decode ──→ JSON 字符串 ──→ jsonToMap ──→ std::map
                  │                           ↓                   ↑
             JWT 标准格式              反序列化                 ↓
                                                 order_sn, price, exp...


三、为何需要 JSON 序列化/反序列化
────────────────────────────
    JWT 标准（RFC 7519）要求 payload 必须是 JSON 格式：
    - header: {"alg":"HS256","typ":"JWT"}
    - payload: {"order_sn":"...","price":"...","exp":"..."}

    但业务代码需要用 map 按 key 取值，所以需要：
    - mapToJson: map → JSON（生成 token 时）
    - jsonToMap: JSON → map（解析 token 时）


四、公开 API 与私有辅助函数
────────────────────────────
    公开 API:
        createToken()    - 生成 token
        verifyToken()    - 解析 token
        base64Encode()   - Base64 编码
        base64Decode()   - Base64 解码

    私有辅助函数 (内部实现):
        hmacSign()        - 生成 HMAC 签名（支持 HS256/384/512）
        mapToJson()       - 将 map 序列化为 JSON
        jsonToMap()       - 将 JSON 反序列化为 map
        split()           - 字符串分割（按 "." 分割 token）


五、日志说明
────────────────────────────
    本模块使用 Logger 打印日志，所有 public 函数执行开始时都会打印中文日志：
    - LOG_INFO: 正常流程日志
    - LOG_WARN: 异常/错误日志
================================================================================
*/

#endif