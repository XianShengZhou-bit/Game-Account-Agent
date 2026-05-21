import React, { useState, useRef, useEffect } from 'react';
import { Button, Input, Spin, message } from 'antd';
import { SendOutlined, RobotOutlined, UserOutlined } from '@ant-design/icons';
import { Message } from '../types';
import { apiClient } from '../services/api';

interface ChatbotProps {
  isOpen: boolean;
  onToggle: () => void;
}

const Chatbot: React.FC<ChatbotProps> = ({ isOpen, onToggle }) => {
  const [messages, setMessages] = useState<Message[]>([
    {
      id: '1',
      type: 'ai',
      content: '你好！我是智能客服，有什么可以帮助你的吗？',
      timestamp: new Date(),
    },
  ]);
  const [inputText, setInputText] = useState('');
  const [isLoading, setIsLoading] = useState(false);
  const messagesEndRef = useRef<HTMLDivElement>(null);

  const scrollToBottom = () => {
    messagesEndRef.current?.scrollIntoView({ behavior: 'smooth' });
  };

  useEffect(() => {
    scrollToBottom();
  }, [messages]);

  const handleSendMessage = async () => {
    if (!inputText.trim()) return;

    const messageContent = inputText.trim();

    const userMessage: Message = {
      id: Date.now().toString(),
      type: 'human',
      content: messageContent,
      timestamp: new Date(),
    };

    setMessages((prev) => [...prev, userMessage]);
    setInputText('');
    setIsLoading(true);

    try {
      // 发送消息到后端
      const result = await apiClient.sendMessage(messageContent);
      
      console.log('API result:', result);
      
      let responseContent = '抱歉，我没能理解你的意思。';
      
      // 解析响应 - 新的API返回格式
      if (result.success && result.response) {
        responseContent = result.response;
      } else if (result.message) {
        responseContent = result.message;
      } else if (typeof result === 'string') {
        responseContent = result;
      } else {
        // 尝试从响应中查找任何可能的响应字段
        responseContent = JSON.stringify(result);
      }

      const aiMessage: Message = {
        id: (Date.now() + 1).toString(),
        type: 'ai',
        content: responseContent,
        timestamp: new Date(),
      };

      setMessages((prev) => [...prev, aiMessage]);
    } catch (error) {
      console.error('Failed to send message:', error);
      const errorMsg = error instanceof Error ? error.message : '未知错误';
      message.error(`发送消息失败: ${errorMsg}`);
      
      // 显示错误消息
      const errorMessage: Message = {
        id: (Date.now() + 1).toString(),
        type: 'error',
        content: `抱歉，服务暂时不可用: ${errorMsg}`,
        timestamp: new Date(),
      };
      setMessages((prev) => [...prev, errorMessage]);
    } finally {
      setIsLoading(false);
    }
  };

  const handleKeyPress = (e: React.KeyboardEvent<HTMLInputElement>) => {
    if (e.key === 'Enter') {
      handleSendMessage();
    }
  };

  if (!isOpen) {
    return (
      <button
        onClick={onToggle}
        className="fixed bottom-8 right-8 w-16 h-16 bg-primary rounded-full shadow-lg flex items-center justify-center text-white text-2xl hover:bg-blue-600 transition-all duration-300 hover:scale-110 z-50"
      >
        <RobotOutlined />
      </button>
    );
  }

  return (
    <>
      <div className="fixed bottom-8 right-8 w-96 h-[600px] bg-white rounded-2xl shadow-2xl flex flex-col z-50 overflow-hidden">
        {/* Header */}
        <div className="bg-primary text-white p-4 flex items-center justify-between">
          <div className="flex items-center gap-2">
            <RobotOutlined className="text-xl" />
            <span className="font-semibold text-lg">智能客服</span>
          </div>
          <button onClick={onToggle} className="text-white hover:text-gray-200">
            ✕
          </button>
        </div>

        {/* Messages */}
        <div className="flex-1 overflow-y-auto p-4 bg-gray-50">
          {messages.map((msg) => (
            <div
              key={msg.id}
              className={`flex mb-4 ${msg.type === 'human' ? 'justify-end' : 'justify-start'}`}
            >
              <div
                className={`max-w-[80%] p-3 rounded-lg ${
                  msg.type === 'human'
                    ? 'bg-primary text-white rounded-tr-none'
                    : 'bg-white text-gray-800 rounded-tl-none shadow-sm'
                }`}
              >
                <div className="flex items-center gap-2 mb-1">
                  {msg.type === 'human' ? (
                    <UserOutlined className="text-sm" />
                  ) : (
                    <RobotOutlined className="text-sm" />
                  )}
                  <span className="text-xs opacity-70">
                    {msg.timestamp.toLocaleTimeString()}
                  </span>
                </div>
                <div className="whitespace-pre-wrap">{msg.content}</div>
              </div>
            </div>
          ))}
          {isLoading && (
            <div className="flex justify-start mb-4">
              <div className="bg-white p-3 rounded-lg rounded-tl-none shadow-sm">
                <Spin size="small" />
              </div>
            </div>
          )}
          <div ref={messagesEndRef} />
        </div>

        {/* Input */}
        <div className="p-4 border-t">
          <div className="flex gap-2">
            <Input
              placeholder="输入消息..."
              value={inputText}
              onChange={(e) => setInputText(e.target.value)}
              onKeyPress={handleKeyPress}
              disabled={isLoading}
            />
            <Button
              type="primary"
              icon={<SendOutlined />}
              onClick={handleSendMessage}
              loading={isLoading}
            >
              发送
            </Button>
          </div>
        </div>
      </div>
    </>
  );
};

export default Chatbot;
