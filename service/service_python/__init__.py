"""
Python gRPC 客户端包
用于连接和调用 C++ 订单服务后端
"""

from .order_client import OrderClient

__all__ = ['OrderClient']
__version__ = '1.0.0'
