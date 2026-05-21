import React, { useState, useEffect } from 'react';
import {
  Typography,
  Spin,
  Card,
  Button,
  Tag,
  Row,
  Col,
  Modal,
  Input,
  Select,
  Form,
  Alert,
} from 'antd';
import {
  StarOutlined,
  TrophyOutlined,
  GiftOutlined,
  EnvironmentOutlined,
  CalendarOutlined,
} from '@ant-design/icons';
import Header from '../components/Header';
import { Account } from '../types';
import { apiClient } from '../services/api';

const { Title, Text, Paragraph } = Typography;

const AccountListPage: React.FC = () => {
  const [accounts, setAccounts] = useState<Account[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [selectedAccount, setSelectedAccount] = useState<Account | null>(null);
  const [isModalOpen, setIsModalOpen] = useState(false);
  const [searchParams, setSearchParams] = useState({
    game_name: '',
    min_price: '',
    max_price: '',
    server_area: '',
  });
  const [gameOptions, setGameOptions] = useState<string[]>([]);
  const [serverOptions, setServerOptions] = useState<string[]>([]);

  useEffect(() => {
    fetchAccounts();
    fetchOptions();
  }, []);

  const fetchOptions = async () => {
    try {
      const [gamesRes, serversRes] = await Promise.all([
        apiClient.getGamesList(),
        apiClient.getServers(),
      ]);
      if (gamesRes.success) setGameOptions(gamesRes.data);
      if (serversRes.success) setServerOptions(serversRes.data);
    } catch (err) {
      console.error('Error fetching options:', err);
    }
  };

  const fetchAccounts = async (params?: typeof searchParams) => {
    try {
      setLoading(true);
      setError(null);
      
      const queryParams: {
        game_name?: string;
        target_min?: number;
        target_max?: number;
        server_area?: string;
      } = {};

      if (params?.game_name && params.game_name !== '全部') {
        queryParams.game_name = params.game_name;
      }
      if (params?.min_price) {
        queryParams.target_min = parseInt(params.min_price) * 100;
      }
      if (params?.max_price) {
        queryParams.target_max = parseInt(params.max_price) * 100;
      }
      if (params?.server_area && params.server_area !== '全部') {
        queryParams.server_area = params.server_area;
      }

      const response = await apiClient.getAccounts(
        Object.keys(queryParams).length > 0 ? queryParams : undefined
      );

      if (response.success && response.data) {
        setAccounts(response.data);
      } else {
        setError(response.message || '获取账号列表失败');
      }
    } catch (err) {
      setError('获取账号列表失败，请检查后端服务是否运行');
      console.error('Error fetching accounts:', err);
    } finally {
      setLoading(false);
    }
  };

  const handleSearch = () => {
    fetchAccounts(searchParams);
  };

  const handleAccountClick = (account: Account) => {
    setSelectedAccount(account);
    setIsModalOpen(true);
  };



  const formatPrice = (price: number) => {
    return (price / 100).toFixed(2);
  };

  const getStatusText = (status: number) => {
    switch (status) {
      case 0:
        return { text: '在售', color: 'green' };
      case 1:
        return { text: '交易中', color: 'orange' };
      case 2:
        return { text: '已售出', color: 'gray' };
      default:
        return { text: '未知', color: 'gray' };
    }
  };

  const parseRareItems = (items: string): string[] => {
    try {
      return JSON.parse(items);
    } catch {
      return items.split(',').map((item: string) => item.trim());
    }
  };

  return (
    <div className="min-h-screen flex flex-col bg-gray-50">
      <Header />

      <main className="flex-1 max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
        <div className="mb-8">
          <Title level={2}>游戏账号列表</Title>
          <p className="text-gray-500 mt-2">浏览可交易的游戏账号，选择心仪的账号进行购买</p>
        </div>

        <Card className="mb-8">
          <Form layout="inline" onFinish={handleSearch}>
            <Form.Item label="游戏名称">
              <Select
                value={searchParams.game_name}
                onChange={(value) =>
                  setSearchParams((prev) => ({ ...prev, game_name: value }))
                }
                style={{ width: 180 }}
                placeholder="全部"
              >
                <Select.Option value="">全部</Select.Option>
                {gameOptions.map((game) => (
                  <Select.Option key={game} value={game}>
                    {game}
                  </Select.Option>
                ))}
              </Select>
            </Form.Item>

            <Form.Item label="最低价格(元)">
              <Input
                type="number"
                value={searchParams.min_price}
                onChange={(e) =>
                  setSearchParams((prev) => ({ ...prev, min_price: e.target.value }))
                }
                placeholder="0"
                style={{ width: 120 }}
              />
            </Form.Item>

            <Form.Item label="最高价格(元)">
              <Input
                type="number"
                value={searchParams.max_price}
                onChange={(e) =>
                  setSearchParams((prev) => ({ ...prev, max_price: e.target.value }))
                }
                placeholder="不限"
                style={{ width: 120 }}
              />
            </Form.Item>

            <Form.Item label="区服">
              <Select
                value={searchParams.server_area}
                onChange={(value) =>
                  setSearchParams((prev) => ({ ...prev, server_area: value }))
                }
                style={{ width: 180 }}
                placeholder="全部"
              >
                <Select.Option value="">全部</Select.Option>
                {serverOptions.map((server) => (
                  <Select.Option key={server} value={server}>
                    {server}
                  </Select.Option>
                ))}
              </Select>
            </Form.Item>

            <Form.Item>
              <Button type="primary" onClick={handleSearch}>
                搜索
              </Button>
            </Form.Item>
          </Form>
        </Card>

        {loading ? (
          <div className="flex justify-center items-center py-20">
            <Spin size="large" />
          </div>
        ) : error ? (
          <Alert message={error} type="error" className="mb-4" />
        ) : accounts.length === 0 ? (
          <Alert
            message="暂无账号"
            description="没有找到符合条件的游戏账号"
            type="info"
          />
        ) : (
          <Row gutter={[16, 16]}>
            {accounts.map((account) => {
              const statusInfo = getStatusText(account.status);
              const rareItems = parseRareItems(account.rare_items);

              return (
                <Col xs={24} sm={12} lg={8} xl={6} key={account.id}>
                  <Card
                    hoverable
                    className="h-full cursor-pointer transition-shadow hover:shadow-lg"
                    onClick={() => handleAccountClick(account)}
                    cover={
                      <div className="h-40 bg-gradient-to-br from-purple-500 to-blue-600 flex items-center justify-center">
                        <Text className="text-white text-4xl font-bold">
                          {account.game_name.slice(0, 2)}
                        </Text>
                      </div>
                    }
                  >
                    <Card.Meta
                      title={
                        <div className="flex items-center justify-between">
                          <span className="font-semibold">{account.title}</span>
                          <Tag color={statusInfo.color}>
                            {statusInfo.text}
                          </Tag>
                        </div>
                      }
                      description={
                        <div className="mt-3 space-y-2">
                          <div className="flex items-center text-gray-600 text-sm">
                            <EnvironmentOutlined className="mr-2 text-gray-400" />
                            <span>
                              {account.game_name} - {account.server_area}
                            </span>
                          </div>

                          <div className="flex items-center text-gray-600 text-sm">
                            <StarOutlined className="mr-2 text-yellow-500" />
                            <span>等级: {account.account_level}</span>
                          </div>

                          <div className="flex items-center text-gray-600 text-sm">
                            <TrophyOutlined className="mr-2 text-orange-500" />
                            <span>英雄: {account.hero_count}</span>
                          </div>

                          <div className="flex items-center text-gray-600 text-sm">
                            <GiftOutlined className="mr-2 text-purple-500" />
                            <span>皮肤: {account.skin_count}</span>
                          </div>

                          {rareItems.length > 0 && (
                            <div className="flex flex-wrap gap-1">
                              {rareItems.slice(0, 3).map((item, index) => (
                                <Tag
                                  key={index}
                                  className="bg-purple-100 text-purple-600 text-sm px-2 py-1"
                                >
                                  {item}
                                </Tag>
                              ))}
                              {rareItems.length > 3 && (
                                <Tag className="bg-gray-100 text-gray-600 text-sm px-2 py-1">
                                  +{rareItems.length - 3}
                                </Tag>
                              )}
                            </div>
                          )}

                          <div className="flex items-center justify-between pt-2 border-t">
                            <Text className="text-xl font-bold text-red-500">
                              ¥{formatPrice(account.price)}
                            </Text>
                            <Text className="text-sm text-gray-500">
                              如需购买请使用智能客服
                            </Text>
                          </div>
                        </div>
                      }
                    />
                  </Card>
                </Col>
              );
            })}
          </Row>
        )}
      </main>

      <Modal
        title="账号详情"
        open={isModalOpen}
        onCancel={() => setIsModalOpen(false)}
        footer={null}
        width={600}
      >
        {selectedAccount && (
          <div className="space-y-4">
            <div className="flex items-start justify-between">
              <div>
                <Title level={3}>{selectedAccount.title}</Title>
                <div className="flex items-center gap-2 mt-1">
                  <Tag color={getStatusText(selectedAccount.status).color}>
                    {getStatusText(selectedAccount.status).text}
                  </Tag>
                  <Text className="text-gray-500 text-sm">
                    账号ID: {selectedAccount.id}
                  </Text>
                </div>
              </div>
              <Text className="text-2xl font-bold text-red-500">
                ¥{formatPrice(selectedAccount.price)}
              </Text>
            </div>

            <div className="grid grid-cols-2 gap-4">
              <div className="bg-gray-50 p-4 rounded-lg">
                <div className="flex items-center gap-2 text-gray-600 mb-2">
                  <EnvironmentOutlined className="text-gray-400" />
                  <span className="font-medium">游戏信息</span>
                </div>
                <p>游戏名称: {selectedAccount.game_name}</p>
                <p>区服: {selectedAccount.server_area}</p>
              </div>

              <div className="bg-gray-50 p-4 rounded-lg">
                <div className="flex items-center gap-2 text-gray-600 mb-2">
                  <StarOutlined className="text-yellow-500" />
                  <span className="font-medium">账号属性</span>
                </div>
                <p>等级: {selectedAccount.account_level}</p>
                <p>英雄数量: {selectedAccount.hero_count}</p>
                <p>皮肤数量: {selectedAccount.skin_count}</p>
              </div>
            </div>

            {selectedAccount.description && (
              <div>
                <Paragraph>
                  <strong>账号描述:</strong> {selectedAccount.description}
                </Paragraph>
              </div>
            )}

            <div>
              <strong>珍稀物品:</strong>
              <div className="flex flex-wrap gap-2 mt-2">
                {parseRareItems(selectedAccount.rare_items).map((item, index) => (
                  <Tag
                    key={index}
                    className="bg-purple-100 text-purple-600"
                  >
                    {item}
                  </Tag>
                ))}
              </div>
            </div>

            <div className="flex items-center text-gray-400 text-sm">
              <CalendarOutlined className="mr-2" />
              <span>上架时间: {selectedAccount.created_at}</span>
            </div>

            <div className="flex justify-end gap-3 pt-4 border-t">
              <Button onClick={() => setIsModalOpen(false)}>关闭</Button>
            </div>
          </div>
        )}
      </Modal>
    </div>
  );
};

export default AccountListPage;
