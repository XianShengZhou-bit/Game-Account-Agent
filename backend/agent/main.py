"""主图模块 - 包含 Main Graph 和 ReAct Loop"""

import os
import sys
from typing import Any, Dict
from dotenv import load_dotenv
from langgraph.graph import StateGraph

# 添加项目根目录到路径
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from agent.state import MainState, ReActState
from agent.tools.database_tools import (
    create_accounts,
    search_accounts,
    update_account,
    create_order,
    get_supported_games
)
from agent.tools.llm_tools import (
    summarize_result,
    extract_account_id,
    generate_payment_link
)
from agent.common.logging_config import setup_logger

load_dotenv()

logger = setup_logger(__name__)


def preprocess(state: MainState) -> Dict[str, Any]:
    """预处理节点 - 对用户输入进行预处理"""
    logger.info(f"预处理用户输入: {state.user_input[:50]}...")
    processed_input = state.user_input.strip()
    if not processed_input:
        processed_input = "您好"
    return {}


def reasoner(state: ReActState) -> Dict[str, Any]:
    """Think 节点 - LLM分析当前状态，决定下一步行动

    这个节点使用 ReAct 模式，让 LLM 自主决定调用哪个工具
    使用 OpenAI Tool Calling 原生支持，无需手动解析 JSON
    """
    logger.info(f"Reasoner - Step {state.step_count}: 分析下一步行动")

    # 定义工具列表（OpenAI Tool Calling 格式）
    tools = [
        {
            "type": "function",
            "function": {
                "name": "get_supported_games",
                "description": "获取平台当前支持的游戏列表",
                "parameters": {}
            }
        },
        {
            "type": "function",
            "function": {
                "name": "search_accounts",
                "description": "搜索游戏账号",
                "parameters": {
                    "game_name": {"type": "string", "description": "游戏名称"},
                    "target_min": {"type": "number", "description": "最低价格（元）"},
                    "target_max": {"type": "number", "description": "最高价格（元）"},
                    "server_area": {"type": "string", "description": "区服"},
                    "skin_count": {"type": "number", "description": "皮肤数量"},
                    "account_level": {"type": "number", "description": "账号等级"},
                    "hero_count": {"type": "number", "description": "英雄数量"},
                    "rare_items_contains": {"type": "string", "description": "稀有物品包含"},
                    "number": {"type": "number", "description": "返回数量"}
                }
            }
        },
        {
            "type": "function",
            "function": {
                "name": "create_accounts",
                "description": "创建游戏账号",
                "parameters": {
                    "game_name": {"type": "string", "description": "游戏名称"},
                    "account_name": {"type": "string", "description": "账号名称"},
                    "password": {"type": "string", "description": "密码"},
                    "phone": {"type": "string", "description": "手机号"},
                    "id_card": {"type": "string", "description": "身份证号"}
                }
            }
        },
        {
            "type": "function",
            "function": {
                "name": "create_order",
                "description": "创建订单, 价格必须从数据库中获取",
                "parameters": {
                    "account_id": {"type": "string", "description": "账号ID"},
                    "buyer_id": {"type": "string", "description": "买家ID"},
                    "price": {"type": "number", "description": "价格"},
                    "buyer_phone": {"type": "string", "description": "买家手机号"},
                    "buyer_id_card": {"type": "string", "description": "买家身份证号"}
                }
            }
        },
        {
            "type": "function",
            "function": {
                "name": "update_account",
                "description": "更新账号状态",
                "parameters": {
                    "account_id": {"type": "string", "description": "账号ID"},
                    "status": {"type": "string", "description": "状态"}
                }
            }
        },
        {
            "type": "function",
            "function": {
                "name": "summarize_result",
                "description": "总结查询结果为自然语言",
                "parameters": {
                    "query_result": {"type": "string", "description": "查询结果"}
                }
            }
        },
        {
            "type": "function",
            "function": {
                "name": "extract_account_id",
                "description": "从用户输入中提取账号ID",
                "parameters": {
                    "user_input": {"type": "string", "description": "用户输入"}
                }
            }
        },
        {
            "type": "function",
            "function": {
                "name": "generate_payment_link",
                "description": "生成支付链接",
                "parameters": {
                    "order_sn": {"type": "string", "description": "订单号"}
                }
            }
    }
]

    system_prompt = """你是一个游戏账号交易平台的智能助手。你的核心目标是帮助用户购买游戏账号。

**重要提醒：**
- 历史记录中包含用户的原始需求，**必须**从中提取 game_name 等关键信息
- **绝对禁止**在用户已明确需求的情况下，调用工具时不传任何参数
- 调用 search_accounts 时，**必须**根据用户需求设置 game_name 参数

决策规则：
1. 如果不清楚平台支持的游戏，先调用 get_supported_games
2. 如果用户想搜索/浏览账号，使用 search_accounts
3. 如果需要提取账号ID，使用 extract_account_id
4. **搜索完成后，立即调用 summarize_result 总结结果**
5. 如果需要生成支付链接，使用 generate_payment_link
6. 搜索账号时，game_name 必须使用 get_supported_games 返回的准确名称
7. 最终给用户的返回必须是通俗易懂，要注意语法的优雅和美观

核心指令：
- 用户说"购买"、"要"、"想要"等词时，表明用户有购买意向
- 搜索结果出来后，直接总结账号信息，然后引导用户提供账号ID进行购买
- 不要追问预算、偏好等额外问题，除非用户主动提及
- 始终保持目标：帮助用户完成购买流程

如果不需要调用工具，可以直接回答用户。"""

    user_input = state.user_input
    observation = state.observation
    history = state.history
    
    # 构建标准的 OpenAI 工具调用消息格式
    messages = [
        {"role": "system", "content": system_prompt},
        {"role": "user", "content": user_input}
    ]
    
    # 添加历史工具调用记录（使用 assistant/tool 角色）
    for idx, record in enumerate(history):
        action = record.get("action", "")
        action_input = record.get("action_input", {})
        obs = record.get("observation", "")
        
        # 添加助手调用工具的消息
        messages.append({
            "role": "assistant",
            "content": "",
            "tool_calls": [{
                "id": f"call_{idx}",
                "type": "function",
                "function": {
                    "name": action,
                    "arguments": action_input
                }
            }]
        })
        
        # 添加工具执行结果的消息
        messages.append({
            "role": "tool",
            "content": obs,
            "tool_call_id": f"call_{idx}"
        })
    
    # 添加最新观察结果
    if observation:
        messages.append({
            "role": "user",
            "content": f"上一步工具执行结果：{observation}"
        })

    # 调用 LLM（使用原生工具调用）
    from agent.tools.llm_tools import llm_call
    result = llm_call(messages, tools=tools)

    if not result:
        return {
            "thought": "LLM调用失败",
            "action": "direct_response",
            "action_input": {"response": "抱歉，服务暂时不可用，请稍后再试。"}
        }

    # 判断是否为工具调用
    if isinstance(result, dict) and result.get("type") == "tool_call":
        logger.info(f"LLM选择调用工具: {result['name']}")
        return {
            "thought": f"调用工具: {result['name']}",
            "action": result["name"],
            "action_input": result["arguments"]
        }

    # 如果是直接回答，直接返回结果
    logger.info("LLM直接回答，不调用工具")
    return {
        "thought": "直接回答用户",
        "action": "direct_response",
        "action_input": {"response": result}
    }


