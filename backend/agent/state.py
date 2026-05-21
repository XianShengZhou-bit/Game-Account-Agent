"""状态定义模块 - 定义 MainState 和 ReActState"""

from dataclasses import dataclass, field
from typing import Any, Dict, List


@dataclass
class MainState:
    """主图状态定义"""
    user_input: str = ""                            # 用户输入
    current_action: str = ""                         # 当前执行的动作
    tool_calls: List[Dict[str, Any]] = field(default_factory=list)  # 工具调用历史
    tool_results: List[Any] = field(default_factory=list)  # 工具执行结果
    response: str = ""                              # 返回给用户的响应


@dataclass
class ReActState:
    """ReAct 循环状态"""
    user_input: str = ""                            # 用户输入
    thought: str = ""                               # LLM 思考过程
    action: str = ""                                # 当前要执行的动作
    action_input: Dict[str, Any] = field(default_factory=dict)  # 动作输入参数
    observation: str = ""                           # 工具执行结果
    response: str = ""                              # 最终响应
    step_count: int = 0                             # 循环步数
    max_steps: int = 20                             # 最大循环步数
    history: List[Dict[str, Any]] = field(default_factory=list)  # 完整历史记录
