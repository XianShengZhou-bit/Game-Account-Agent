import React from 'react';
import { Link, useLocation } from 'react-router-dom';
import SearchBar from './SearchBar';

const Header: React.FC = () => {
  const location = useLocation();

  const navItems = [
    { path: '/', label: '首页' },
    { path: '/accounts', label: '账号列表' },
    { path: '/search', label: '智能搜索' },
  ];

  return (
    <header className="bg-white shadow-sm sticky top-0 z-50">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-4">
        <div className="flex items-center justify-between">
          <div className="flex items-center gap-3">
            <div className="text-3xl">🎮</div>
            <Link to="/" className="text-2xl font-bold text-gray-800 hover:text-blue-600">
              游戏账号交易平台
            </Link>
          </div>
          
          <nav className="flex items-center gap-6">
            {navItems.map((item) => (
              <Link
                key={item.path}
                to={item.path}
                className={`font-medium transition-colors ${
                  location.pathname === item.path
                    ? 'text-blue-600'
                    : 'text-gray-600 hover:text-blue-600'
                }`}
              >
                {item.label}
              </Link>
            ))}
          </nav>
          
          <div className="flex-1 max-w-xl mx-8">
            <SearchBar />
          </div>
        </div>
      </div>
    </header>
  );
};

export default Header;
