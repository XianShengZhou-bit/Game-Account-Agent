"""LLM工具模块 - 提供LLM相关的工具"""

import os
import sys
import re
import json
from typing import Any, Dict, List
from dotenv import load_dotenv
from openai import OpenAI

# 添加项目根目录到路径
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../..')))

from agent.common.logging_config import setup_logger

load_dotenv()

logger = setup_logger(__name__)

LLM_API_KEY = os.getenv("LLM_API_KEY")
LLM_BASE_URL = os.getenv("LLM_BASE_URL")
LLM_MODELS: List[str] = eval(os.getenv("LLM_MODELS"))
LLM_TIMEOUT = int(os.getenv("LLM_TIMEOUT"))


def summarize_result(**kwargs) -> str:
    """总结任意查询结果为礼貌得体的自然语言

    Args:
        **kwargs: 任意关键字参数（支持 query_result, results, data, items 等任意参数名）

    Returns:
        格式化的自然语言响应
    """
    data = None
    for key, value in kwargs.items():
        if value is not None:
            data = value
            break
    
    logger.info(f"总结查询结果: {type(data).__name__}")

    if not data:
        return "抱歉，没有找到符合条件的结果。"

    system_prompt = """你是一个智能助手。请根据提供的数据，用自然、友好的语言为用户总结查询结果。

要求：
1. 分析数据内容，识别数据类型和关键信息
2. 根据数据类型选择合适的总结方式
3. 如果是列表数据，清晰列出每个项目的关键信息
4. 如果是字典数据，解释各字段含义
5. 如果是文本数据，提取核心要点
6. 格式美观，易于阅读
7. 语言礼貌得体"""

    user_prompt = f"""请帮我总结以下查询结果：

{data}

请用自然、友好的语言总结这些信息。"""

    messages = [
        {"role": "system", "content": system_prompt},
        {"role": "user", "content": user_prompt}
    ]

    response = llm_call(messages, temperature=0.7)

    if not response:
        logger.error("LLM总结失败，使用默认响应")
        return f"查询完成，共 {len(query_result) if hasattr(query_result, '__len__') else 1} 条结果。"

    return response


def extract_account_id(user_input: str) -> Dict[str, Any]:
    """从用户输入中提取账号ID

    Args:
        user_input: 用户输入，包含账号ID

    Returns:
        包含account_id的字典
    """
    logger.info(f"从用户输入中提取账号ID: {user_input}")

    # 匹配16位大写字母数字组合
    pattern = r'\b([A-F0-9]{16})\b'
    matches = re.findall(pattern, user_input.upper())

    if matches:
        account_id = matches[0]
        logger.info(f"提取到账号ID: {account_id}")
        return {
            "account_id": account_id
        }
    else:
        logger.warning("未找到账号ID")
        return {
            "account_id": ""
        }


def generate_payment_link(order_sn: str) -> Dict[str, Any]:
    """生成支付链接

    Args:
        order_sn: 订单号

    Returns:
        包含payment_link的字典
    """
    logger.info(f"生成支付链接: order_sn={order_sn}")

    # 这里应该调用真实的支付系统
    # 暂时使用模拟的支付链接
    payment_link = f"https://example.com/pay/{order_sn}"

    return {
        "payment_link": payment_link
    }


def llm_call(messages: List[Dict[str, str]], temperature: float = 0.0, tools: List[Dict[str, Any]] = None) -> Any:
    """调用LLM接口，支持工具调用

    Args:
        messages: 消息列表
        temperature: 温度参数
        tools: 工具定义列表（支持OpenAI Tool Calling格式）

    Returns:
        LLM的响应文本或工具调用信息
    """
    if not LLM_API_KEY or not LLM_BASE_URL:
        logger.warning("LLM配置不完整")
        return ""

    logger.info("开始LLM调用，尝试模型列表")

    cnt = 0
    while cnt < 3:
        cnt += 1
        for model in LLM_MODELS:
            try:
                client = OpenAI(
                    api_key=LLM_API_KEY,
                    base_url=LLM_BASE_URL,
                    max_retries=0,
                    timeout=float(LLM_TIMEOUT)
                )

                kwargs = {
                    "model": model,
                    "messages": messages,
                    "temperature": temperature,
                }

                if tools:
                    kwargs["tools"] = tools
                    kwargs["tool_choice"] = "auto"

                response = client.chat.completions.create(**kwargs)

                message = response.choices[0].message

                if message.tool_calls:
                    logger.info(f"LLM选择调用工具: {message.tool_calls[0].function.name}")
                    return {
                        "type": "tool_call",
                        "name": message.tool_calls[0].function.name,
                        "arguments": json.loads(message.tool_calls[0].function.arguments)
                    }

                result = message.content.strip()
                logger.info(f"LLM调用成功: model={model}")
                return result

            except Exception as e:
                logger.warning(f"模型 {model} 调用失败: {e}，尝试下一个模型")
                continue

    logger.error("所有模型均调用失败")
    return ""
