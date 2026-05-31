import sys
import os
import grpc

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'proto'))
import order_pb2
import order_pb2_grpc

import time


class OrderClient:
    def __init__(self, host='localhost', port=9000):
        self.host = host
        self.port = port
        self.channel = None
        self.stub = None
        self.connected = False

    def connect(self):
        try:
            self.channel = grpc.insecure_channel(
                f'{self.host}:{self.port}',
                options=[
                    ('grpc.max_send_message_length', 10 * 1024 * 1024),
                    ('grpc.max_receive_message_length', 10 * 1024 * 1024),
                    ('grpc.initial_reconnect_backoff_ms', 1000),
                    ('grpc.min_reconnect_backoff_ms', 1000),
                ]
            )
            self.stub = order_pb2_grpc.OrderServiceStub(self.channel)
            self.connected = True
            print(f"✅ 成功连接到 gRPC 服务器 {self.host}:{self.port}")
            return True
        except grpc.RpcError as e:
            print(f"❌ 连接失败: {e.code()}: {e.details()}")
            self.connected = False
            return False

    def disconnect(self):
        if self.channel:
            self.channel.close()
            self.connected = False
            print("🔌 已断开与 gRPC 服务器的连接")

    def create_order(self, account_id, buyer_id, buyer_phone, buyer_id_card=''):
        if not self.connected:
            print("❌ 未连接到服务器，请先调用 connect()")
            return None

        try:
            request = order_pb2.CreateOrderRequest(
                account_id=account_id,
                buyer_id=buyer_id,
                buyer_phone=buyer_phone,
                buyer_id_card=buyer_id_card
            )
            print(f"📤 发送创建订单请求...")
            print(f"   - 账号ID: {account_id}")
            print(f"   - 买家ID: {buyer_id}")
            print(f"   - 买家电话: {buyer_phone}")

            response = self.stub.CreateOrder(request, timeout=10)

            print(f"📥 收到响应:")
            print(f"   - 成功: {response.success}")
            if response.success:
                print(f"   - 订单号: {response.order_sn}")
                print(f"   - 支付令牌: {response.payment_token}")
                print(f"   - 价格: {response.price}")
                print(f"   - 游戏名: {response.game_name}")
                print(f"   - 服务器: {response.server_area}")
                print(f"   - 账号标题: {response.account_title}")
            else:
                print(f"   - 错误码: {order_pb2.ERROR_CODE.Name(response.error.error_code)}")
                print(f"   - HTTP状态: {response.error.http_status_code}")
                print(f"   - 用户消息: {response.error.user_message}")
                print(f"   - 错误详情: {response.error.error_message}")

            return response

        except grpc.RpcError as e:
            print(f"❌ RPC 调用失败: {e.code()}: {e.details()}")
            return None
        except Exception as e:
            print(f"❌ 创建订单异常: {str(e)}")
            return None

    def update_order_status(self, token, password=''):
        if not self.connected:
            print("❌ 未连接到服务器，请先调用 connect()")
            return None

        try:
            request = order_pb2.UpdateOrderStatusRequest(
                token=token,
                password=password
            )
            print(f"📤 发送更新订单状态请求...")
            print(f"   - 令牌: {token[:20]}..." if len(token) > 20 else f"   - 令牌: {token}")

            response = self.stub.UpdateOrderStatus(request, timeout=10)

            print(f"📥 收到响应:")
            print(f"   - 成功: {response.success}")
            if not response.success:
                print(f"   - 错误码: {order_pb2.ERROR_CODE.Name(response.error.error_code)}")
                print(f"   - HTTP状态: {response.error.http_status_code}")
                print(f"   - 用户消息: {response.error.user_message}")
                print(f"   - 错误详情: {response.error.error_message}")

            return response

        except grpc.RpcError as e:
            print(f"❌ RPC 调用失败: {e.code()}: {e.details()}")
            return None
        except Exception as e:
            print(f"❌ 更新订单状态异常: {str(e)}")
            return None