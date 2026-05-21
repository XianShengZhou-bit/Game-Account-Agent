import React from 'react';
import { useSearchParams } from 'react-router-dom';
import { Typography, Result } from 'antd';
import Header from '../components/Header';

const { Title } = Typography;

const SearchPage: React.FC = () => {
  const [searchParams] = useSearchParams();
  const gameName = searchParams.get('game_name');

  return (
    <div className="min-h-screen flex flex-col">
      <Header />
      
      <main className="flex-1 max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
        <Title level={2}>搜索结果: {gameName}</Title>
        <Result
          status="info"
          title="功能开发中"
          subTitle="搜索功能正在开发中，敬请期待！"
        />
      </main>
    </div>
  );
};

export default SearchPage;
