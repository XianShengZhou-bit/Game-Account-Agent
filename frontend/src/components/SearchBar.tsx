import React, { useState } from 'react';
import { Input } from 'antd';
import { SearchOutlined } from '@ant-design/icons';
import { useNavigate } from 'react-router-dom';

const SearchBar: React.FC = () => {
  const [searchText, setSearchText] = useState('');
  const navigate = useNavigate();

  const handleSearch = (value: string) => {
    if (value.trim()) {
      navigate(`/search?game_name=${encodeURIComponent(value.trim())}`);
    }
  };

  return (
    <Input.Search
      placeholder="请输入您要搜索的游戏(本功能暂时未实现)"
      allowClear
      enterButton={<SearchOutlined />}
      size="large"
      value={searchText}
      onChange={(e) => setSearchText(e.target.value)}
      onSearch={handleSearch}
      className="w-full max-w-2xl"
    />
  );
};

export default SearchBar;