def tool_executor(state: ReActState) -> Dict[str, Any]:
    """Act 节点 - 执行 LLM 选择的工具"""
    logger.info(f"Tool Executor - 执行工具: {state.action}")

    action = state.action
    action_input = state.action_input
    logger.info(f"Tool Executor - 执行工具参数: {action_input}")

    try:
        if action == "get_supported_games":
            result = get_supported_games()
            observation = f"支持的游戏列表：{result}"

        elif action == "search_accounts":
            result = search_accounts(**action_input)
            observation = f"搜索结果：{result}"

        elif action == "create_order":
            result = create_order(**action_input)
            observation = f"创建订单结果：{result}"

        elif action == "update_account":
            result = update_account(**action_input)
            observation = f"更新账号结果：{result}"

        elif action == "summarize_result":
            result = summarize_result(**action_input)
            observation = result

        elif action == "extract_account_id":
            result = extract_account_id(**action_input)
            observation = f"提取账号ID结果：{result}"

        elif action == "generate_payment_link":
            result = generate_payment_link(**action_input)
            observation = f"支付链接：{result.get('payment_link', '')}"

        else:
            observation = f"未知工具: {action}"

        logger.info(f"工具执行完成: {observation[:100]}...")
        return {
            "observation": observation,
            "step_count": state.step_count + 1
        }

    except Exception as e:
        logger.error(f"工具执行失败: {e}")
        return {
            "observation": f"工具执行失败: {str(e)}",
            "step_count": state.step_count + 1
        }


