import grpc
import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'proto'))

import order_pb2
import order_pb2_grpc

class OrderServiceTester:
    def __init__(self, server_address):
        self.channel = grpc.insecure_channel(server_address)
        self.stub = order_pb2_grpc.OrderServiceStub(self.channel)

    def test_create_order_success(self):
        print("\n" + "="*60)
        print("TEST: CreateOrder - Success Case")
        print("="*60)
        
        request = order_pb2.CreateOrderRequest()
        request.account_id = "ACC123456"
        request.buyer_id = 10001
        request.buyer_phone = "13800138000"
        request.buyer_id_card = "110101199001011234"
        
        print(f"Request.account_id: {request.account_id}")
        print(f"Request.buyer_id: {request.buyer_id}")
        print(f"Request.buyer_phone: {request.buyer_phone}")
        print(f"Request.buyer_id_card: {request.buyer_id_card[:4]}****")
        
        response = self.stub.CreateOrder(request)
        
        print(f"\nResponse.success: {response.success}")
        print(f"Response.order_sn: {response.order_sn}")
        print(f"Response.payment_url: {response.payment_url}")
        
        if response.success:
            print("\n[PASS] CreateOrder - Success Case")
        else:
            print("\n[FAIL] CreateOrder - Success Case")
            print(f"Error: {response.error.error_message}")
        
        return response.success

    def test_create_order_empty_account(self):
        print("\n" + "="*60)
        print("TEST: CreateOrder - Empty Account ID")
        print("="*60)
        
        request = order_pb2.CreateOrderRequest()
        request.account_id = ""
        request.buyer_id = 10001
        request.buyer_phone = "13800138000"
        request.buyer_id_card = "110101199001011234"
        
        print(f"Request.account_id: (empty)")
        
        response = self.stub.CreateOrder(request)
        
        print(f"\nResponse.success: {response.success}")
        if not response.success:
            print(f"Response.error.error_code: {order_pb2.ERROR_CODE.Name(response.error.error_code)}")
            print(f"Response.error.http_status_code: {response.error.http_status_code}")
            print(f"Response.error.user_message: {response.error.user_message}")
            print(f"Response.error.error_message: {response.error.error_message}")
            print(f"Response.error.timestamp: {response.error.timestamp}")
        
        if not response.success:
            print("\n[PASS] CreateOrder - Empty Account ID (correctly rejected)")
        else:
            print("\n[FAIL] CreateOrder - Empty Account ID (should be rejected)")
        
        return not response.success

    def test_update_order_status_success(self):
        print("\n" + "="*60)
        print("TEST: UpdateOrderStatus - Success Case")
        print("="*60)
        
        request = order_pb2.UpdateOrderStatusRequest()
        request.token = "valid_token_12345"
        request.password = "password123"
        
        print(f"Request.token: {request.token}")
        print(f"Request.password: *****")
        
        response = self.stub.UpdateOrderStatus(request)
        
        print(f"\nResponse.success: {response.success}")
        
        if response.success:
            print("\n[PASS] UpdateOrderStatus - Success Case")
        else:
            print("\n[FAIL] UpdateOrderStatus - Success Case")
            print(f"Error: {response.error.error_message}")
        
        return response.success

    def test_update_order_status_invalid_token(self):
        print("\n" + "="*60)
        print("TEST: UpdateOrderStatus - Invalid Token")
        print("="*60)
        
        request = order_pb2.UpdateOrderStatusRequest()
        request.token = ""
        request.password = "password123"
        
        print(f"Request.token: (empty)")
        
        response = self.stub.UpdateOrderStatus(request)
        
        print(f"\nResponse.success: {response.success}")
        if not response.success:
            print(f"Response.error.error_code: {order_pb2.ERROR_CODE.Name(response.error.error_code)}")
            print(f"Response.error.user_message: {response.error.user_message}")
        
        if not response.success:
            print("\n[PASS] UpdateOrderStatus - Invalid Token (correctly rejected)")
        else:
            print("\n[FAIL] UpdateOrderStatus - Invalid Token (should be rejected)")
        
        return not response.success

    def test_update_order_status_expired_token(self):
        print("\n" + "="*60)
        print("TEST: UpdateOrderStatus - Expired Token")
        print("="*60)
        
        request = order_pb2.UpdateOrderStatusRequest()
        request.token = "expired_token"
        request.password = "password123"
        
        print(f"Request.token: {request.token}")
        
        response = self.stub.UpdateOrderStatus(request)
        
        print(f"\nResponse.success: {response.success}")
        if not response.success:
            print(f"Response.error.error_code: {order_pb2.ERROR_CODE.Name(response.error.error_code)}")
            print(f"Response.error.http_status_code: {response.error.http_status_code}")
            print(f"Response.error.user_message: {response.error.user_message}")
            print(f"Response.error.error_message: {response.error.error_message}")
        
        if not response.success and response.error.error_code == order_pb2.ERROR_CODE.TOKEN_EXPIRED:
            print("\n[PASS] UpdateOrderStatus - Expired Token (correctly rejected)")
        else:
            print("\n[FAIL] UpdateOrderStatus - Expired Token")
        
        return not response.success and response.error.error_code == order_pb2.ERROR_CODE.TOKEN_EXPIRED

    def test_all_error_codes(self):
        print("\n" + "="*60)
        print("TEST: Verify All ERROR_CODE Values")
        print("="*60)
        
        error_codes = [
            ("SUCCESS", order_pb2.ERROR_CODE.SUCCESS),
            ("ACCOUNT_LOCKED", order_pb2.ERROR_CODE.ACCOUNT_LOCKED),
            ("ACCOUNT_SOLD", order_pb2.ERROR_CODE.ACCOUNT_SOLD),
            ("INVALID_TOKEN", order_pb2.ERROR_CODE.INVALID_TOKEN),
            ("TOKEN_EXPIRED", order_pb2.ERROR_CODE.TOKEN_EXPIRED),
            ("ORDER_NOT_FOUND", order_pb2.ERROR_CODE.ORDER_NOT_FOUND),
            ("ORDER_PAID", order_pb2.ERROR_CODE.ORDER_PAID),
            ("BALANCE_INSUFFICIENT", order_pb2.ERROR_CODE.BALANCE_INSUFFICIENT),
            ("INTERNAL_ERROR", order_pb2.ERROR_CODE.INTERNAL_ERROR),
            ("DB_ERROR", order_pb2.ERROR_CODE.DB_ERROR),
            ("REDIS_ERROR", order_pb2.ERROR_CODE.REDIS_ERROR),
        ]
        
        print("\nDefined ERROR_CODE enum values:")
        for name, value in error_codes:
            print(f"  {name} = {value}")
        
        print("\n[PASS] All ERROR_CODE values are accessible")
        return True

    def run_all_tests(self):
        print("\n" + "#"*60)
        print("#  OrderService Python Client Test Suite")
        print("#"*60)
        
        results = []
        
        results.append(("CreateOrder - Success", self.test_create_order_success()))
        results.append(("CreateOrder - Empty Account", self.test_create_order_empty_account()))
        results.append(("UpdateOrderStatus - Success", self.test_update_order_status_success()))
        results.append(("UpdateOrderStatus - Invalid Token", self.test_update_order_status_invalid_token()))
        results.append(("UpdateOrderStatus - Expired Token", self.test_update_order_status_expired_token()))
        results.append(("Error Codes Enum", self.test_all_error_codes()))
        
        print("\n" + "="*60)
        print("TEST SUMMARY")
        print("="*60)
        
        passed = sum(1 for _, result in results if result)
        total = len(results)
        
        for name, result in results:
            status = "[PASS]" if result else "[FAIL]"
            print(f"  {status} {name}")
        
        print(f"\nTotal: {passed}/{total} tests passed")
        print("="*60)
        
        return passed == total

    def close(self):
        self.channel.close()


def main():
    server_address = "localhost:50051"
    if len(sys.argv) > 1:
        server_address = sys.argv[1]
    
    print(f"\nConnecting to gRPC server at: {server_address}")
    
    try:
        tester = OrderServiceTester(server_address)
        success = tester.run_all_tests()
        tester.close()
        
        sys.exit(0 if success else 1)
    except grpc.RpcError as e:
        print(f"\n[ERROR] gRPC call failed: {e.code()}: {e.details()}")
        sys.exit(1)
    except Exception as e:
        print(f"\n[ERROR] {type(e).__name__}: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()