import React, { useState, useEffect } from 'react';
import { Typography, Spin, Alert, Button } from 'antd';
import Header from '../components/Header';
import GameGrid from '../components/GameGrid';
import Chatbot from '../components/Chatbot';
import { Game } from '../types';
import { apiClient } from '../services/api';

const { Title } = Typography;

type TabType = 'games_list' | 'servers' | 'rares' | null;

const HomePage: React.FC = () => {
  const [isChatOpen, setIsChatOpen] = useState(false);
  const [games, setGames] = useState<Game[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  
  const [activeTab, setActiveTab] = useState<TabType>(null);
  const [tabData, setTabData] = useState<string[]>([]);
  const [tabLoading, setTabLoading] = useState(false);
  const [tabError, setTabError] = useState<string | null>(null);

  useEffect(() => {
    fetchGames();
  }, []);

  const fetchGames = async () => {
    try {
      setLoading(true);
      setError(null);
      const response = await apiClient.getGames();
      
      if (response.success && response.data) {
        setGames(response.data);
      } else {
        setError(response.message || '获取游戏列表失败');
      }
    } catch (err) {
      setError('获取游戏列表失败，请检查后端服务是否运行');
      console.error('Error fetching games:', err);
    } finally {
      setLoading(false);
    }
  };

  const handleButtonClick = async (tab: TabType) => {
    if (activeTab === tab) {
      setActiveTab(null);
      return;
    }
    
    setActiveTab(tab);
    setTabLoading(true);
    setTabError(null);
    
    try {
      let response;
      switch (tab) {
        case 'games_list':
          response = await apiClient.getGamesList();
          break;
        case 'servers':
          response = await apiClient.getServers();
          break;
        case 'rares':
          response = await apiClient.getRares();
          break;
        default:
          return;
      }
      
      if (response.success && response.data) {
        setTabData(response.data);
      } else {
        setTabError(response.message || '获取数据失败');
      }
    } catch (err) {
      setTabError('获取数据失败，请检查后端服务是否运行');
      console.error('Error fetching data:', err);
    } finally {
      setTabLoading(false);
    }
  };

  const handleGameClick = (game: Game) => {
    console.log('点击游戏:', game);
  };

  const getButtonLabel = (tab: TabType) => {
    switch (tab) {
      case 'games_list':
        return '本交易所支持的游戏';
      case 'servers':
        return '游戏可能属于的区服';
      case 'rares':
        return '账号可能有的珍稀装备';
      default:
        return '';
    }
  };

  return (
    <div className="min-h-screen flex flex-col">
      <Header />
      
      <main className="flex-1 max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
        <div className="flex justify-center gap-4 mb-8">
          <Button
            type={activeTab === 'games_list' ? 'primary' : 'default'}
            onClick={() => handleButtonClick('games_list')}
            className="w-48 h-20 text-lg font-medium"
          >
            本交易所支持的游戏
          </Button>
          <Button
            type={activeTab === 'servers' ? 'primary' : 'default'}
            onClick={() => handleButtonClick('servers')}
            className="w-48 h-20 text-lg font-medium"
          >
            游戏可能属于的区服
          </Button>
          <Button
            type={activeTab === 'rares' ? 'primary' : 'default'}
            onClick={() => handleButtonClick('rares')}
            className="w-48 h-20 text-lg font-medium"
          >
            账号可能有的珍稀装备
          </Button>
        </div>

        {activeTab && (
          <div className="mb-8 p-6 bg-gray-50 rounded-lg">
            <h3 className="text-lg font-semibold mb-4 text-gray-800">
              {getButtonLabel(activeTab)}
            </h3>
            {tabLoading ? (
              <div className="flex justify-center items-center py-8">
                <Spin size="large" />
              </div>
            ) : tabError ? (
              <Alert message={tabError} type="error" />
            ) : (
              <div className="grid grid-cols-2 sm:grid-cols-3 md:grid-cols-4 lg:grid-cols-6 gap-3">
                {tabData.map((item, index) => (
                  <div
                    key={index}
                    className="px-3 py-2 bg-white rounded-md border border-gray-200 text-gray-700 text-sm"
                  >
                    {item}
                  </div>
                ))}
              </div>
            )}
          </div>
        )}

        <Alert
          message="免责声明"
          description="本项目为个人技术项目，展示基于 LangGraph 的 AI Agent 架构设计与实现。页面所涉及的游戏内容均为AI生成，不涉及任何商业交易"
          type="info"
          showIcon
          className="mb-4"
        />
        
        <div className="mb-8">
          <Title level={2} className="!mb-2">热门游戏</Title>
          <p className="text-gray-500">选择你感兴趣的游戏，浏览可交易的账号</p>
        </div>
        
        {loading ? (
          <div className="flex justify-center items-center py-20">
            <Spin size="large" />
          </div>
        ) : error ? (
          <Alert message={error} type="error" className="mb-4" />
        ) : (
          <GameGrid games={games} onGameClick={handleGameClick} />
        )}
      </main>

      <Chatbot isOpen={isChatOpen} onToggle={() => setIsChatOpen(!isChatOpen)} />
    </div>
  );
};

export default HomePage;