def should_continue(state: ReActState) -> str:
    """Observe 节点 - 判断是否继续循环"""
    if state.step_count >= state.max_steps:
        logger.info("达到最大循环次数，结束")
        return "end"

    if "summarize_result" in state.observation or "为您找到" in state.observation:
        logger.info("搜索结果已总结，结束")
        return "end"

    if "订单创建成功" in state.observation or "支付链接" in state.observation:
        logger.info("订单已创建，结束")
        return "end"

    logger.info("继续循环")
    return "continue"


def react_loop(state: ReActState) -> Dict[str, Any]:
    """ReAct Loop - 完整的思考-行动-观察循环"""
    logger.info(f"开始 ReAct Loop，步骤: {state.step_count}")

    observation = ""
    step_count = state.step_count
    history = state.history if state.history else []
    
    if step_count == 0 and state.user_input and not history:
        history.append({
            "action": "user_input",
            "action_input": {"input": state.user_input},
            "observation": "用户输入已记录"
        })

    while step_count < state.max_steps:

        logger.info(f"========== 第 {step_count} 步历史记录 ==========")
        if history:
            for i, record in enumerate(history, 1):
                logger.info(f"步骤{i}:")
                logger.info(f"  action: {record.get('action', '未知')}")
                logger.info(f"  action_input: {record.get('action_input', {})}")
                logger.info(f"  observation: {record.get('observation', '')}")
        else:
            logger.info("历史记录为空")
        logger.info("=============================================")

        current_state = ReActState(
            user_input=state.user_input,
            observation=observation,
            step_count=step_count,
            max_steps=state.max_steps,
            history=history
        )

        reasoner_result = reasoner(current_state)
        action = reasoner_result.get("action", "")
        action_input = reasoner_result.get("action_input", {})

        # 如果是直接回答，立即返回，不需要走工具调用流程
        if action == "direct_response":
            observation = action_input.get("response", "")
            logger.info(f"LLM直接回答，立即返回: {observation[:100]}...")
            break
        
        # 无效的action，直接返回
        if not action:
            observation = "抱歉，我暂时无法处理您的请求，请稍后再试。"
            logger.warning(f"无效的action: {action}")
            break

        # 否则执行工具调用
        executor_state = ReActState(
            user_input=state.user_input,
            thought=reasoner_result.get("thought", ""),
            action=action,
            action_input=action_input,
            step_count=step_count,
            max_steps=state.max_steps,
            history=history
        )

        executor_result = tool_executor(executor_state)
        observation = executor_result.get("observation", "")
        step_count = executor_result.get("step_count", step_count + 1)

        history.append({
            "action": action,
            "action_input": action_input,
            "observation": observation
        })

        should_continue_state = ReActState(
            user_input=state.user_input,
            action=action,
            observation=observation,
            step_count=step_count,
            max_steps=state.max_steps,
            history=history
        )

        if should_continue(should_continue_state) == "end":
            break

    return {
        "response": observation,
        "step_count": step_count
    }


def response_formatter(state: MainState) -> Dict[str, Any]:
    """响应格式化节点"""
    logger.info("格式化响应")
    return {}


def create_main_graph():
    """创建主图"""
    graph = StateGraph(MainState)

    # 添加节点
    graph.add_node("preprocess", preprocess)
    graph.add_node("react_loop", react_loop)
    graph.add_node("response_formatter", response_formatter)

    # 添加边
    graph.add_edge("__start__", "preprocess")
    graph.add_edge("preprocess", "react_loop")
    graph.add_edge("react_loop", "response_formatter")
    graph.add_edge("response_formatter", "__end__")

    return graph.compile(name="Main Graph")


# 创建主图实例
main_graph = create_main_graph()


def run_agent(user_input: str) -> str:
    """运行 Agent 处理用户输入

    Args:
        user_input: 用户输入

    Returns:
        Agent 的响应
    """
    logger.info(f"运行 Agent: {user_input[:50]}...")

    # 创建初始状态（每次都是新的）
    initial_state = MainState(user_input=user_input)

    # 运行图
    result = main_graph.invoke(initial_state)

    return result.get("response", "抱歉，服务暂时不可用。")


# 导出主要接口
__all__ = ["main_graph", "run_agent", "create_main_graph"]
